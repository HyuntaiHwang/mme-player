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

#include "MmpDecoderImage_V4L2.hpp"
#include "MmpUtil.hpp"
#include "mmp_buffer_mgr.hpp"
#include "v4l2_jpeg_api.h"
#include "MmpImageTool.hpp"

/////////////////////////////////////////////////////////////
//CMmpDecoderImage_V4L2 Member Functions

CMmpDecoderImage_V4L2::CMmpDecoderImage_V4L2(struct MmpDecoderCreateConfig *pCreateConfig) : CMmpDecoderImage(pCreateConfig, MMP_FALSE)
,m_p_buf_imageframe_rgb(NULL)
,m_p_buf_imageframe_yuv(NULL)
{
    
}

CMmpDecoderImage_V4L2::~CMmpDecoderImage_V4L2()
{

}

MMP_RESULT CMmpDecoderImage_V4L2::Open()
{
    MMP_RESULT mmpResult;
    
    mmpResult = CMmpDecoderImage::Open();
    if(mmpResult!=MMP_SUCCESS)
    {
        return mmpResult;
    }

    
    return MMP_SUCCESS;
}


MMP_RESULT CMmpDecoderImage_V4L2::Close()
{
    MMP_RESULT mmpResult;
    
    mmpResult=CMmpDecoderImage::Close();
    if(mmpResult!=MMP_SUCCESS)
    {
        MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpDecoderImage_V4L2::Close] CMmpDecoderVideo::Close() \n\r")));
        return mmpResult;
    }

    if(m_p_buf_imageframe_rgb != NULL) {
        mmp_buffer_mgr::get_instance()->free_media_buffer(m_p_buf_imageframe_rgb);
        m_p_buf_imageframe_rgb = NULL;
    }

    if(m_p_buf_imageframe_yuv != NULL) {
        mmp_buffer_mgr::get_instance()->free_media_buffer(m_p_buf_imageframe_yuv);
        m_p_buf_imageframe_yuv = NULL;
    }
        
    //MMPDEBUGMSG(MMPZONE_MONITOR, (TEXT("[CMmpDecoderImage_V4L2::Close] Success nForamt=(0x%08x %s) \n\r"), 
      //            m_nFormat, m_szCodecName ));

    return MMP_SUCCESS;
}

#if 1

MMP_RESULT CMmpDecoderImage_V4L2::DecodeAu(class mmp_buffer_imagestream* p_buf_imagestream, class mmp_buffer_imageframe** pp_buf_imageframe) {

    MMP_RESULT mmpResult = MMP_SUCCESS;
    struct v4l2_ion_buffer v4l2_ion_stream_src;
    struct v4l2_ion_frame  v4l2_ion_frame_dst;
    MMP_S32 iret, i;
    MMP_S32 pic_width, pic_height, jpeg_size;
    enum MMP_FOURCC fourcc;


    if(pp_buf_imageframe != NULL) {
        *pp_buf_imageframe = NULL;
    }
    memset(&v4l2_ion_stream_src, 0x00, sizeof(v4l2_ion_stream_src));
    memset(&v4l2_ion_frame_dst, 0x00, sizeof(v4l2_ion_frame_dst));
    
    v4l2_ion_stream_src.shared_fd = p_buf_imagestream->get_buf_shared_fd();
    v4l2_ion_stream_src.buf_size = p_buf_imagestream->get_buf_size();
    v4l2_ion_stream_src.vir_addr = p_buf_imagestream->get_buf_vir_addr();
    v4l2_ion_stream_src.mem_offset = 0;
    jpeg_size = p_buf_imagestream->get_stream_real_size();
    mmpResult = CMmpImageTool::Jpeg_GetWidthHeightFourcc((MMP_U8*)v4l2_ion_stream_src.vir_addr, jpeg_size, &pic_width, &pic_height, &fourcc);
    if(mmpResult == MMP_SUCCESS) {

        if(m_p_buf_imageframe_yuv != NULL) {
            mmp_buffer_mgr::get_instance()->free_media_buffer(m_p_buf_imageframe_yuv);
            m_p_buf_imageframe_yuv = NULL;
        }       

        m_p_buf_imageframe_yuv = mmp_buffer_mgr::get_instance()->alloc_media_imageframe(pic_width, pic_height, fourcc);
        
        v4l2_ion_frame_dst.pic_width = pic_width;
        v4l2_ion_frame_dst.pic_height = pic_height;
        v4l2_ion_frame_dst.plane_count = m_p_buf_imageframe_yuv->get_plane_count();
        v4l2_ion_frame_dst.fourcc = m_p_buf_imageframe_yuv->get_fourcc();
        for(i = 0; i < v4l2_ion_frame_dst.plane_count; i++) {
            v4l2_ion_frame_dst.plane[i].shared_fd = m_p_buf_imageframe_yuv->get_buf_shared_fd(i);
            v4l2_ion_frame_dst.plane[i].buf_size = m_p_buf_imageframe_yuv->get_buf_size(i);
            v4l2_ion_frame_dst.plane[i].vir_addr = (unsigned int)m_p_buf_imageframe_yuv->get_buf_vir_addr(i);
            v4l2_ion_frame_dst.plane[i].mem_offset = 0;
            v4l2_ion_frame_dst.plane[i].stride = m_p_buf_imageframe_yuv->get_stride(i);
        }
        
        iret = v4l2_jpeg_decode_ion(&v4l2_ion_stream_src, jpeg_size, &v4l2_ion_frame_dst);
        if(iret != 0) {
            mmpResult = MMP_FAILURE;
        }
        else {
            
            if(pp_buf_imageframe != NULL) {
               *pp_buf_imageframe = m_p_buf_imageframe_yuv;
            }
        }
    }

    return mmpResult;
}

#else
MMP_RESULT CMmpDecoderImage_V4L2::DecodeAu(class mmp_buffer_imagestream* p_buf_imagestream, class mmp_buffer_imageframe** pp_buf_imageframe) {

    MMP_RESULT mmpResult = MMP_SUCCESS;
    MMP_S32 v4l2_fd = -1;
    MMP_S32 iret, i;
    struct v4l2_jpeg_dec_config dec_cfg;
    struct v4l2_jpeg_dec_buf dec_inbuf;
    struct v4l2_jpeg_dec_buf dec_outbuf;
    MMP_U8* jpegdata = NULL;
    MMP_S32 jpegsize = 0, pic_width, pic_height;

    memset(&dec_inbuf, 0x00, sizeof(struct v4l2_jpeg_dec_buf));
    memset(&dec_outbuf, 0x00, sizeof(struct v4l2_jpeg_dec_buf));

#if 1

    if(pp_buf_imageframe != NULL) {
        *pp_buf_imageframe = NULL;
    }

    jpegdata = (MMP_U8*)p_buf_imagestream->get_stream_real_ptr();
    jpegsize = p_buf_imagestream->get_stream_real_size();
    CMmpImageTool::Jpeg_GetWidthHeight(jpegdata, jpegsize, &pic_width, &pic_height);
	
MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[%s::%s] ln=%d mmpResult=%d"), MMP_CLASS_NAME, MMP_CLASS_FUNC, __LINE__, mmpResult ));   
    /* open v4l2_jpu */
    if(mmpResult == MMP_SUCCESS) {
        v4l2_fd = v4l2_jpeg_dec_open();
        if(v4l2_fd < 0) {
            MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[%s::%s] FAIL: v4l2_jpeg_dec_open"), MMP_CLASS_NAME, MMP_CLASS_FUNC ));
            mmpResult = MMP_FAILURE;
        }
        else {
            iret = v4l2_jpeg_querycap(v4l2_fd);
            if(iret != 0) {
                MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[%s::%s] FAIL:  v4l2_jpeg_querycap"), MMP_CLASS_NAME, MMP_CLASS_FUNC ));
                mmpResult = MMP_FAILURE;
            }
        }
    }
MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[%s::%s] ln=%d mmpResult=%d"), MMP_CLASS_NAME, MMP_CLASS_FUNC, __LINE__, mmpResult ));   
    /* set config */
    if(mmpResult == MMP_SUCCESS) {
        memset(&dec_cfg, 0x00, sizeof(dec_cfg));
        dec_cfg.pic_width = pic_width;
        dec_cfg.pic_height = pic_height;
        dec_cfg.jpeg_image_size = jpegsize;//MMP_BYTE_ALIGN(jpegsize, 1024);
        dec_cfg.pixel_format_out = V4L2_PIX_FMT_YUV444_P1;
        iret = v4l2_jpeg_dec_set_config(v4l2_fd, &dec_cfg);
        if(iret != 0) {
            MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[%s::%s] FAIL:  v4l2_jpeg_dec_set_config"), MMP_CLASS_NAME, MMP_CLASS_FUNC ));
            mmpResult = MMP_FAILURE;
        }
    }

MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[%s::%s] ln=%d mmpResult=%d  stream_phy_addr = 0x%08x"), MMP_CLASS_NAME, MMP_CLASS_FUNC, __LINE__, mmpResult,  p_buf_imagestream->get_buf_phy_addr() ));
#if 1
    /* alloc inbuf */
    if(mmpResult == MMP_SUCCESS) {
        iret = v4l2_jpeg_dec_alloc_inbuf(v4l2_fd, &dec_inbuf);
        if(iret != 0) {
             MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[%s::%s] FAIL:  jpu_v4l2_dec_alloc_inbuf"), MMP_CLASS_NAME, MMP_CLASS_FUNC ));   
             mmpResult = MMP_FAILURE;
        }
        else {
            dec_inbuf.memory_type = V4L2_MEMORY_DMABUF;
            dec_inbuf.num_planes = 1;
            dec_inbuf.start[0] = (void*)p_buf_imagestream->get_buf_shared_fd();
            dec_inbuf.length[0] = p_buf_imagestream->get_buf_size();
        }
    }

MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[%s::%s] ln=%d mmpResult=%d"), MMP_CLASS_NAME, MMP_CLASS_FUNC, __LINE__, mmpResult ));   
    /* alloc outbuf */
    if(mmpResult == MMP_SUCCESS) {
        iret = v4l2_jpeg_dec_alloc_outbuf(v4l2_fd, &dec_cfg, &dec_outbuf);
        if(iret != 0) {
             MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[%s::%s] FAIL:  v4l2_jpeg_dec_alloc_outbuf"), MMP_CLASS_NAME, MMP_CLASS_FUNC ));   
             mmpResult = MMP_FAILURE;
        }
        else {
        
            if(m_p_buf_imageframe_rgb != NULL) {
                mmp_buffer_mgr::get_instance()->free_media_buffer(m_p_buf_imageframe_rgb);
                m_p_buf_imageframe_rgb = NULL;
            }       

            m_p_buf_imageframe_rgb = mmp_buffer_mgr::get_instance()->alloc_media_imageframe(pic_width, pic_height, MMP_FOURCC_IMAGE_YUV444_P1);
            
            //memcpy((void*)m_p_buf_imageframe_rgb->get_buf_vir_addr(), dec_outbuf.start[0], MMP_BYTE_ALIGN(pic_width*3,4)*pic_height);

MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[%s::%s] ln=%d mmpResult=%d  frame_phy_addr = 0x%08x"), MMP_CLASS_NAME, MMP_CLASS_FUNC, __LINE__, mmpResult,  
            m_p_buf_imageframe_rgb->get_buf_phy_addr() ));


            dec_outbuf.memory_type = V4L2_MEMORY_DMABUF;
            dec_outbuf.num_planes = 1;
            for(i = 0; i < dec_outbuf.num_planes; i++) {
            
                dec_outbuf.start[i] = (void*)m_p_buf_imageframe_rgb->get_buf_shared_fd(i);
                dec_outbuf.length[i] = m_p_buf_imageframe_rgb->get_buf_size(i);
            }

            //for(i = 0; i < dec_outbuf.num_planes; i++) {
              //  JPUDEBUGMSG(JPUZONE_ERROR, ("\t buf %d. 0x%08x %d \n\r", i, (unsigned int)dec_outbuf.start[i], dec_outbuf.length[i] ));
            //}
        }
    }
#endif

MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[%s::%s] ln=%d mmpResult=%d"), MMP_CLASS_NAME, MMP_CLASS_FUNC, __LINE__, mmpResult ));   
    /* run dec */
    if(mmpResult == MMP_SUCCESS) {
    
        iret = v4l2_jpeg_dec_exe(v4l2_fd, &dec_inbuf, &dec_outbuf);
        if(iret != 0) {
             MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[%s::%s] FAIL:  v4l2_jpeg_dec_exe"), MMP_CLASS_NAME, MMP_CLASS_FUNC ));   
             mmpResult = MMP_FAILURE;
        }
        else {
                    

            if(pp_buf_imageframe != NULL) {
               *pp_buf_imageframe = m_p_buf_imageframe_rgb;
            }
        }
    }

MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[%s::%s] ln=%d mmpResult=%d  v4l2_fd=%d "), MMP_CLASS_NAME, MMP_CLASS_FUNC, __LINE__, mmpResult, v4l2_fd ));   
    if(v4l2_fd >= 0) {
        v4l2_jpeg_dec_close(v4l2_fd, &dec_inbuf, &dec_outbuf);
    }

MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[%s::%s] ln=%d mmpResult=%d"), MMP_CLASS_NAME, MMP_CLASS_FUNC, __LINE__, mmpResult ));   

#else

if(m_p_buf_imageframe_rgb != NULL) {
                mmp_buffer_mgr::get_instance()->free_media_buffer(m_p_buf_imageframe_rgb);
                m_p_buf_imageframe_rgb = NULL;
            }       

            m_p_buf_imageframe_rgb = mmp_buffer_mgr::get_instance()->alloc_media_imageframe(320, 240, MMP_FOURCC_IMAGE_RGB888);
            
            if(pp_buf_imageframe != NULL) {
               *pp_buf_imageframe = m_p_buf_imageframe_rgb;
            }

#endif
    
    return mmpResult;

}
#endif




