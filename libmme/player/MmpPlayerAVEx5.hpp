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

#ifndef MMPPLAYERAVEX5_HPP__
#define MMPPLAYERAVEX5_HPP__

#include "MmpPlayer.hpp"
#include "TemplateList.hpp"
#include "mmp_oal_mutex.hpp"
#include "mmp_oal_cond.hpp"
#include "mmp_oal_task.hpp"

#define CMmpPlayerAVEx5_DEMUX_TASK_ADD_AUDIO_DATA_ENABLE 1
#define CMmpPlayerAVEx5_DEMUX_TASK_ADD_VIDEO_DATA_ENABLE 1
#define CMmpPlayerAVEx5_AUDIO_DEC_TASK_ADD_PCM_ENABLE 1

class CMmpPlayerAVEx5 : public CMmpPlayer
{
friend class CMmpPlayer;


private:
    MMP_BOOL m_is_player_run;
    MMP_BOOL m_is_media[MMP_MEDIATYPE_MAX];

    CMmpDemuxer* m_pDemuxer;
    CMmpDecoderAudio* m_pDecoderAudio;
    CMmpDecoderVideo* m_pDecoderVideo;
    CMmpRendererVideo* m_pRendererVideo;
    CMmpRendererAudio* m_pRendererAudio;
    
    /* Video Dec */
    MMP_BOOL m_bServiceRun_VideoDec;
    class mmp_oal_task* m_p_task_video_dec;
    
    TCircular_Queue<class mmp_buffer_videostream*> m_queue_video_stream;
    class mmp_oal_mutex* m_p_mutex_video_dec;
    class mmp_oal_cond* m_p_cond_video_dec;

    TCircular_Queue<class mmp_buffer_videoframe*> m_queue_video_yuv;
    class mmp_oal_mutex* m_p_mutex_video_render;
    class mmp_oal_cond* m_p_cond_video_render;

    /* Audio Dec */
    MMP_BOOL m_bServiceRun_AudioDec;
    class mmp_oal_task* m_p_task_audio_dec;
    
    TCircular_Queue<class mmp_buffer_audiostream*> m_queue_audio_stream;
    class mmp_oal_mutex* m_p_mutex_audio_dec;
    class mmp_oal_cond* m_p_cond_audio_dec;

    TCircular_Queue<class mmp_buffer_audioframe*> m_queue_audio_pcm;
    class mmp_oal_mutex* m_p_mutex_audio_render;
    class mmp_oal_cond* m_p_cond_audio_render;
        
    /* Audio Render */
    MMP_BOOL m_bServiceRun_AudioRender;
    class mmp_oal_task* m_p_task_audio_render;
    
    /* Video Render */
    MMP_BOOL m_bServiceRun_VideoRender;
    class mmp_oal_task* m_p_task_video_render;
    
    MMP_S64 m_play_start_timestamp;
    MMP_S64 m_seek_target_time;

    /* Monitoring Value */
    MMP_S32 m_mon_vdec_count;
    MMP_S32 m_mon_vdec_last_diff;

    MMP_S32 m_mon_vren_count;
    MMP_S32 m_mon_vren_last_diff;
    
    MMP_S32 m_mon_adec_count;
    MMP_S32 m_mon_adec_last_diff;

    MMP_S32 m_mon_aren_count;
    MMP_S32 m_mon_aren_last_diff;
    
protected:
    CMmpPlayerAVEx5(CMmpPlayerCreateProp* pPlayerProp);
    virtual ~CMmpPlayerAVEx5();

    virtual MMP_RESULT Open();
    virtual MMP_RESULT Close();

private:
    void flush_queue_audio_stream();
    void flush_queue_audio_frame();
    void flush_queue_video_stream();
    void flush_queue_video_frame();
    
    inline MMP_BOOL is_audio() { return m_is_media[MMP_MEDIATYPE_AUDIO]; }
    inline MMP_BOOL is_video() { return m_is_media[MMP_MEDIATYPE_VIDEO]; }
    inline void set_audio(MMP_BOOL bflag) { m_is_media[MMP_MEDIATYPE_AUDIO] = bflag; }
    inline void set_video(MMP_BOOL bflag) { m_is_media[MMP_MEDIATYPE_VIDEO] = bflag; }

protected:
    virtual void Service();
    virtual void Service_AudioDec();
    virtual void Service_VideoDec();
    virtual void Service_AudioRender();
    virtual void Service_VideoRender();

    virtual MMP_RESULT Seek(MMP_S64 pts);

public:
    virtual MMP_RESULT PlayStop();
};

#endif


