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

#include "MmpRenderer_DummyVideo.hpp"
#include "MmpUtil.hpp"
#include "mmp_buffer_mgr.hpp"

/////////////////////////////////////////////////////////////
//CMmpRenderer_DummyVideo Member Functions

MMP_U32 CMmpRenderer_DummyVideo::m_render_file_id = 0;

CMmpRenderer_DummyVideo::CMmpRenderer_DummyVideo(struct CMmpRendererVideo::create_config* p_create_config) :  CMmpRendererVideo(p_create_config)
{

}

CMmpRenderer_DummyVideo::~CMmpRenderer_DummyVideo()
{

}

MMP_RESULT CMmpRenderer_DummyVideo::Open()
{
    MMP_RESULT mmpResult;

    mmpResult = CMmpRenderer::Open();
        
    return mmpResult;
}


MMP_RESULT CMmpRenderer_DummyVideo::Close()
{
    MMPBITMAPINFOHEADER bih;

    
    CMmpRenderer::Close();

    return MMP_SUCCESS;
}

MMP_RESULT CMmpRenderer_DummyVideo::Init_Renderer(MMP_S32 pic_width, MMP_S32 pic_height)
{
    MMP_RESULT mmpResult = MMP_SUCCESS;
    MMP_CHAR filename[64];

    mmpResult = CMmpRenderer::Open();
        
    CMmpRenderer_DummyVideo::m_render_file_id++;

    return mmpResult;
}

MMP_RESULT CMmpRenderer_DummyVideo::vf_config_internal(MMP_S32 bufcnt, MMP_S32 pic_width, MMP_S32 pic_height, struct mmp_rect display_crop_rect) {

    MMP_RESULT mmpResult;
    mmpResult = CMmpRendererVideo::vf_config_internal(bufcnt, pic_width, pic_height, display_crop_rect);
    if(mmpResult == MMP_SUCCESS) {
        mmpResult = this->Init_Renderer(pic_width, pic_height);
    }
    return mmpResult;
}

void CMmpRenderer_DummyVideo::SetFirstRenderer() {

    //MMPDEBUGMSG(1, (TEXT("[%s::%s] "), MMP_CLASS_NAME, MMP_CLASS_FUNC ));
}

void CMmpRenderer_DummyVideo::SetRotate(enum MMP_ROTATE rotate) {
    //MMPDEBUGMSG(1, (TEXT("[%s::%s] "), MMP_CLASS_NAME, MMP_CLASS_FUNC ));
}

MMP_RESULT CMmpRenderer_DummyVideo::Render(class mmp_buffer_imageframe* p_buf_imageframe) {

    return MMP_FAILURE;
}

MMP_RESULT CMmpRenderer_DummyVideo::Render(class mmp_buffer_videoframe* p_buf_vf, MMP_MEDIA_ID decoder_id) {

    MMP_S32 pic_width, pic_height;
    class mmp_buffer_addr buf_addr;
    enum MMP_FOURCC fourcc;
    MMP_U8 *Y, *U, *V;
    MMP_U8 *line;
    MMP_S32 y, y_stride, uv_stride;
    
    pic_width = p_buf_vf->get_pic_width();
    pic_height = p_buf_vf->get_pic_height();
    y_stride = p_buf_vf->get_stride_luma();
    uv_stride = p_buf_vf->get_stride_chroma();

    fourcc = p_buf_vf->get_fourcc();
    switch(fourcc) {
    
        case MMP_FOURCC_IMAGE_YUV420:  /* V4L2_PIX_FMT_YUV420 12  YUV 4:2:0  3P contigous */
        case MMP_FOURCC_IMAGE_YVU420:  /* V4L2_PIX_FMT_YUV420 12  YUV 4:2:0  3P contigous */
        case MMP_FOURCC_IMAGE_YUV420M: /* V4L2_PIX_FMT_YUV420M 12  YUV 4:2:0  3P non contiguous planes - Y, Cb, Cr */ 
            Y = (MMP_U8*)p_buf_vf->get_buf_vir_addr_y();
            U = (MMP_U8*)p_buf_vf->get_buf_vir_addr_u();
            V = (MMP_U8*)p_buf_vf->get_buf_vir_addr_v();

            line = Y;
            for(y = 0; y < pic_height; y++) {
                line += y_stride;
            }
            
            line = U;
            for(y = 0; y < (pic_height/2); y++) {
                line += uv_stride;
            }
            
            line = V;
            for(y = 0; y < (pic_height/2); y++) {
                line += uv_stride;
            }
            break;
    }

    
    //CMmpRenderer::EncodeAndMux(p_buf_vf);
    p_buf_vf->set_own(decoder_id);

    return MMP_SUCCESS;
}

