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

#ifndef MMPRENDERER_AUDIO_HPP__
#define MMPRENDERER_AUDIO_HPP__

#include "MmpRenderer.hpp"

class CMmpRendererAudio : public CMmpRenderer
{
public:
    enum ID {
        ID_SOUND_OUT=0,
        ID_WAV_WRITER
    };

    struct create_config {
        enum CMmpRendererAudio::ID id;
        enum MMP_FOURCC fourcc_in;
        MMP_S32 sr; /* m_sampling_freq; */
        MMP_S32 ch; /* m_channel */
        MMP_S32 bps; /* m_bits_per_sample */
    
        public:
            create_config() : 
              id(CMmpRendererAudio::ID_SOUND_OUT)
              ,fourcc_in(MMP_FOURCC_AUDIO_PCM) 
              ,sr(44100)
              ,ch(2)
              ,bps(16)
              {
              
              }
    };

public:
    //static CMmpRendererAudio* CreateObject(MMPWAVEFORMATEX* pwf, MMP_S32 render_type = AUDIO_RENDER_DEFUALT);
    //static CMmpRendererAudio* CreateObject(struct mmp_audio_format af);
    static CMmpRendererAudio* CreateObject(struct CMmpRendererAudio::create_config *p_create_config);

private:
    MMP_S32 m_sr; /* m_sampling_freq; */
    MMP_S32 m_ch; /* m_channel */
    MMP_S32 m_bps; /* m_bits_per_sample */
    
    
protected:
    CMmpRendererAudio(CMmpRendererAudio::create_config *p_create_config, enum MMP_FOURCC fourcc_in=MMP_FOURCC_AUDIO_PCM);
    virtual ~CMmpRendererAudio();

    virtual MMP_RESULT Open();
    virtual MMP_RESULT Close();

public:
    inline MMP_S32 get_sr() { return m_sr; } /* sample rate */
    inline MMP_S32 get_ch() { return m_ch; }    /* chanel count */
    inline MMP_S32 get_bps() { return m_bps; } /* bits/sample */

};

#ifdef __cplusplus
extern "C" {
#endif

void* mmp_render_audio_create(int samplerate, int ch, int bitperpixel);
int mmp_render_audio_destroy(void* hdl);
int mmp_render_audio_write(void* hdl, char* data, int datasize);

#ifdef __cplusplus
}
#endif


#endif

