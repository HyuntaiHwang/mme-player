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

#ifndef MMPPLAYERVIDEOEX3_HPP__
#define MMPPLAYERVIDEOEX3_HPP__

#include "MmpPlayer.hpp"
#include "mmp_buffer_mgr.hpp"

#define V4L2_JPEG_ENABLE 0

#if (V4L2_JPEG_ENABLE == 1)
#include "v4l2_jpeg_api.h"
#endif


class CMmpPlayerVideoEx3 : public CMmpPlayer
{
friend class CMmpPlayer;

private:
    CMmpDemuxer* m_pDemuxer;
    CMmpDecoderVideo* m_pDecoderVideo;
    CMmpRendererVideo* m_pRendererVideo;
        
    MMP_S64 m_last_packet_pts;
    MMP_S32 m_fps;

    class mmp_buffer_videostream* m_p_buf_vs;

#if (V4L2_JPEG_ENABLE == 1)
    void*  m_v4l2_hdl;
    int m_jpeg_enc_quality;
    struct v4l2_ion_frame m_v4l2_ion_frame_src;
    struct v4l2_ion_buffer m_v4l2_ion_jpeg_dst;
    int m_jpeg_id;
#endif

    MMP_TICKS m_seek_pts;
    MMP_BOOL m_is_seek;

    
protected:
    CMmpPlayerVideoEx3(CMmpPlayerCreateProp* pPlayerProp);
    virtual ~CMmpPlayerVideoEx3();

    virtual MMP_RESULT Open();
    virtual MMP_RESULT Close();

    virtual void Service();

    
public:
    virtual MMP_S64 GetDuration();
    virtual MMP_S64 GetPlayPosition();
    virtual MMP_S32 GetPlayFPS();
    
    virtual MMP_S32 GetVideoPicWidth();
    virtual MMP_S32 GetVideoPicHeight();
    virtual enum MMP_FOURCC GetVideoFourcc();

    virtual MMP_S32 GetVideoDecoderFPS();
    virtual MMP_S32 GetVideoDecoderDur();
    virtual MMP_S32 GetVideoDecoderTotalDecFrameCount();
    virtual const MMP_CHAR* GetVideoDecoderClassName();

    /* Video Renderer */
    virtual void SetFirstVideoRenderer();
    virtual MMP_BOOL IsFirstVideoRenderer();
    virtual void SetVideoRotate(enum MMP_ROTATE rotate);
    
    virtual MMP_RESULT Seek(MMP_S64 pts);
};

#endif


