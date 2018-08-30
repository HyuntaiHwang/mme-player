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

#ifndef _MMPPLAYER_HPP__
#define _MMPPLAYER_HPP__

#include "MmpDefine.h"
#include "MmpPlayerDef.h"
#include "MmpPlayerService.hpp"
#include "MmpDemuxer.hpp"
#include "MmpDecoder.hpp"
#include "MmpDecoderVideo.hpp"
#include "MmpDecoderAudio.hpp"
#include "MmpDecoderImage.hpp"
#include "MmpRendererVideo.hpp"
#include "MmpRendererAudio.hpp"


struct mmp_player_callback_playtime {
    //MMP_S64 player_start_time;
    //MMP_S64 player_cur_time;
    MMP_S64 media_duration;
    MMP_S64 media_pts;
    //MMP_S64 last_packet_pts[MMP_MEDIATYPE_MAX];
    //MMP_S64 last_render_time[MMP_MEDIATYPE_MAX];
};

class CMmpPlayer : public CMmpPlayerService
{
public:
    /* Player Type */
    enum {
        DEFAULT     = 0x100,
        VIDEO_ONLY,
        AUDIO_ONLY,
        AUDIO_VIDEO,
        STAGEFRIGHT,
        TONEPLAYER,
        YUVPLAYER,
        JPEG,
        OMX_VIDEO_ONLY,
        OMX_AUDIO_ONLY,
        OMX_AUDIO_VIDEO,
        OMX_VIDEO_ENC,
        ANDROID_COMPRESS,
    };

    /* Plyaer Callback Message */
    enum {
        CALLBACK_DURATION = 0x1000,
        CALLBACK_PLAYTIME
    };
    
public:
    static CMmpPlayer* CreateObject(MMP_U32 playerID, CMmpPlayerCreateProp* pPlayerProp);
    static MMP_RESULT DestroyObject(CMmpPlayer* pObj);
    
protected:
    CMmpPlayerCreateProp m_create_config;
   

protected:
    CMmpPlayer(CMmpPlayerCreateProp* pPlayerProp, MMP_CHAR* player_name);
    virtual ~CMmpPlayer();

    virtual MMP_RESULT Open();
    virtual MMP_RESULT Close();
    
    CMmpDemuxer* CreateDemuxer(void);
    
    CMmpDecoderAudio* CreateDecoderAudio(CMmpDemuxer* pDemuxer);
    CMmpDecoderVideo* CreateDecoderVideo(CMmpDemuxer* pDemuxer, CMmpRendererVideo* pRendererVideo,MMP_BOOL bFfmpegUse = MMP_FALSE);
    
    CMmpRendererAudio* CreateRendererAudio(CMmpDecoderAudio* pDecoderAudio);
    CMmpRendererVideo* CreateRendererVideo(CMmpRendererVideo::ID vid_ren_id = CMmpRendererVideo::ID_DISPLAY);

    MMP_RESULT DecodeAudioExtraData(CMmpDemuxer* pDemuxer, CMmpDecoderAudio* pDecoder);
    MMP_RESULT DecodeVideoExtraData(CMmpDemuxer* pDemuxer, CMmpDecoderVideo* pDecoder);

    
#if (MMPPLAYER_DUMP_PCM == 1)
    void DumpPCM_Write(MMP_U8* pcmdata, MMP_S32 pcmbytesize);
#endif

};

extern CMmpPlayer* CMmpPlayer_CreateOMXObject(MMP_U32 playerID, CMmpPlayerCreateProp* pPlayerProp);

#endif


