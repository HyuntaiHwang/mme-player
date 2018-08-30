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

#include "MmpRenderer_OpenGLEx2.hpp"
#include "MmpUtil.hpp"
#include "colorspace/colorspace.h"
#include "MmpImageTool.hpp"

/////////////////////////////////////////////////////////////
//CMmpRenderer_OpenGLEx2 Member Functions

#define RENDER_FOURCC_IN MMP_FOURCC_IMAGE_YVU420
//#define RENDER_FOURCC_IN MMP_FOURCC_IMAGE_ABGR8888
//#define RENDER_FOURCC_IN MMP_FOURCC_IMAGE_YUV420M_META

CMmpRenderer_OpenGLEx2::CMmpRenderer_OpenGLEx2(struct CMmpRendererVideo::create_config* p_create_config) :  
CMmpRendererVideo(p_create_config, RENDER_FOURCC_IN)

    ,m_hRenderWnd(p_create_config->hRenderWnd)
    ,m_hRenderDC(p_create_config->hRenderDC)
    ,m_is_alloc_dc(MMP_FALSE)
    ,m_pMmpGL(NULL)
    ,m_iRenderCount(0)

#if (MMPRENDERER_OPENGLEX1_DUMP == 1)
,m_fp_dump(NULL)
#endif

#if (CMmpRenderer_OpenGLEx2_YUV_DUMP == 1)
,m_yuv_dump_hdl(NULL)
,m_dump_buffer(NULL)
#endif
{
    colorspace_init();
    
    //set mmx
    yv12_to_bgra= yv12_to_bgra_mmx;
    yv12_to_bgr=yv12_to_bgr_mmx;

    if(m_hRenderDC == NULL) {
        m_hRenderDC = (void*)::GetDC((HWND)p_create_config->hRenderWnd);
        m_is_alloc_dc = MMP_TRUE;
    }
    
}

CMmpRenderer_OpenGLEx2::~CMmpRenderer_OpenGLEx2()
{
#if (MMPRENDERER_OPENGLEX1_DUMP == 1)
    if(m_fp_dump!=NULL) fclose(m_fp_dump);
#endif

    if(m_is_alloc_dc == MMP_TRUE) {
        ::ReleaseDC((HWND)m_hRenderWnd, (HDC)m_hRenderDC);
    }
}

MMP_RESULT CMmpRenderer_OpenGLEx2::Open()
{
    MMP_RESULT mmpResult;

    mmpResult = CMmpRendererVideo::Open();
    
    return mmpResult;
}

MMP_RESULT CMmpRenderer_OpenGLEx2::Close()
{
    MMP_RESULT mmpResult;

    if(m_pMmpGL)
    {
        m_pMmpGL->Close();
        delete m_pMmpGL;
        m_pMmpGL=NULL;
    }

    mmpResult=CMmpRendererVideo::Close();
    if(mmpResult!=MMP_SUCCESS)
    {
        return mmpResult;
    }

#if (CMmpRenderer_OpenGLEx2_YUV_DUMP == 1)
    yuv_dump_destroy(m_yuv_dump_hdl);
    m_yuv_dump_hdl = NULL;
    if(m_dump_buffer != NULL) {
        delete [] m_dump_buffer;
    }
#endif


    return MMP_SUCCESS;
}

MMP_RESULT CMmpRenderer_OpenGLEx2::Init_OpenGL(MMP_S32 pic_width, MMP_S32 pic_height)
{
    if(m_pMmpGL)
    {
        m_pMmpGL->Close();
        delete m_pMmpGL;
        m_pMmpGL=NULL;
    }


    if(this->m_hRenderWnd != NULL) {

        m_pMmpGL = new CMmpGL_MovieEx1((HWND)this->m_hRenderWnd, 
                                    (HDC)this->m_hRenderDC,
                                    pic_width, pic_height);
        if(m_pMmpGL==NULL)
        {
            MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CWndOpenGL::OnCreate] FAIL: CMmpGL::CreateObject \n\r")));
            return MMP_FAILURE;
        }

        if(m_pMmpGL->Open()!=MMP_SUCCESS)
        {
            m_pMmpGL->Close();
            delete m_pMmpGL;
            m_pMmpGL=NULL;
            return MMP_FAILURE;
        }

        RECT rect;
        ::GetClientRect((HWND)this->m_hRenderWnd, &rect);
        //m_pMmpGL->Resize( (rect.right-rect.left)/2, (rect.bottom-rect.top)/2);
        m_pMmpGL->Resize( (rect.right-rect.left), (rect.bottom-rect.top));

    }

    return MMP_SUCCESS;
}

MMP_RESULT CMmpRenderer_OpenGLEx2::vf_config_internal(MMP_S32 bufcnt, MMP_S32 pic_width, MMP_S32 pic_height, struct mmp_rect display_crop_rect) {

    MMP_RESULT mmpResult;
    mmpResult = CMmpRendererVideo::vf_config_internal(bufcnt, pic_width, pic_height, display_crop_rect);
    if(mmpResult == MMP_SUCCESS) {
        mmpResult = this->Init_OpenGL(pic_width, pic_height);
    }
    return mmpResult;
}


MMP_RESULT CMmpRenderer_OpenGLEx2::Render(class mmp_buffer_videoframe* p_buf_vf, MMP_MEDIA_ID decoder_id) {

    MMP_U8 *Y,*U,*V;
    MMP_S32 pic_width, pic_height;
    MMP_S32 y_stride, uv_stride;
    unsigned char* pImageBuffer;
    enum MMP_FOURCC fourcc;
    MMP_RESULT mmpResult = MMP_SUCCESS;

    p_buf_vf->set_own((MMP_MEDIA_ID)this);

    if(m_pMmpGL != NULL) {

        pic_width = p_buf_vf->get_pic_width();
        pic_height = p_buf_vf->get_pic_height();
        pImageBuffer= m_pMmpGL->GetImageBuffer();
        
        fourcc = p_buf_vf->get_fourcc();

#if (CMmpRenderer_OpenGLEx2_YUV_DUMP == 1)
        if(m_yuv_dump_hdl == NULL) {
            m_yuv_dump_hdl = yuv_dump_write_create("MmeVideRenOpenGL_Dump", pic_width, pic_height, CMmpRenderer_OpenGLEx2_DUMP_FOURCC);
        }
        if(m_dump_buffer == NULL) {
            m_dump_buffer = new MMP_U8[pic_width*pic_height*4];
        }
#endif
        

        switch(fourcc) {

            case MMP_FOURCC_IMAGE_YVU420: 
            case MMP_FOURCC_IMAGE_YUV420:  /* V4L2_PIX_FMT_YUV420 12  YUV 4:2:0  3P contigous */
            case MMP_FOURCC_IMAGE_YUV420M: /* V4L2_PIX_FMT_YUV420M 12  YUV 4:2:0  3P non contiguous planes - Y, Cb, Cr */ 
                y_stride = p_buf_vf->get_stride_luma();
                uv_stride = p_buf_vf->get_stride_chroma();
                               
                Y = (MMP_U8*)p_buf_vf->get_buf_vir_addr_y();
                U = (MMP_U8*)p_buf_vf->get_buf_vir_addr_u();
                V = (MMP_U8*)p_buf_vf->get_buf_vir_addr_v();
                
#if (CMmpRenderer_OpenGLEx2_YUV_DUMP == 1)
                {
                    CMmpImageTool::ConvertYUV420MtoRGB(Y, U, V, 
                                                       y_stride, uv_stride, uv_stride,
                                                       pic_width, pic_height,
                                                       m_dump_buffer, CMmpRenderer_OpenGLEx2_DUMP_FOURCC);
                    yuv_dump_write_data(m_yuv_dump_hdl, m_dump_buffer, pic_width*pic_height*4);
                }
#endif

                (*yv12_to_bgr)( pImageBuffer, //uint8_t * x_ptr,
				            MMP_BYTE_ALIGN(pic_width*3,4), //int x_stride,
					        Y, //uint8_t * y_src,
					        V, //uint8_t * v_src,
					        U, //uint8_t * u_src,
					        y_stride,//int y_stride,
					        uv_stride, //int uv_stride,
					        pic_width, //int width,
					        pic_height, //int height,
					        1 //int vflip
                            );

                this->Dump(Y, U, V, pic_width, pic_height);
                break;

            case MMP_FOURCC_IMAGE_YUV420M_META:
                {
                    struct mmp_image_meta* p_meta = (struct mmp_image_meta*)p_buf_vf->get_buf_vir_addr();

                    y_stride = p_meta->buf_width[0];
                    uv_stride = p_meta->buf_width[1];
                               
                    Y = (MMP_U8*)p_meta->buf_viraddr[0];
                    U = (MMP_U8*)p_meta->buf_viraddr[1];
                    V = (MMP_U8*)p_meta->buf_viraddr[2];

                    (*yv12_to_bgr)( pImageBuffer, //uint8_t * x_ptr,
				                MMP_BYTE_ALIGN(pic_width*3,4), //int x_stride,
					            Y, //uint8_t * y_src,
					            V, //uint8_t * v_src,
					            U, //uint8_t * u_src,
					            y_stride,//int y_stride,
					            uv_stride, //int uv_stride,
					            pic_width, //int width,
					            pic_height, //int height,
					            1 //int vflip
                                );

                    this->Dump(Y, U, V, pic_width, pic_height);
                }
                break;
            
            case MMP_FOURCC_IMAGE_ARGB8888:
            case MMP_FOURCC_IMAGE_ABGR8888:
                CMmpImageTool::ConvertRGB32toRGB24((MMP_U8*)p_buf_vf->get_buf_vir_addr(), pic_width, pic_height, fourcc,
                                                   pImageBuffer, MMP_FOURCC_IMAGE_RGB888);
                break;

            default:
                mmpResult = MMP_FAILURE;
                break;
        }
    
        if(mmpResult == MMP_SUCCESS) {

           m_pMmpGL->Draw();
        }
      
    }
    else {
        pic_width = this->get_pic_width();
        pic_height = this->get_pic_height();

        Y = (MMP_U8*)p_buf_vf->get_buf_vir_addr_y();
        U = (MMP_U8*)p_buf_vf->get_buf_vir_addr_u();
        V = (MMP_U8*)p_buf_vf->get_buf_vir_addr_v();

    }

    m_iRenderCount++;
    p_buf_vf->set_own(decoder_id);
    
    return MMP_SUCCESS;
}

MMP_RESULT CMmpRenderer_OpenGLEx2::Render(class mmp_buffer_imageframe* p_buf_imageframe) {

    MMP_RESULT mmpResult = MMP_FAILURE;
    MMP_U32 fourcc;

    fourcc = p_buf_imageframe->get_fourcc();
    switch(fourcc) {
        
        case MMP_FOURCC_IMAGE_RGB888:
            mmpResult = this->Render_RGB888(p_buf_imageframe->get_pic_width(), p_buf_imageframe->get_pic_height(), (MMP_U8*)p_buf_imageframe->get_buf_vir_addr());
            break;
        case MMP_FOURCC_IMAGE_BGR888:
            mmpResult = this->Render_RGB888(p_buf_imageframe->get_pic_width(), p_buf_imageframe->get_pic_height(), (MMP_U8*)p_buf_imageframe->get_buf_vir_addr());
            break;

        case MMP_FOURCC_IMAGE_YUV444_P1:
            mmpResult = this->Render_YUV444_P1(p_buf_imageframe);
            break;
        
#if 0
        case MMP_FOURCC_IMAGE_YCbCr422_P2:
            mmpResult = this->Render_YUV422_P2(p_buf_imageframe);
            break;
#endif
        
        case MMP_FOURCC_IMAGE_YUV420M:
            mmpResult = this->Render_YUV420_P3(p_buf_imageframe);
            break;
    }

    return mmpResult;
}
    
MMP_RESULT CMmpRenderer_OpenGLEx2::Render_RGB888(MMP_S32 pic_width, MMP_S32 pic_height, MMP_U8* p_image) {

    unsigned char* pImageBuffer;

    if(m_pMmpGL != NULL) {

        pImageBuffer= m_pMmpGL->GetImageBuffer();
        memcpy(pImageBuffer, p_image, MMP_BYTE_ALIGN(pic_width*3,4)*pic_height);

        CMmpImageTool::Flip_V(pic_width, pic_height, pImageBuffer, MMP_FOURCC_IMAGE_RGB888);
    
        m_pMmpGL->Draw();
    }

    return MMP_SUCCESS;
}

MMP_RESULT CMmpRenderer_OpenGLEx2::Render_BGR888(MMP_S32 pic_width, MMP_S32 pic_height, MMP_U8* p_image) {

    unsigned char* pImageBuffer;

    if(m_pMmpGL != NULL) {

        pImageBuffer= m_pMmpGL->GetImageBuffer();
        memcpy(pImageBuffer, p_image, MMP_BYTE_ALIGN(pic_width*3,4)*pic_height);
        
        CMmpImageTool::Flip_V(pic_width, pic_height, pImageBuffer, MMP_FOURCC_IMAGE_BGR888);
    
        m_pMmpGL->Draw();
    }

    return MMP_SUCCESS;
}

MMP_RESULT CMmpRenderer_OpenGLEx2::Render_YUV444_P1(class mmp_buffer_imageframe* p_buf_imageframe) {

    MMP_U8 *yuv420_y = NULL;
    MMP_U8 *yuv420_u = NULL;
    MMP_U8 *yuv420_v = NULL;
    MMP_U8 *yuv444;

    MMP_S32 y_stride, uv_stride, yuv444_stride;
        
    int pic_width, pic_height;
    unsigned char* pImageBuffer;

    if(m_pMmpGL != NULL) {

        pImageBuffer= m_pMmpGL->GetImageBuffer();
        
        pic_width = p_buf_imageframe->get_pic_width();
        pic_height = p_buf_imageframe->get_pic_height();
        y_stride = MMP_VIDEO_FRAME_STRIDE_ALIGN(pic_width);
        uv_stride = MMP_VIDEO_FRAME_STRIDE_ALIGN(pic_width/2);
        
        yuv420_y = (MMP_U8*)MMP_MALLOC(y_stride*pic_height);
        yuv420_u = (MMP_U8*)MMP_MALLOC(uv_stride*pic_height);
        yuv420_v = (MMP_U8*)MMP_MALLOC(uv_stride*pic_height);
        
        yuv444 = (MMP_U8*)p_buf_imageframe->get_buf_vir_addr();
        yuv444_stride = p_buf_imageframe->get_stride();

        CMmpImageTool::ConvertYUV444P1toYUV420P3(yuv444, yuv444_stride, 
                                                pic_width, pic_height, 
                                                yuv420_y, yuv420_u, yuv420_v, 
                                                y_stride, uv_stride, uv_stride);

            
        (*yv12_to_bgr)( pImageBuffer, //uint8_t * x_ptr,
				        MMP_BYTE_ALIGN(pic_width*3,4), //int x_stride,
					    yuv420_y, //uint8_t * y_src,
					    yuv420_v, //uint8_t * v_src,
					    yuv420_u, //uint8_t * u_src,
					    y_stride,//int y_stride,
					    uv_stride, //int uv_stride,
					    pic_width, //int width,
					    pic_height, //int height,
					    1 //int vflip
                        );


    
        m_pMmpGL->Draw();

        MMP_FREE(yuv420_y);
        MMP_FREE(yuv420_u);
        MMP_FREE(yuv420_v);
    }

    m_iRenderCount++;

    return MMP_SUCCESS;
}

MMP_RESULT CMmpRenderer_OpenGLEx2::Render_YUV422_P2(class mmp_buffer_imageframe* p_buf_imageframe) {

    MMP_U8 *yuv420_y = NULL;
    MMP_U8 *yuv420_u = NULL;
    MMP_U8 *yuv420_v = NULL;
    MMP_U8 *yuv422_y;
    MMP_U8 *yuv422_uv;

    MMP_S32 y_stride, uv_stride;
    MMP_S32 yuv422_y_stride, yuv422_uv_stride;
        
    int pic_width, pic_height;
    unsigned char* pImageBuffer;

    if(m_pMmpGL != NULL) {

        pImageBuffer= m_pMmpGL->GetImageBuffer();
        
        pic_width = p_buf_imageframe->get_pic_width();
        pic_height = p_buf_imageframe->get_pic_height();
        y_stride = MMP_VIDEO_FRAME_STRIDE_ALIGN(pic_width);
        uv_stride = MMP_VIDEO_FRAME_STRIDE_ALIGN(pic_width/2);
        yuv420_y = (MMP_U8*)MMP_MALLOC(y_stride*pic_height);
        yuv420_u = (MMP_U8*)MMP_MALLOC(uv_stride*pic_height);
        yuv420_v = (MMP_U8*)MMP_MALLOC(uv_stride*pic_height);
        
        yuv422_y = (MMP_U8*)p_buf_imageframe->get_buf_vir_addr(0);
        yuv422_uv = (MMP_U8*)p_buf_imageframe->get_buf_vir_addr(1);
        yuv422_y_stride = p_buf_imageframe->get_stride(0);
        yuv422_uv_stride = p_buf_imageframe->get_stride(1);

        CMmpImageTool::ConvertYUV422P2toYUV420P3(yuv422_y, yuv422_uv, 
                                            yuv422_y_stride, yuv422_uv_stride,
                                            pic_width, pic_height, 
                                            yuv420_y, yuv420_u, yuv420_v, 
                                            y_stride, uv_stride, uv_stride
                                            );

            
        (*yv12_to_bgr)( pImageBuffer, //uint8_t * x_ptr,
				        MMP_BYTE_ALIGN(pic_width*3,4), //int x_stride,
					    yuv420_y, //uint8_t * y_src,
					    yuv420_v, //uint8_t * v_src,
					    yuv420_u, //uint8_t * u_src,
					    y_stride,//int y_stride,
					    uv_stride, //int uv_stride,
					    pic_width, //int width,
					    pic_height, //int height,
					    1 //int vflip
                        );


    
        m_pMmpGL->Draw();

        MMP_FREE(yuv420_y);
        MMP_FREE(yuv420_u);
        MMP_FREE(yuv420_v);
    }

    m_iRenderCount++;

    return MMP_SUCCESS;
}

MMP_RESULT CMmpRenderer_OpenGLEx2::Render_YUV420_P3(class mmp_buffer_imageframe* p_buf_imageframe) {

    MMP_U8 *Y,*Cb,*Cr;
    int pic_width, pic_height;
    int lumaSize, y_stride, uv_stride;
    unsigned char* pImageBuffer;

    if(m_pMmpGL != NULL) {

        pic_width = m_pMmpGL->GetPicWidth();
        pic_height = m_pMmpGL->GetPicHeight();
        pImageBuffer= m_pMmpGL->GetImageBuffer();
        lumaSize=pic_width*pic_height;
        
        Y = (MMP_U8*)p_buf_imageframe->get_buf_vir_addr_y();
        Cb = (MMP_U8*)p_buf_imageframe->get_buf_vir_addr_cb();
        Cr = (MMP_U8*)p_buf_imageframe->get_buf_vir_addr_cr();

        y_stride = p_buf_imageframe->get_stride(0);
        uv_stride = p_buf_imageframe->get_stride(1);
            
        (*yv12_to_bgr)( pImageBuffer, //uint8_t * x_ptr,
				        MMP_BYTE_ALIGN(pic_width*3,4), //int x_stride,
					    Y, //uint8_t * y_src,
					    Cr, //uint8_t * v_src,
					    Cb, //uint8_t * u_src,
					    y_stride,//int y_stride,
					    uv_stride, //int uv_stride,
					    pic_width, //int width,
					    pic_height, //int height,
					    1 //int vflip
                        );
       m_pMmpGL->Draw();
    }

    m_iRenderCount++;

    return MMP_SUCCESS;
}


void CMmpRenderer_OpenGLEx2::Dump(MMP_U8* Y, MMP_U8* U, MMP_U8* V, MMP_U32 buffer_width, MMP_U32 buffer_height) {
    
#if (MMPRENDERER_OPENGLEX1_DUMP == 1)
    if(m_fp_dump==NULL) {
        m_fp_dump = fopen("d:\\work\\dump.yuv", "wb");
    }

    if(m_fp_dump != NULL) {
    
        int lumaSize;
        int chromaSize;
        
        lumaSize = buffer_width*buffer_height;
        chromaSize = lumaSize/4;
    
        //if( (m_iRenderCount >= 30*2) && (m_iRenderCount < 30*3) ) {
        if( (m_iRenderCount == 30*2) || (m_iRenderCount == 30*10) ) {
            fwrite(Y, 1, lumaSize, m_fp_dump);
            fwrite(U, 1, chromaSize, m_fp_dump);
            fwrite(V, 1, chromaSize, m_fp_dump);
        }
    }

#endif
}

