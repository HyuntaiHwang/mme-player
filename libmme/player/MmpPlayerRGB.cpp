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

#include "MmpPlayerRGB.hpp"
#include "MmpUtil.hpp"
#include "mmp_buffer_mgr.hpp"
#include "MmpImageTool.hpp"

/////////////////////////////////////////////////////////////
//CMmpPlayerRGB Member Functions

CMmpPlayerRGB::CMmpPlayerRGB(CMmpPlayerCreateProp* pPlayerProp) : CMmpPlayer(pPlayerProp, "RGB")

,m_fp(NULL)
,m_pRendererVideo(NULL)
{
    

}

CMmpPlayerRGB::~CMmpPlayerRGB()
{
    
}

MMP_RESULT CMmpPlayerRGB::Open()
{
    MMP_RESULT mmpResult = MMP_SUCCESS;
    struct CMmpRendererVideo::create_config renderer_video_create_config;

    /* create YUVFile */
    if(mmpResult == MMP_SUCCESS) {
        m_fp = fopen(this->m_create_config.filename, "rb");
        if(m_fp == NULL) {
            mmpResult = MMP_FAILURE;
        }
    }

    /* create video renderer */
    if(mmpResult == MMP_SUCCESS ) {
        
#if (MMP_OS == MMP_OS_WIN32)
        renderer_video_create_config.hRenderWnd = this->m_create_config.video_config.m_hRenderWnd;
        renderer_video_create_config.hRenderDC = this->m_create_config.video_config.m_hRenderDC;
#endif

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


MMP_RESULT CMmpPlayerRGB::Close()
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

void CMmpPlayerRGB::Service()
{
    MMP_U32 frame_count = 0;
    MMP_S32 i, rdsz;
    MMP_S32  y, pic_width, pic_height;
    MMP_U8 *Y, *U, *V, *line;
    MMP_S32 y_stride, uv_stride;
    class mmp_buffer_videoframe* p_buf_vf_arr[16];
    class mmp_buffer_videoframe* p_buf_vf;
    MMP_S32 use_buf_cnt = m_pRendererVideo->vf_get_count();
    MMP_U8* rgbdata;// = new MMP_U8[pic_width*pic_height*4];
    MMP_S32 frame_cnt = 0;
    MMP_CHAR bmpfilename[256];
    enum MMP_FOURCC fourcc;
    MMP_U32 cur_tick, before_tick;
    MMP_S64 t1, t2;

    pic_width = m_create_config.option.yuv.width;
    pic_height = m_create_config.option.yuv.height;
    fourcc = m_create_config.option.yuv.fourcc;

    for(i = 0; i < use_buf_cnt; i++) {
        p_buf_vf_arr[i] = this->m_pRendererVideo->vf_get(i);
        p_buf_vf_arr[i]->set_own((MMP_MEDIA_ID)this);
    }

    before_tick = CMmpUtil::GetTickCount();
    rgbdata = new MMP_U8[pic_width*pic_height*4];
       
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

            
            fread((void*)rgbdata, 1, pic_width*pic_height*4, m_fp);    

#if 0
            sprintf(bmpfilename, "d:\\work\\temp%04d.bmp", frame_cnt);
            CMmpImageTool::Bmp_SaveFile(bmpfilename, pic_width, pic_height, rgbdata, fourcc);
#endif

            this->mon_vdec_begin();
            t1 = CMmpUtil::GetTickCountUS();
            CMmpImageTool::ConvertRGBtoYUV420M_neon_ex1(rgbdata, 
                                       pic_width, pic_height, 
                                       fourcc,
                                       Y, U, V,
                                       y_stride, uv_stride, uv_stride );
            t2 = CMmpUtil::GetTickCountUS();
            p_buf_vf->set_coding_dur(t2-t1);
            this->mon_vdec_end(NULL, NULL, p_buf_vf);
            
            frame_cnt++;
        }
                
        if(m_bServiceRun != MMP_TRUE) break;

        if(p_buf_vf != NULL) {
            this->mon_vren_begin();
            m_pRendererVideo->Render(p_buf_vf, (MMP_MEDIA_ID)this);
            this->mon_vren_end(p_buf_vf);
        }
           
        cur_tick = CMmpUtil::GetTickCount();
        if( (cur_tick - before_tick) > 1000 ) {
            
            this->mon_print_every_1sec_video(NULL);
            this->mon_reset_every_1sec();
            before_tick = cur_tick;
        }
        

        CMmpUtil::Sleep(1);
        frame_count++;
        
    } /* endo fo while(m_bServiceRun == MMP_TRUE) { */

    delete [] rgbdata;

    MMPDEBUGMSG(1, (TEXT("[CMmpPlayerRGB::Service] Task Ended!!")));

}

