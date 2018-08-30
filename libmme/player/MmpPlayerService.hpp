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

#ifndef MMPPLAYER_SERVICE_HPP__
#define MMPPLAYER_SERVICE_HPP__

#include "MmpDefine.h"
#include "MmpPlayerDef.h"
#include "MmpDemuxer.hpp"
#include "MmpDecoder.hpp"
#include "MmpDecoderVideo.hpp"
#include "MmpDecoderAudio.hpp"
#include "MmpRenderer.hpp"
#include "mmp_oal_task.hpp"


class CMmpPlayerService {

/* monitor function */
private:
    struct monitor_prop_time {
        MMP_U32 start_tick;
        MMP_U32 end_tick;
        MMP_S64 pts;
    };
    struct monitor_prop_audio {
        MMP_S32 fps;
        struct monitor_prop_time t;
    };
    struct monitor_prop_video {
        MMP_S32 pic_width;
        MMP_S32 pic_height;
        MMP_S32 fps;
        MMP_U32 dur_sum;
        MMP_U32 dur_sum_dec;
        MMP_S32 fps_t;
        MMP_U32 dur_sum_dec_t;
        MMP_S64 stream_size_t;
        MMP_FOURCC fourcc_in;
        MMP_FOURCC fourcc_out;
        struct monitor_prop_time t;
    };
    struct monitor {
        struct monitor_prop_audio adec;
        struct monitor_prop_audio aren;
        struct monitor_prop_video vdec;
        struct monitor_prop_video vren;
    };
    struct monitor m_mon;

protected:
    void mon_reset();
    void mon_reset_every_1sec();
    
    void mon_vdec_begin();
    void mon_vdec_end(CMmpDecoderVideo* pDecoderVideo, class mmp_buffer_videostream *p_buf_vs, class mmp_buffer_videoframe* p_buf_vf);
    void mon_vren_begin();
    void mon_vren_end(class mmp_buffer_videoframe* p_buf_vf);

    void mon_print_every_1sec_video(const MMP_CHAR* codec_name);
    void mon_print_every_1sec_audio(const MMP_CHAR* codec_name);
    void mon_print_every_1sec(const MMP_CHAR* codec_name, MMP_BOOL is_audio=MMP_TRUE, MMP_BOOL is_video=MMP_TRUE);

protected:

    MMP_BOOL m_bServiceRun;
    class mmp_oal_task* m_p_task;

private:
    MMP_CHAR m_player_name[64];

        
protected:
    CMmpPlayerService(MMP_CHAR* player_name);
    virtual ~CMmpPlayerService();

    virtual MMP_RESULT Open();
    virtual MMP_RESULT Close();

    static void ServiceStub(void* parm);
    virtual void Service()=0;

    static void ServiceStub_VideoDec(void* parm);
    virtual void Service_VideoDec() { };

    static void ServiceStub_AudioDec(void* parm);
    virtual void Service_AudioDec() { };

    static void ServiceStub_AudioRender(void* parm);
    virtual void Service_AudioRender() { };

    static void ServiceStub_VideoRender(void* parm);
    virtual void Service_VideoRender() { };

protected:

    void Service_Audio_Only(CMmpDemuxer* pDemuxer, CMmpDecoderAudio* pDecoderAudio, CMmpRenderer* pRendererAudio);

    void Service_AV_Simple(CMmpDemuxer* pDemuxer, 
                           CMmpDecoderAudio* pDecoderAudio, CMmpDecoderVideo* pDecoderVideo,
                           CMmpRenderer* pRendererAudio, CMmpRenderer* pRendererVideo);
    
public:
    inline MMP_CHAR* GetPlayerName() { return m_player_name; }

    virtual MMP_RESULT PlayStart();
    virtual MMP_RESULT PlayStop();
    virtual MMP_RESULT PlayPause() { return MMP_FAILURE; }
    virtual MMP_RESULT PlayResume() { return MMP_FAILURE; }

    virtual MMP_RESULT Seek(MMP_S64 pts) { return MMP_FAILURE; }
		
    /* Play Property*/
    virtual MMP_S64 GetDuration() { return 0LL;}
    virtual MMP_S64 GetDuration(MMP_S32 *hour, MMP_S32* min, MMP_S32* sec, MMP_S32* msec);
    virtual MMP_S64 GetPlayPosition() { return 0LL;}
    virtual MMP_S64 GetPlayPosition(MMP_S32 *hour, MMP_S32* min, MMP_S32* sec, MMP_S32* msec);

    virtual MMP_U32 GetDurationMS() { return 0LL;}
    virtual MMP_U32 GetPlayPositionMS() { return 0LL;}
    virtual MMP_S32 GetPlayFPS() { return 0; }

    virtual MMP_S32 GetVideoPicWidth() { return 0; }
    virtual MMP_S32 GetVideoPicHeight() { return 0; }
    virtual enum MMP_FOURCC GetVideoFourcc() { return MMP_FOURCC_VIDEO_UNKNOWN; }
    
    /* Decoder Prop */
    virtual MMP_S32 GetVideoDecoderFPS() { return 0; }
    virtual MMP_S32 GetVideoDecoderDur() { return 0; }
    virtual MMP_S32 GetVideoDecoderTotalDecFrameCount() { return 0; }
    virtual const MMP_CHAR* GetVideoDecoderClassName() { return "Unknown"; }
    
    /* Video Renderer */
    virtual void SetFirstVideoRenderer() { }
    virtual MMP_BOOL IsFirstVideoRenderer() { return MMP_FALSE; }
    virtual void SetVideoRotate(enum MMP_ROTATE rotate) { }
    
};

#endif

