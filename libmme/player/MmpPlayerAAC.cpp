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


#include "MmpPlayerAAC.hpp"
#include "MmpUtil.hpp"

/////////////////////////////////////////////////////////////
//CMmpPlayerAAC Member Functions

CMmpPlayerAAC::CMmpPlayerAAC(CMmpPlayerCreateProp* pPlayerProp) : CMmpPlayer(pPlayerProp, "AAC" )

,m_fp_pcm(NULL)

,m_pDecoderAudio(NULL)
,m_pRendererAudio(NULL)

,m_p_buf_as(NULL)
,m_p_buf_af(NULL)

{
    

}

CMmpPlayerAAC::~CMmpPlayerAAC()
{
    
    
}

MMP_RESULT CMmpPlayerAAC::Open()
{
    MMP_RESULT mmpResult = MMP_SUCCESS;
    MMP_S64 file_pos;

    CMmpDecoderAudio::create_config decoder_audio_create_config;
        
    
    /* create pcm file */
    if(mmpResult == MMP_SUCCESS ) {
        m_fp_pcm = fopen(this->m_create_config.filename, "rb");
        if(m_fp_pcm == NULL) {
            mmpResult = MMP_FAILURE;
        }
        else {
            fseek(m_fp_pcm, 0, SEEK_END);
            file_pos = (MMP_S64)ftell(m_fp_pcm);
            fseek(m_fp_pcm, 0, SEEK_SET);

            //m_play_duration = (file_pos*1000000L) / (MMP_S64)this->m_create_config.audio_wf.nAvgBytesPerSec;
        }
    }
    
    /* alloc audio stream buffer */
    if(mmpResult == MMP_SUCCESS) {
        m_p_buf_as = mmp_buffer_mgr::get_instance()->alloc_media_audiostream((MMP_MEDIA_ID)this, mmp_buffer_audiostream::DEFAULT_MAX_STREAM_SIZE, mmp_buffer::HEAP);
        if(m_p_buf_as == NULL) {
            mmpResult = MMP_FAILURE;
            MMPDEBUGMSG(MMPZONE_ERROR, (TEXT(MMP_CTAG"FAIL: alloc audio stream"), MMP_CNAME));
        }
    }

    /* alloc audio frame buffer */
    if(mmpResult == MMP_SUCCESS) {
        m_p_buf_af = mmp_buffer_mgr::get_instance()->alloc_media_audioframe((MMP_MEDIA_ID)this, mmp_buffer_audioframe::DEFAULT_MAX_FRAME_SIZE, mmp_buffer::HEAP);
        if(m_p_buf_af == NULL) {
            mmpResult = MMP_FAILURE;
            MMPDEBUGMSG(MMPZONE_ERROR, (TEXT(MMP_CTAG"FAIL: alloc audio frame"), MMP_CNAME));
        }
    }
    

    /* create audio decoder */
    if(mmpResult == MMP_SUCCESS ) {

        decoder_audio_create_config.fourcc_in = MMP_FOURCC_AUDIO_AAC; 
        decoder_audio_create_config.fourcc_out = MMP_FOURCC_AUDIO_PCM; 
        m_pDecoderAudio = CMmpDecoderAudio::CreateObject(&decoder_audio_create_config);
        if(m_pDecoderAudio == NULL) {
            mmpResult = MMP_FAILURE;
        }
        else {

#if 0
            mmpResult = m_pDemuxer->GetAudioExtraData(m_p_buf_as);
            if(mmpResult == MMP_SUCCESS) {
                m_p_buf_as->set_flag(mmp_buffer_media::FLAG_CONFIGDATA);
                mmpResult = m_pDecoderAudio->DecodeAu(m_p_buf_as, m_p_buf_af);
            }
#endif
            
        }
    }

    /* create audio render */
    if(mmpResult == MMP_SUCCESS ) {
        CMmpRendererAudio::create_config renderer_audio_create_config;
        m_pRendererAudio = CMmpRendererAudio::CreateObject(&renderer_audio_create_config);
        if(m_pRendererAudio == NULL) {
            mmpResult = MMP_FAILURE;
        }
        else {
    
     
        }
    }


    return mmpResult;
}


MMP_RESULT CMmpPlayerAAC::Close()
{
    CMmpPlayer::Close();

    if(m_pDecoderAudio != NULL) {
        CMmpDecoder::DestroyObject(m_pDecoderAudio);
        m_pDecoderAudio = NULL;
    }

    if(m_pRendererAudio != NULL) {
        CMmpRenderer::DestroyObject(m_pRendererAudio);  
        m_pRendererAudio = NULL;

        CMmpUtil::Sleep(1000);
    }
    
    if(m_p_buf_as != NULL) { 
        mmp_buffer_mgr::get_instance()->free_media_buffer(m_p_buf_as);
        m_p_buf_as = NULL;
    }
    
    if(m_p_buf_af != NULL) { 
        mmp_buffer_mgr::get_instance()->free_media_buffer(m_p_buf_af);
        m_p_buf_af = NULL;
    }
    
    if(m_fp_pcm!=NULL) { 
        fclose(m_fp_pcm); 
        m_fp_pcm=NULL; 
    }
     
    return MMP_SUCCESS;

}

void CMmpPlayerAAC::Service()
{
}

