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

#ifndef _MMPRENDERER_OPENGLEX1_HPP__
#define _MMPRENDERER_OPENGLEX1_HPP__

#include "../opengl/MmpGL_MovieEx1.hpp"
#include "MmpRendererVideo.hpp"

#define MMPRENDERER_OPENGLEX1_DUMP 0

#ifdef WIN32
#define CMmpRenderer_OpenGLEx1_YUV_DUMP 0
#define CMmpRenderer_OpenGLEx1_DUMP_FOURCC MMP_FOURCC_IMAGE_ABGR8888
#endif

class CMmpRenderer_OpenGLEx1 : public CMmpRendererVideo
{
friend class CMmpRendererVideo;

private:
    void* m_hRenderWnd;
    void* m_hRenderDC;

    MMP_BOOL m_is_alloc_dc;
    
    MMP_BOOL m_is_init_gl;
    CMmpGL_MovieEx1* m_pMmpGL;
        
#if (MMPRENDERER_OPENGLEX1_DUMP == 1)
    FILE* m_fp_dump;
#endif
    MMP_S32 m_iRenderCount;
    
#if (CMmpRenderer_OpenGLEx1_YUV_DUMP == 1)
    void* m_yuv_dump_hdl;
    MMP_U8* m_dump_buffer;
#endif

protected:
    CMmpRenderer_OpenGLEx1(struct CMmpRendererVideo::create_config* p_create_config);
    virtual ~CMmpRenderer_OpenGLEx1();

    virtual MMP_RESULT Open();
    virtual MMP_RESULT Close();

    void Dump(MMP_U8* Y, MMP_U8* U, MMP_U8* V, MMP_U32 buffer_width, MMP_U32 buffer_height);

private:
    virtual MMP_RESULT vf_config_internal(MMP_S32 bufcnt, MMP_S32 pic_width, MMP_S32 pic_height);
    MMP_RESULT Init_OpenGL(MMP_S32 pic_width, MMP_S32 pic_height);

    MMP_RESULT Render_RGB888(MMP_S32 pic_width, MMP_S32 pic_height, MMP_U8* p_image);
    MMP_RESULT Render_BGR888(MMP_S32 pic_width, MMP_S32 pic_height, MMP_U8* p_image);
    MMP_RESULT Render_YUV444_P1(class mmp_buffer_imageframe* p_buf_imageframe);
    MMP_RESULT Render_YUV422_P2(class mmp_buffer_imageframe* p_buf_imageframe);
    MMP_RESULT Render_YUV420_P3(class mmp_buffer_imageframe* p_buf_imageframe);

public:
    
    virtual MMP_RESULT Render(class mmp_buffer_videoframe* p_buf_videoframe, MMP_MEDIA_ID decoder_id);
    virtual MMP_RESULT Render(class mmp_buffer_imageframe* p_buf_imageframe);
    
    virtual MMP_RESULT OnSize(int cx, int cy) 
    { 
        if(m_pMmpGL)
        {
            m_pMmpGL->Resize(cx,cy);
        }
        return MMP_SUCCESS;
    }
    
};

#endif

