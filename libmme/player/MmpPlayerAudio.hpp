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

#ifndef _MMPPLAYERAUDIO_HPP__
#define _MMPPLAYERAUDIO_HPP__

#include "MmpPlayer.hpp"

#if (MMP_PLATFORM==MMP_PLATFORM_WIN32)
#define CMmpPlayerAudio_PCM_DUMP 0
#else
#define CMmpPlayerAudio_PCM_DUMP 0
#endif

#if (CMmpPlayerAudio_PCM_DUMP == 1)
#define CMmpPlayerAudio_PCM_DUMPNAME "CMmpPlayerAudio.Dump.pcm"
#if (MMP_PLATFORM==MMP_PLATFORM_WIN32)
#define CMmpPlayerAudio_PCM_FILENAME "d:\\work\\"CMmpPlayerAudio_PCM_DUMPNAME
#elif (MMP_PLATFORM==MMP_PLATFORM_ANDROID)
#define CMmpPlayerAudio_PCM_FILENAME "/data/"CMmpPlayerAudio_PCM_DUMPNAME
#endif
#endif

class CMmpPlayerAudio : public CMmpPlayer
{
friend class CMmpPlayer;

private:
    CMmpDemuxer* m_pDemuxer;
    CMmpDecoderAudio* m_pDecoderAudio;
    CMmpRendererAudio* m_pRendererAudio;

    //CMmpMediaSample m_MediaSample;
    //CMmpMediaSampleDecodeResult m_DecResult;
    
    //MMP_U8* m_stream_buffer;
    //MMP_U32 m_stream_buffer_max_size;

    //MMP_U8* m_pcm_buffer;
    //MMP_U32 m_pcm_buffer_max_size;
    class mmp_buffer_audiostream *m_p_buf_as;
    class mmp_buffer_audioframe *m_p_buf_af;
    
#if (CMmpPlayerAudio_PCM_DUMP == 1)
    void* m_pcm_dump_hdl;
#endif

    MMP_BOOL m_is_seek;
    MMP_S64 m_seek_pts;
    MMP_TICKS m_last_render_pts;


protected:
    CMmpPlayerAudio(CMmpPlayerCreateProp* pPlayerProp);
    virtual ~CMmpPlayerAudio();

    virtual MMP_RESULT Open();
    virtual MMP_RESULT Close();
    virtual MMP_RESULT Seek(MMP_S64 pts);
    virtual MMP_S64 GetDuration();
    virtual MMP_S64 GetPlayPosition();


    virtual void Service();
};

#endif


