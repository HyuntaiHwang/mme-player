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
#include "MmpMuxer.hpp"

void* mme_muxer_create_object(struct mme_muxer_config* p_muxer_config) {

    struct MmpMuxerCreateConfig muxer_create_config;
    CMmpMuxer* pMuxer;

    memset(&muxer_create_config, 0x00, sizeof(muxer_create_config));
    strcpy((char*)muxer_create_config.filename, p_muxer_config->filename);

    muxer_create_config.bMedia[MMP_MEDIATYPE_VIDEO] = MMP_TRUE;
    muxer_create_config.bih.biSize = sizeof(MMPBITMAPINFOHEADER);
    muxer_create_config.bih.biCompression = p_muxer_config->video_encoder_config.format;
    muxer_create_config.bih.biWidth = p_muxer_config->video_encoder_config.width;
    muxer_create_config.bih.biHeight = p_muxer_config->video_encoder_config.height;
    muxer_create_config.video_bitrate = p_muxer_config->video_encoder_config.bitrate;
    muxer_create_config.video_fps = p_muxer_config->video_encoder_config.fps;
    muxer_create_config.video_idr_period = p_muxer_config->video_encoder_config.idr_period;

    pMuxer = CMmpMuxer::CreateObject(&muxer_create_config);

    return (void*)pMuxer;
}

int mme_muxer_destroy_object(void* hdl) {

    CMmpMuxer* pMuxer = (CMmpMuxer*)hdl;
    
    if(pMuxer!=NULL) {
        CMmpMuxer::DestroyObject(pMuxer);
    }

    return 0;
}

int mme_muxer_add_video_config(void* hdl, unsigned char* p_config_data, int config_data_size) {

    MMP_RESULT mmpResult;
    int iret = -1;
    CMmpMuxer* pMuxer = (CMmpMuxer*)hdl;
    
    if( (pMuxer != NULL) && (config_data_size>0) ) {
            
        mmpResult = pMuxer->AddVideoConfig(p_config_data, config_data_size);
        if(mmpResult == MMP_SUCCESS) {
            iret = 0;
        }
    }

   
    return iret;
}

int mme_muxer_add_video_data(void* hdl, unsigned char* p_video_stream, int video_stream_size, unsigned int flag) {

    MMP_RESULT mmpResult;
    int iret = -1;
    MMP_S64 pts;
    const MMP_S64 SECOND = 1000000L;
    MMP_S64 FPS;
    MMP_S64 DurPerSecond; ;
    CMmpMuxer* pMuxer = (CMmpMuxer*)hdl;
    
    if( (pMuxer != NULL) && (video_stream_size>0) ) {
            
        /* set pts */
        FPS = (MMP_S64)pMuxer->get_video_fps();
        DurPerSecond = SECOND/FPS;
        
        pts = pMuxer->get_last_input_pts();
        pts += DurPerSecond;

        mmpResult = pMuxer->AddVideoData(p_video_stream, video_stream_size, flag, pts);
        if(mmpResult == MMP_SUCCESS) {
            iret = -1;
        }

    }
    
    return iret;
}
