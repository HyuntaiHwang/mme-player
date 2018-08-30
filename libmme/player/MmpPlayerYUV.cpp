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

#include "MmpPlayerYUV.hpp"
#include "MmpUtil.hpp"
#include "mmp_buffer_mgr.hpp"

/////////////////////////////////////////////////////////////
//CMmpPlayerYUV Member Functions

CMmpPlayerYUV::CMmpPlayerYUV(CMmpPlayerCreateProp* pPlayerProp) : CMmpPlayer(pPlayerProp, "YUV")

,m_fp(NULL)
,m_pRendererVideo(NULL)
{
    

}

CMmpPlayerYUV::~CMmpPlayerYUV()
{
    
}

MMP_RESULT CMmpPlayerYUV::Open()
{
    MMPBITMAPINFOHEADER bih;
    MMP_RESULT mmpResult = MMP_SUCCESS;
    struct CMmpRendererVideo::create_config renderer_video_create_config;

    /* create YUVFile */
    if(mmpResult == MMP_SUCCESS) {
        m_fp = fopen(this->m_create_config.filename, "rb");
        if(m_fp == NULL) {
            mmpResult = MMP_FAILURE;
        }
        else {
            fseek(m_fp, -1 * (int)sizeof(MMPBITMAPINFOHEADER), SEEK_END);
            fread(&bih, 1, sizeof(MMPBITMAPINFOHEADER), m_fp);
            fseek(m_fp, 0, SEEK_SET);
        
            if( (bih.biSize == sizeof(MMPBITMAPINFOHEADER))
               &&  (bih.biCompression == MMPMAKEFOURCC('Y','U','V',' ') )
            ){
                m_create_config.option.yuv.width = bih.biWidth;
                m_create_config.option.yuv.height = bih.biHeight;

                MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpPlayerYUV::Open] YUV W:%d H:%d  "), bih.biWidth, bih.biHeight ));
            }
            else {
                MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpPlayerYUV::Open] FAIL: Cannot Read  YUV Width, Height ")));
#ifndef WIN32
                return MMP_FAILURE;
#endif
            }
        }
    }

    /* create video renderer */
    if(mmpResult == MMP_SUCCESS ) {
        
#if (MMP_OS == MMP_OS_WIN32)
        renderer_video_create_config.hRenderWnd = this->m_create_config.video_config.m_hRenderWnd;
        renderer_video_create_config.hRenderDC = this->m_create_config.video_config.m_hRenderDC;
#endif
        renderer_video_create_config.id = CMmpRendererVideo::ID_DISPLAY;
        m_pRendererVideo = CMmpRendererVideo::CreateObject(&renderer_video_create_config);
        if(m_pRendererVideo == NULL) {
            mmpResult = MMP_FAILURE;
        }
        else {
            m_pRendererVideo->vf_config(4, m_create_config.option.yuv.width, m_create_config.option.yuv.height);
        }
    }

    
    /* Base Class Open */
    if(mmpResult == MMP_SUCCESS ) {
        mmpResult = CMmpPlayer::Open();
    }

    return mmpResult;
}


MMP_RESULT CMmpPlayerYUV::Close()
{
    CMmpPlayer::Close();

    if(m_pRendererVideo != NULL) {
        CMmpRenderer::DestroyObject(m_pRendererVideo);  
        m_pRendererVideo = NULL;
    }

    if(m_fp != NULL) {
        fclose(m_fp);
        m_fp = NULL;
    }

    return MMP_SUCCESS;
}

void CMmpPlayerYUV::Service()
{
    MMP_U32 frame_count = 0;
    MMP_S32 i, rdsz;
    MMP_S32  y, pic_width, pic_height;
    MMP_U8 *Y, *U, *V, *line;
    MMP_S32 y_stride, uv_stride;
    class mmp_buffer_videoframe* p_buf_vf_arr[16];
    class mmp_buffer_videoframe* p_buf_vf;
    MMP_S32 use_buf_cnt = m_pRendererVideo->vf_get_count();

    pic_width = m_create_config.option.yuv.width;
    pic_height = m_create_config.option.yuv.height;

    for(i = 0; i < use_buf_cnt; i++) {
        p_buf_vf_arr[i] = this->m_pRendererVideo->vf_get(i);
        p_buf_vf_arr[i]->set_own((MMP_MEDIA_ID)this);
    }
       
    while(m_bServiceRun == MMP_TRUE) {

        p_buf_vf = NULL;
        for(i = 0; i < use_buf_cnt; i++) {
            if(p_buf_vf_arr[i]->is_own((MMP_MEDIA_ID)this) == MMP_TRUE) {
                p_buf_vf = p_buf_vf_arr[i];
                break;
            }
        }

        if(p_buf_vf != NULL) {
            switch(p_buf_vf->get_fourcc()) {
                case MMP_FOURCC_IMAGE_YUV420:  /* V4L2_PIX_FMT_YUV420 12  YUV 4:2:0  3P contigous */
                case MMP_FOURCC_IMAGE_YVU420: 
                case MMP_FOURCC_IMAGE_YUV420M: /* V4L2_PIX_FMT_YUV420M 12  YUV 4:2:0  3P non contiguous planes - Y, Cb, Cr */ 

                    y_stride = p_buf_vf->get_stride_luma();
                    uv_stride = p_buf_vf->get_stride_chroma();
                               
                    Y = (MMP_U8*)p_buf_vf->get_buf_vir_addr_y();
                    U = (MMP_U8*)p_buf_vf->get_buf_vir_addr_u();
                    V = (MMP_U8*)p_buf_vf->get_buf_vir_addr_v();
                
                    break;
                
                default:
                    p_buf_vf = NULL;
                    break;
            }
        }

        if(p_buf_vf != NULL) {

            //Read Y
            line = Y;
            for(y = 0; y < pic_height; y++) {
               rdsz = fread((void*)line, 1, pic_width, m_fp);    
               if(rdsz != pic_width) {
                   m_bServiceRun = MMP_FALSE;
                   break;
               }

               line += y_stride;
            }

            //Read U
            line = U;
            for(y = 0; y < pic_height/2; y++) {
               rdsz = fread((void*)line, 1, pic_width/2, m_fp);    
               if(rdsz != (pic_width/2)) {
                   m_bServiceRun = MMP_FALSE;
                   break;
               }

               line += uv_stride;
            }
            //Read V
            line = V;
            for(y = 0; y < pic_height/2; y++) {
               rdsz = fread((void*)line, 1, pic_width/2, m_fp);    
               if(rdsz != (pic_width/2)) {
                   m_bServiceRun = MMP_FALSE;
                   break;
               }

               line += uv_stride;
            }

        }
                
        if(m_bServiceRun != MMP_TRUE) break;

        if(p_buf_vf != NULL) {
            m_pRendererVideo->Render(p_buf_vf, (MMP_MEDIA_ID)this);
        }
           

        frame_count++;
        
        CMmpUtil::Sleep(50);
    } /* endo fo while(m_bServiceRun == MMP_TRUE) { */

    
    MMPDEBUGMSG(1, (TEXT("[CMmpPlayerYUV::Service] Task Ended!!")));

}

