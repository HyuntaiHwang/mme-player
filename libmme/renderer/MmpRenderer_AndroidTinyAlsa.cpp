/*
 * Copyright (c) 2014 Anapass Co., Ltd.
 *              http://www.anapass.com/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Alternatively, Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


#include "MmpRenderer_AndroidTinyAlsa.hpp"
#include "MmpUtil.hpp"

// TODO: determine actual audio DSP and hardware latency
// Additionnal latency introduced by audio DSP and hardware in ms
#define AUDIO_HW_OUT_LATENCY_MS 0
// Default audio output sample rate
#define AUDIO_HW_OUT_SAMPLERATE 44100
// Default audio output channel mask
#define AUDIO_HW_OUT_CHANNELS (AudioSystem::CHANNEL_OUT_STEREO)
// Default audio output sample format
#define AUDIO_HW_OUT_FORMAT (AudioSystem::PCM_16_BIT)
// Kernel pcm out buffer size in frames at 44.1kHz
#define AUDIO_HW_OUT_PERIOD_SZ 2048 /* 1024 */
#define AUDIO_HW_OUT_PERIOD_CNT 4
// Default audio output buffer size in bytes
#define AUDIO_HW_OUT_PERIOD_BYTES (AUDIO_HW_OUT_PERIOD_SZ * 2 * sizeof(int16_t))

// Default audio input sample rate
#define AUDIO_HW_IN_SAMPLERATE 44100
// Default audio input channel mask
#define AUDIO_HW_IN_CHANNELS (AudioSystem::CHANNEL_IN_MONO)
// Default audio input sample format
#define AUDIO_HW_IN_FORMAT (AudioSystem::PCM_16_BIT)
// Kernel pcm in buffer size in frames at 44.1kHz (before resampling)
#define AUDIO_HW_IN_PERIOD_SZ 2048
#define AUDIO_HW_IN_PERIOD_CNT 2
// Default audio input buffer size in bytes (8kHz mono)
#define AUDIO_HW_IN_PERIOD_BYTES ((AUDIO_HW_IN_PERIOD_SZ*sizeof(int16_t))/8)




/////////////////////////////////////////////////////////////
//CMmpRenderer_AndroidTinyAlsa Member Functions

CMmpRenderer_AndroidTinyAlsa::CMmpRenderer_AndroidTinyAlsa(CMmpRendererCreateProp* pRendererProp) :  CMmpRendererAudio(MMP_MEDIATYPE_AUDIO, pRendererProp)
,m_pcm_handle(NULL)
{
    
}

CMmpRenderer_AndroidTinyAlsa::~CMmpRenderer_AndroidTinyAlsa()
{

}

MMP_RESULT CMmpRenderer_AndroidTinyAlsa::Open()
{
    MMP_RESULT mmpResult = MMP_SUCCESS;

    int haveAlternateCard = 0;
    unsigned flags = PCM_OUT;
#ifdef WIN32
    struct pcm_config config = {
        /*channels : */2,
        /*rate : */AUDIO_HW_OUT_SAMPLERATE,
        /*period_size : */AUDIO_HW_OUT_PERIOD_SZ,
        /*period_count : */AUDIO_HW_OUT_PERIOD_CNT,
        /*format : */PCM_FORMAT_S16_LE,
        /*start_threshold : */0,
        /*stop_threshold : */0,
        /*silence_threshold : */0,
    };
#else
    struct pcm_config config = {
        channels : 2,
        rate : AUDIO_HW_OUT_SAMPLERATE,
        period_size : AUDIO_HW_OUT_PERIOD_SZ,
        period_count : AUDIO_HW_OUT_PERIOD_CNT,
        format : PCM_FORMAT_S16_LE,
        start_threshold : 0,
        stop_threshold : 0,
        silence_threshold : 0,
    };
#endif

    config.rate = this->m_RendererProp.m_wf.nSamplesPerSec;
    config.channels = this->m_RendererProp.m_wf.nChannels;
    //config.rate = this->m_RendererProp.m_wf.nSamplesPerSec;

    /* open pcm handle */
    if(mmpResult == MMP_SUCCESS) {
        m_pcm_handle = pcm_open(haveAlternateCard ? 1 : 0, 0, flags, &config);
        if(m_pcm_handle == NULL) {
            mmpResult = MMP_FAILURE;
            MMPDEBUGMSG(MMPZONE_ERROR, (TEXT(MMP_CTAG" FAIL: pcm_open"), MMP_CNAME));
        }
    }

    /* check ready */
    if(mmpResult == MMP_SUCCESS) {
    
        if (!pcm_is_ready(m_pcm_handle)) {
            MMPDEBUGMSG(MMPZONE_ERROR, (TEXT(MMP_CTAG" FAIL: pcm_is_ready"), MMP_CNAME));
            mmpResult = MMP_FAILURE;
        }
        
    }

    return MMP_SUCCESS;
}

MMP_RESULT CMmpRenderer_AndroidTinyAlsa::Close()
{
    if(m_pcm_handle != NULL) {
        pcm_close(m_pcm_handle);
        m_pcm_handle = NULL;
    }
    
    return MMP_SUCCESS;
}

MMP_RESULT CMmpRenderer_AndroidTinyAlsa::Render(class mmp_buffer_audioframe* p_buf_af)
{
    MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpRenderer_AndroidTinyAlsa::Render] 1. sz = %d"), p_buf_af->get_data_size()));

    pcm_wait(m_pcm_handle, 1000);

    MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpRenderer_AndroidTinyAlsa::Render] 2. sz = %d"), p_buf_af->get_data_size()));

    pcm_write(m_pcm_handle, (void*)p_buf_af->get_buf_vir_addr(), p_buf_af->get_data_size());
    
    MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpRenderer_AndroidTinyAlsa::Render] 3. sz = %d"), p_buf_af->get_data_size()));

    return MMP_SUCCESS;
}
