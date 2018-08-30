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

#include "MmpDefine.h"
#include "mme_c_api.h"
#include "MmpDemuxer.hpp"
#include "MmpMuxer.hpp"

int mme_movie_mux_direct_copy_video(const char* srcfilename, const char* dstfilename, unsigned long start_pts_ms, unsigned long dur_ms, int fps) {

    struct MmpDemuxerCreateConfig demuxer_create_config;
    struct MmpMuxerCreateConfig muxer_create_config;

    CMmpDemuxer* pDemuxer;
    CMmpMuxer* pMuxer;
    MMP_S32 strmsize;
    MMP_U8* p_stream;
    MMP_TICKS start_pts = (MMP_TICKS)start_pts_ms*1000L;
    MMP_TICKS dur = (MMP_TICKS)dur_ms*1000L;
    MMP_TICKS frame_interval = 1000000L/(MMP_TICKS)fps; 
    
    class mmp_buffer_videostream* p_buf_vs;

    /* create stream buffer */
    p_buf_vs = mmp_buffer_mgr::get_instance()->alloc_media_videostream((MMP_MEDIA_ID)0, 0x00300000, mmp_buffer::HEAP);
    MMP_ASSERT(p_buf_vs);
    
    /* open demuxer */
    strcpy((char*)demuxer_create_config.filename, srcfilename);
    pDemuxer = CMmpDemuxer::CreateObject(&demuxer_create_config);
    MMP_ASSERT(pDemuxer);

    /* set muxer config */
    memset(&muxer_create_config, 0x00, sizeof(muxer_create_config));
    strcpy((char*)muxer_create_config.filename, dstfilename);
    muxer_create_config.bMedia[MMP_MEDIATYPE_VIDEO] = MMP_TRUE;
    muxer_create_config.bMedia[MMP_MEDIATYPE_AUDIO] = MMP_FALSE;
    muxer_create_config.bih.biSize = sizeof(MMPBITMAPINFOHEADER);
    muxer_create_config.bih.biCompression = pDemuxer->GetVideoFourcc();
    muxer_create_config.bih.biWidth = pDemuxer->GetVideoPicWidth();
    muxer_create_config.bih.biHeight = pDemuxer->GetVideoPicHeight();
    muxer_create_config.video_bitrate = 24*100000;//pDemuxer->GetVideoBitrate();
    muxer_create_config.video_fps = 30;//pDemuxer->GetVideoFPS();
    muxer_create_config.video_idr_period = 10;
        
    pMuxer = CMmpMuxer::CreateObject(&muxer_create_config);
    MMP_ASSERT(pMuxer);

    /* set dsi data */
    pDemuxer->GetVideoExtraData(p_buf_vs);
    strmsize = p_buf_vs->get_stream_size();
    if(strmsize > 0) {
        p_stream = (MMP_U8*)p_buf_vs->get_buf_vir_addr();
        pMuxer->AddVideoConfig(p_stream, strmsize);
    }

    MMP_TICKS pts_src, pts_dst = 0;
    
    while(1) {
        pDemuxer->GetNextVideoData(p_buf_vs);
        p_buf_vs->set_stream_offset(0);
        strmsize = p_buf_vs->get_stream_real_size();
        if(strmsize <= 0) {
            break;
        }
        p_stream = (MMP_U8*)p_buf_vs->get_buf_vir_addr();
        
        pts_src = p_buf_vs->get_pts();
        if(pts_src >= start_pts) {
            p_buf_vs->set_pts(pts_dst);
            pMuxer->AddMediaData(p_buf_vs);
            pts_dst += frame_interval;
        }

        if(pts_src > start_pts + dur) {
             break;
        }
        
    }

    CMmpMuxer::DestroyObject(pMuxer);
    CMmpDemuxer::DestroyObject(pDemuxer);

    return 0;
}

