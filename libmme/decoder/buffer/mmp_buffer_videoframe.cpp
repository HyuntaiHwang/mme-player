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

#include "mmp_buffer_videoframe.hpp"
#include "mmp_buffer_mgr.h"
#include <media/hardware/HardwareAPI.h>
#include <media/hardware/MetadataBufferType.h>
#include "MmpImageTool.hpp"

/**********************************************************
class member
**********************************************************/

mmp_buffer_videoframe::mmp_buffer_videoframe(MMP_MEDIA_ID producer_id, enum MMP_BUFTYPE buftype) : mmp_buffer_media(VIDEO_FRAME, producer_id)

,m_pic_width(0)
,m_pic_height(0)
,m_plane_count(0)
,m_fourcc(MMP_FOURCC_VIDEO_UNKNOWN)
,m_buftype(buftype)

,m_android_metadata_ptr(NULL)
,m_android_metadata_size(0)
{
    MMP_S32 i;

    for(i = 0; i < MMP_IMAGE_MAX_PLANE_COUNT; i++) {
        m_p_mmp_buffer[i] = NULL;
    }

}

mmp_buffer_videoframe::~mmp_buffer_videoframe() {

    if(m_android_metadata_ptr != NULL) {
        delete [] m_android_metadata_ptr;
    }
}

class mmp_buffer_addr mmp_buffer_videoframe::get_buf_addr(MMP_S32 frame_id) {

    class mmp_buffer_addr buf_addr;

    if( (frame_id >= 0) && (frame_id < m_plane_count) ) {
        buf_addr = this->m_p_mmp_buffer[frame_id]->get_buf_addr();
    }
    
    return buf_addr;
}

void mmp_buffer_videoframe::sync_buf(MMP_S32 frame_id) {

    if( (frame_id >= 0) && (frame_id < m_plane_count) ) {
        this->m_p_mmp_buffer[frame_id]->sync_buf();
    }
    
    return ;
}

void mmp_buffer_videoframe::sync_buf() {

    for(int frame_id = 0; frame_id < m_plane_count; frame_id++) {
        this->m_p_mmp_buffer[frame_id]->sync_buf();
    }
    
    return ;
}

MMP_ADDR mmp_buffer_videoframe::get_buf_vir_addr(MMP_S32 frame_id) {

    MMP_ADDR p_addr = 0;

    if( (frame_id >= 0) && (frame_id < m_plane_count) ) {
        p_addr = (MMP_ADDR)this->m_p_mmp_buffer[frame_id]->get_vir_addr();
    }
    else {

        switch(m_fourcc) {

            case MMP_FOURCC_IMAGE_YUV420:
                if(frame_id  == 1) p_addr = get_buf_vir_addr_u();
                else if(frame_id  == 2) p_addr = get_buf_vir_addr_v();
                break;

            case MMP_FOURCC_IMAGE_YVU420:
                if(frame_id  == 1) p_addr = get_buf_vir_addr_v();
                else if(frame_id  == 2) p_addr = get_buf_vir_addr_u();
                break;
        }
    }
    
    
    return p_addr;
}

MMP_ADDR mmp_buffer_videoframe::get_buf_cal_addr_y(MMP_ADDR base_addr) { 
    return base_addr;
}

MMP_ADDR mmp_buffer_videoframe::get_buf_cal_addr_u(MMP_ADDR base_addr) { 

    MMP_ADDR addr = 0;
    MMP_S32 luma_buf_size, chroma_buf_size;
    enum MMP_FOURCC fourcc = m_fourcc;

    switch(fourcc) {
    
        case MMP_FOURCC_IMAGE_YUV420:
            addr = base_addr; 
            luma_buf_size = m_buf_stride[MMP_YUV420_PLAINE_INDEX_Y] * m_buf_height[MMP_YUV420_PLAINE_INDEX_Y];
            addr += luma_buf_size;
            break;

        case MMP_FOURCC_IMAGE_YVU420:
            addr = base_addr; 
            luma_buf_size = m_buf_stride[MMP_YUV420_PLAINE_INDEX_Y] * m_buf_height[MMP_YUV420_PLAINE_INDEX_Y];
            chroma_buf_size = m_buf_stride[MMP_YUV420_PLAINE_INDEX_V] * m_buf_height[MMP_YUV420_PLAINE_INDEX_V];
            addr += luma_buf_size + chroma_buf_size;
            break;

        case MMP_FOURCC_IMAGE_NV12:
        case MMP_FOURCC_IMAGE_NV21:
            addr = base_addr; 
            luma_buf_size = m_buf_stride[MMP_YUV420_PLAINE_INDEX_Y] * m_buf_height[MMP_YUV420_PLAINE_INDEX_Y];
            addr += luma_buf_size;
            break;
    }
    
    return addr;
}

MMP_ADDR mmp_buffer_videoframe::get_buf_cal_addr_v(MMP_ADDR base_addr) { 

    MMP_ADDR addr = 0;
    MMP_S32 luma_buf_size, chroma_buf_size;
    enum MMP_FOURCC fourcc = m_fourcc;

    switch(fourcc) {
    
        case MMP_FOURCC_IMAGE_YUV420:
            addr = base_addr;
            luma_buf_size = m_buf_stride[MMP_YUV420_PLAINE_INDEX_Y] * m_buf_height[MMP_YUV420_PLAINE_INDEX_Y];
            chroma_buf_size = m_buf_stride[MMP_YUV420_PLAINE_INDEX_U] * m_buf_height[MMP_YUV420_PLAINE_INDEX_U];
            addr += luma_buf_size + chroma_buf_size;
            break;

        case MMP_FOURCC_IMAGE_YVU420:
            addr = base_addr;
            luma_buf_size = m_buf_stride[MMP_YUV420_PLAINE_INDEX_Y] * m_buf_height[MMP_YUV420_PLAINE_INDEX_Y];
            addr += luma_buf_size;
            break;

        case MMP_FOURCC_IMAGE_NV12:
        case MMP_FOURCC_IMAGE_NV21:
            addr = base_addr; 
            luma_buf_size = m_buf_stride[MMP_YUV420_PLAINE_INDEX_Y] * m_buf_height[MMP_YUV420_PLAINE_INDEX_Y];
            addr += luma_buf_size;
            break;
    }
    
    return addr;
}


MMP_ADDR mmp_buffer_videoframe::get_buf_vir_addr_y() { 

    MMP_ADDR addr = 0;
    enum MMP_FOURCC fourcc = m_fourcc;

    switch(fourcc) {
        case MMP_FOURCC_IMAGE_YUV420:
        case MMP_FOURCC_IMAGE_YVU420:
            addr = this->get_buf_vir_addr(MMP_YUV420_PLAINE_INDEX_Y);
            addr = this->get_buf_cal_addr_y(addr);
            break;

        default:
            addr = this->get_buf_vir_addr(MMP_YUV420_PLAINE_INDEX_Y);
            break;

    }
    
    return addr;
}

MMP_ADDR mmp_buffer_videoframe::get_buf_vir_addr_u() { 

    MMP_ADDR addr = 0;
    enum MMP_FOURCC fourcc = m_fourcc;

    switch(fourcc) {
    
        case MMP_FOURCC_IMAGE_NV12:
        case MMP_FOURCC_IMAGE_NV21:
        case MMP_FOURCC_IMAGE_YUV420:
        case MMP_FOURCC_IMAGE_YVU420:
            addr = this->get_buf_vir_addr_y();
            addr = this->get_buf_cal_addr_u(addr);
            break;
        
        case MMP_FOURCC_IMAGE_YUV420M:
            addr = this->get_buf_vir_addr(MMP_YUV420_PLAINE_INDEX_U); 
            break;
    }
    
    return addr;
}

MMP_ADDR mmp_buffer_videoframe::get_buf_vir_addr_v() { 

    MMP_ADDR addr = 0;
    enum MMP_FOURCC fourcc = m_fourcc;

    switch(fourcc) {
    
        case MMP_FOURCC_IMAGE_NV12:
        case MMP_FOURCC_IMAGE_NV21:
        case MMP_FOURCC_IMAGE_YUV420:
        case MMP_FOURCC_IMAGE_YVU420:
            addr = this->get_buf_vir_addr_y();
            addr = this->get_buf_cal_addr_v(addr);
            break;

        case MMP_FOURCC_IMAGE_YUV420M:
            addr = this->get_buf_vir_addr(MMP_YUV420_PLAINE_INDEX_V); 
            break;
    }
    
    return addr;
}
    

MMP_ADDR mmp_buffer_videoframe::get_buf_phy_addr(MMP_S32 frame_id) {

    MMP_ADDR addr = 0;

    if( (frame_id >= 0) && (frame_id < m_plane_count) ) {
        addr = this->m_p_mmp_buffer[frame_id]->get_phy_addr();
    }
    else {
        switch(m_fourcc) {
        
            case MMP_FOURCC_IMAGE_YUV420:
                if(frame_id  == 1) addr = get_buf_phy_addr_u();
                else if(frame_id  == 2) addr = get_buf_phy_addr_v();
                break;

            case MMP_FOURCC_IMAGE_YVU420:
                if(frame_id  == 1) addr = get_buf_phy_addr_v();
                else if(frame_id  == 2) addr = get_buf_phy_addr_u();
                break;
        }
    }
    
    return addr;
}

MMP_ADDR mmp_buffer_videoframe::get_buf_phy_addr_y() { 

    MMP_ADDR addr = 0;
    enum MMP_FOURCC fourcc = m_fourcc;

    switch(fourcc) {
        case MMP_FOURCC_IMAGE_YUV420:
        case MMP_FOURCC_IMAGE_YVU420:
            addr = this->get_buf_phy_addr(MMP_YUV420_PLAINE_INDEX_Y);
            addr = this->get_buf_cal_addr_y(addr);
            break;

        default:
            addr = this->get_buf_phy_addr(MMP_YUV420_PLAINE_INDEX_Y);
            break;

    }
    
    return addr;
}

MMP_ADDR mmp_buffer_videoframe::get_buf_phy_addr_u() { 

    MMP_ADDR addr = 0;
    enum MMP_FOURCC fourcc = m_fourcc;

    switch(fourcc) {
    
        case MMP_FOURCC_IMAGE_YUV420:
        case MMP_FOURCC_IMAGE_YVU420:
            addr = this->get_buf_phy_addr_y();
            addr = this->get_buf_cal_addr_u(addr);
            break;
        
        case MMP_FOURCC_IMAGE_YUV420M:
            addr = this->get_buf_phy_addr(MMP_YUV420_PLAINE_INDEX_U); 
            break;
    }
    
    return addr;
}

MMP_ADDR mmp_buffer_videoframe::get_buf_phy_addr_v() { 

    MMP_ADDR addr = 0;
    enum MMP_FOURCC fourcc = m_fourcc;

    switch(fourcc) {
    
        case MMP_FOURCC_IMAGE_YUV420:
        case MMP_FOURCC_IMAGE_YVU420:
            addr = this->get_buf_phy_addr_y();
            addr = this->get_buf_cal_addr_v(addr);
            break;

        case MMP_FOURCC_IMAGE_YUV420M:
            addr = this->get_buf_phy_addr(MMP_YUV420_PLAINE_INDEX_V); 
            break;
    }
    
    return addr;
}
    

MMP_S32 mmp_buffer_videoframe::get_buf_shared_fd(MMP_S32 frame_id) {

    MMP_S32 shared_fd = -1;

    if( (frame_id >= 0) && (frame_id < m_plane_count) ) {
        shared_fd = this->m_p_mmp_buffer[frame_id]->get_buf_shared_fd();
    }
    
    return shared_fd;
}

MMP_S32 mmp_buffer_videoframe::get_buf_total_size() {

    MMP_S32 buf_sz = 0;
    MMP_S32 frame_id;

    for(frame_id = 0; frame_id < m_plane_count; frame_id++) {
        buf_sz += this->m_p_mmp_buffer[frame_id]->get_buf_size();
    }
    
    return buf_sz;
}
    
MMP_S32 mmp_buffer_videoframe::get_buf_size(MMP_S32 frame_id) {

    MMP_S32 buf_sz = 0;

    if( (frame_id >= 0) && (frame_id < m_plane_count) ) {
        buf_sz = this->m_p_mmp_buffer[frame_id]->get_buf_size();
    }
    
    return buf_sz;
}

MMP_S32 mmp_buffer_videoframe::get_buf_size_luma() {
    
    enum MMP_FOURCC fourcc = m_fourcc;
    MMP_S32 luma_buf_size = 0;

    switch(fourcc) {
    
        case MMP_FOURCC_IMAGE_YUV420:
        case MMP_FOURCC_IMAGE_YVU420:
            luma_buf_size = m_buf_stride[MMP_YUV420_PLAINE_INDEX_Y] * m_buf_height[MMP_YUV420_PLAINE_INDEX_Y];
            break;
    }

    return luma_buf_size;
}

MMP_S32 mmp_buffer_videoframe::get_buf_size_chroma() {

    enum MMP_FOURCC fourcc = m_fourcc;
    MMP_S32 chroma_buf_size = 0;

    switch(fourcc) {
    
        case MMP_FOURCC_IMAGE_YUV420:
        case MMP_FOURCC_IMAGE_YVU420:
            chroma_buf_size = m_buf_stride[MMP_YUV420_PLAINE_INDEX_U] * m_buf_height[MMP_YUV420_PLAINE_INDEX_U];
            break;
    }

    return chroma_buf_size;
}

MMP_RESULT mmp_buffer_videoframe::save_bmp(const MMP_CHAR* filename) {

    MMP_RESULT mmpResult = MMP_FAILURE;
    enum MMP_FOURCC fourcc = m_fourcc;
    MMP_U8 *yuv420_y, *yuv420_u, *yuv420_v;
    MMP_U8 *yuv420_uv;
    MMP_S32 luma_stride, chroma_stride;
    MMP_S32 pic_width, pic_height;
    MMP_U8* rgb_data;

    switch(fourcc) {
        case MMP_FOURCC_IMAGE_YUV420:
        case MMP_FOURCC_IMAGE_YVU420:
            yuv420_y = (MMP_U8 *)this->get_buf_vir_addr_y();
            yuv420_u = (MMP_U8 *)this->get_buf_vir_addr_u();
            yuv420_v = (MMP_U8 *)this->get_buf_vir_addr_v();
            luma_stride = this->get_stride_luma();
            chroma_stride = this->get_stride_chroma();
            pic_width = this->get_pic_width();
            pic_height = this->get_pic_height();
            rgb_data = new MMP_U8[ pic_width*4*pic_height];
            mmpResult = CMmpImageTool::ConvertYUV420MtoRGB(yuv420_y, yuv420_u, yuv420_v, 
                                               luma_stride, chroma_stride, chroma_stride, 
                                               pic_width, pic_height, 
                                               rgb_data, MMP_FOURCC_IMAGE_ABGR8888);
            if(mmpResult == MMP_SUCCESS) {
                mmpResult = CMmpImageTool::Bmp_SaveFile((MMP_CHAR*)filename, pic_width, pic_height, rgb_data, MMP_FOURCC_IMAGE_ABGR8888); 
            }
            delete [] rgb_data;
            break;

        case MMP_FOURCC_IMAGE_NV12:
        case MMP_FOURCC_IMAGE_NV21:
            yuv420_y = (MMP_U8 *)this->get_buf_vir_addr_y();
            yuv420_uv = (MMP_U8 *)this->get_buf_vir_addr_u();
            luma_stride = this->get_stride_luma();
            chroma_stride = this->get_stride_chroma();
            pic_width = this->get_pic_width();
            pic_height = this->get_pic_height();
            rgb_data = new MMP_U8[ pic_width*4*pic_height];
            mmpResult = CMmpImageTool::ConvertNV12MtoRGB(yuv420_y, yuv420_uv,
                                               luma_stride, chroma_stride, 
                                               pic_width, pic_height, 
                                               rgb_data, MMP_FOURCC_IMAGE_ABGR8888);
            if(mmpResult == MMP_SUCCESS) {
                mmpResult = CMmpImageTool::Bmp_SaveFile((MMP_CHAR*)filename, pic_width, pic_height, rgb_data, MMP_FOURCC_IMAGE_ABGR8888); 
            }
            delete [] rgb_data;
            break;
    }

    return mmpResult;
}


#if (MMP_PLATFORM == MMP_PLATFORM_WIN32)
MMP_RESULT mmp_buffer_videoframe::make_android_metadat_gralloc_source() {

#if 0
            typedef struct native_handle
            {
                int version;        /* sizeof(native_handle_t) */
                int numFds;         /* number of file-descriptors at &data[0] */
                int numInts;        /* number of ints at &data[numFds] */
                int *data;        /* numFds + numInts ints */
            } native_handle_t;
            typedef const native_handle_t* buffer_handle_t;

            /* android */
            typedef struct native_handle
            {
                int version;        /* sizeof(native_handle_t) */
                int numFds;         /* number of file-descriptors at &data[0] */
                int numInts;        /* number of ints at &data[numFds] */
                int data[0];        /* numFds + numInts ints */
            } native_handle_t;

#endif

    MMP_RESULT mmpResult = MMP_FAILURE;
    MMP_U32 *metadata_array;
    native_handle_t* buf_hdl;

    if(m_android_metadata_ptr != NULL) {
        delete [] m_android_metadata_ptr;
    }

    m_android_metadata_size = 8;
    m_android_metadata_ptr = new MMP_U8[m_android_metadata_size + 64];
    if(m_android_metadata_ptr != NULL) {

        MMP_S32 fd = this->get_buf_shared_fd();
        metadata_array = (MMP_U32*)m_android_metadata_ptr;

        MMP_U8* buf_memory = (MMP_U8*)&metadata_array[2];

        buf_hdl = (native_handle_t*)&buf_memory[0];
        buf_hdl->version = 0;
        buf_hdl->numFds = 1;
        buf_hdl->numInts = 0;
        buf_hdl->data = (int*)&buf_memory[32];
        buf_hdl->data[0] = fd;

        metadata_array[0] = (MMP_U32)android::kMetadataBufferTypeGrallocSource;
        metadata_array[1] = (MMP_U32)buf_hdl;

        mmpResult = MMP_SUCCESS;
    }

    return mmpResult;
}

#endif


MMP_S32 mmp_buffer_videoframe::s_get_stride(enum MMP_FOURCC fourcc, MMP_S32 pic_width, enum MMP_BUFTYPE buftype) {

    MMP_S32 v = 0;

    switch(fourcc) {
        case MMP_FOURCC_IMAGE_YUV420:
        case MMP_FOURCC_IMAGE_YUV420M:
        case MMP_FOURCC_IMAGE_YVU420:
        case MMP_FOURCC_IMAGE_YVU420M:
            if(buftype == MMP_BUFTYPE_HW) v = V4L2_VIDEO_YUV420_Y_STRIDE(pic_width);
            else  v = V4L2_VIDEO_SW_YUV420_Y_STRIDE(pic_width);
            break;

        case MMP_FOURCC_IMAGE_NV12:
        case MMP_FOURCC_IMAGE_NV21:
            if(buftype == MMP_BUFTYPE_HW) v = V4L2_VIDEO_NV12_Y_STRIDE(pic_width);
            else  v = V4L2_VIDEO_SW_NV12_Y_STRIDE(pic_width);
            break;

        case MMP_FOURCC_IMAGE_ARGB8888:
        case MMP_FOURCC_IMAGE_ABGR8888: 
            if(buftype == MMP_BUFTYPE_HW) v = (V4L2_BYTE_ALIGN((pic_width*4), 64))/4; //V4L2_VIDEO_YUV420_Y_STRIDE(pic_width) * 4; /* HW-RGB is 32byte align */
            else  v = pic_width*4;
            break;

        case MMP_FOURCC_IMAGE_RGB888:
        case MMP_FOURCC_IMAGE_BGR888: 
            if(buftype == MMP_BUFTYPE_HW) v = (V4L2_BYTE_ALIGN((pic_width*3), 64))/3; //v = V4L2_VIDEO_YUV420_Y_STRIDE(pic_width) * 2; /* HW-RGB is 32byte align */
            else v = MMP_BYTE_ALIGN( (pic_width*3), 4);
            break;
    }
    
    return v;
}

MMP_S32 mmp_buffer_videoframe::s_get_height(enum MMP_FOURCC fourcc, MMP_S32 pic_height, enum MMP_BUFTYPE buftype) {

    MMP_S32 v = 0;

    switch(fourcc) {
        case MMP_FOURCC_IMAGE_YUV420:
        case MMP_FOURCC_IMAGE_YUV420M:
        case MMP_FOURCC_IMAGE_YVU420:
        case MMP_FOURCC_IMAGE_YVU420M:
            if(buftype == MMP_BUFTYPE_HW) v = V4L2_VIDEO_YUV420_Y_HEIGHT_ALIGN(pic_height);
            else v = V4L2_VIDEO_SW_YUV420_Y_HEIGHT_ALIGN(pic_height);
            break;

        case MMP_FOURCC_IMAGE_NV12:
        case MMP_FOURCC_IMAGE_NV21:
            if(buftype == MMP_BUFTYPE_HW) v = V4L2_VIDEO_NV12_Y_HEIGHT_ALIGN(pic_height);
            else v = V4L2_VIDEO_SW_NV12_Y_HEIGHT_ALIGN(pic_height);
            break;

        case MMP_FOURCC_IMAGE_ARGB8888:
        case MMP_FOURCC_IMAGE_ABGR8888: 
        case MMP_FOURCC_IMAGE_RGB888:
        case MMP_FOURCC_IMAGE_BGR888: 
            v = pic_height;
            break;
    }
    
    return v;
}


MMP_S32 mmp_buffer_videoframe::s_get_size(enum MMP_FOURCC fourcc, MMP_S32 pic_width, MMP_S32 pic_height, enum MMP_BUFTYPE buftype) {

    MMP_S32 v = 0;
    MMP_S32 s;

    switch(fourcc) {
        case MMP_FOURCC_IMAGE_YUV420: 
        case MMP_FOURCC_IMAGE_YVU420: 
              if(buftype == MMP_BUFTYPE_HW) v = V4L2_VIDEO_YVU420_FRAME_SIZE(pic_width, pic_height); 
              else v = V4L2_VIDEO_SW_YVU420_FRAME_SIZE(pic_width, pic_height); 
              break;
              
        case MMP_FOURCC_IMAGE_NV12: 
        case MMP_FOURCC_IMAGE_NV21: 
              if(buftype == MMP_BUFTYPE_HW) v = V4L2_VIDEO_NV12_FRAME_SIZE(pic_width, pic_height); 
              else v = V4L2_VIDEO_SW_NV12_FRAME_SIZE(pic_width, pic_height); 
              break;


        case MMP_FOURCC_IMAGE_ARGB8888:
        case MMP_FOURCC_IMAGE_ABGR8888:
               if(buftype == MMP_BUFTYPE_HW) s = (V4L2_BYTE_ALIGN((pic_width*4), 64))/4; /* HW-RGB is 32byte align */
               else  s = pic_width*4;
               v = s*pic_height;
               break;
        
        case MMP_FOURCC_IMAGE_RGB888:
        case MMP_FOURCC_IMAGE_BGR888:
               if(buftype == MMP_BUFTYPE_HW) s = (V4L2_BYTE_ALIGN((pic_width*3), 64))/3; /* HW-RGB is 32byte align */
               else s = MMP_BYTE_ALIGN( (pic_width*3), 4);
               v = s * pic_height;
               break;
        
        case MMP_FOURCC_IMAGE_ANDROID_OPAQUE:
            v = MMP_IMAGE_ANDROID_OPAQUE_SIZE; 
            break;
    }

    return v;
}


MMP_S32 mmp_buffer_videoframe::s_get_u_stride(enum MMP_FOURCC fourcc, MMP_S32 pic_width, enum MMP_BUFTYPE buftype) {
    MMP_S32 ustride = 0;


    switch(fourcc) {
        case MMP_FOURCC_IMAGE_YUV420: 
        case MMP_FOURCC_IMAGE_YVU420: 
            if(buftype == MMP_BUFTYPE_HW) ustride = V4L2_VIDEO_YVU420_U_STRIDE(pic_width); 
            else ustride = V4L2_VIDEO_SW_YVU420_U_STRIDE(pic_width); 
            break;

        case MMP_FOURCC_IMAGE_NV12: 
        case MMP_FOURCC_IMAGE_NV21: 
            if(buftype == MMP_BUFTYPE_HW) ustride = V4L2_VIDEO_NV12_UV_STRIDE(pic_width); 
            else ustride = V4L2_VIDEO_SW_NV12_UV_STRIDE(pic_width); 
            break;
    }
    return ustride;    
}


MMP_S32 mmp_buffer_videoframe::s_get_u_height(enum MMP_FOURCC fourcc, MMP_S32 pic_height, enum MMP_BUFTYPE buftype) {
    
    MMP_S32 uheight = 0;

    switch(fourcc) {
        case MMP_FOURCC_IMAGE_YUV420: 
        case MMP_FOURCC_IMAGE_YVU420: 
            if(buftype == MMP_BUFTYPE_HW) uheight = V4L2_VIDEO_YVU420_U_HEIGHT_ALIGN(pic_height); 
            else uheight = V4L2_VIDEO_SW_YVU420_U_HEIGHT_ALIGN(pic_height); 
            break;

        case MMP_FOURCC_IMAGE_NV12: 
        case MMP_FOURCC_IMAGE_NV21: 
            if(buftype == MMP_BUFTYPE_HW) uheight = V4L2_VIDEO_NV12_UV_HEIGHT_ALIGN(pic_height); 
            else uheight = V4L2_VIDEO_SW_NV12_UV_HEIGHT_ALIGN(pic_height); 
            break;
    }
    return uheight;    
}


MMP_S32 mmp_buffer_videoframe::s_get_y_offset(enum MMP_FOURCC fourcc, MMP_S32 pic_width, MMP_S32 pic_height, enum MMP_BUFTYPE buftype) {

    return 0;
}

MMP_S32 mmp_buffer_videoframe::s_get_u_offset(enum MMP_FOURCC fourcc, MMP_S32 pic_width, MMP_S32 pic_height, enum MMP_BUFTYPE buftype) {
    
    MMP_S32 uoff = 0;

    switch(fourcc) {
        case MMP_FOURCC_IMAGE_YUV420: 
            if(buftype == MMP_BUFTYPE_HW) uoff = V4L2_VIDEO_YUV420_U_FRAME_OFFSET(pic_width, pic_height); 
            else uoff = V4L2_VIDEO_SW_YUV420_U_FRAME_OFFSET(pic_width, pic_height); 
            break;
        case MMP_FOURCC_IMAGE_YVU420: 
            if(buftype == MMP_BUFTYPE_HW) uoff = V4L2_VIDEO_YVU420_U_FRAME_OFFSET(pic_width, pic_height); 
            else uoff = V4L2_VIDEO_SW_YVU420_U_FRAME_OFFSET(pic_width, pic_height); 
            break;

        case MMP_FOURCC_IMAGE_NV12: 
        case MMP_FOURCC_IMAGE_NV21: 
            if(buftype == MMP_BUFTYPE_HW) uoff = V4L2_VIDEO_NV12_UV_FRAME_OFFSET(pic_width, pic_height); 
            else uoff = V4L2_VIDEO_SW_NV12_UV_FRAME_OFFSET(pic_width, pic_height); 
            break;
    }

    return uoff;
}

MMP_S32 mmp_buffer_videoframe::s_get_v_offset(enum MMP_FOURCC fourcc, MMP_S32 pic_width, MMP_S32 pic_height, enum MMP_BUFTYPE buftype) {
    
    MMP_S32 voff = 0;

    switch(fourcc) {
        case MMP_FOURCC_IMAGE_YUV420: 
            if(buftype == MMP_BUFTYPE_HW) voff = V4L2_VIDEO_YUV420_V_FRAME_OFFSET(pic_width, pic_height); 
            else voff = V4L2_VIDEO_SW_YUV420_V_FRAME_OFFSET(pic_width, pic_height); 
            break;

        case MMP_FOURCC_IMAGE_YVU420: 
            if(buftype == MMP_BUFTYPE_HW) voff = V4L2_VIDEO_YVU420_V_FRAME_OFFSET(pic_width, pic_height); 
            else voff = V4L2_VIDEO_SW_YVU420_V_FRAME_OFFSET(pic_width, pic_height); 
            break;

        case MMP_FOURCC_IMAGE_NV12: 
        case MMP_FOURCC_IMAGE_NV21: 
            if(buftype == MMP_BUFTYPE_HW) voff = V4L2_VIDEO_NV12_UV_FRAME_OFFSET(pic_width, pic_height); 
            else voff = V4L2_VIDEO_SW_NV12_UV_FRAME_OFFSET(pic_width, pic_height); 
            break;
    }

    return voff;
}

MMP_RESULT mmp_buffer_videoframe::csc_YUV420toYUV420(class mmp_buffer_videoframe *p_buf_vf_src, class mmp_buffer_videoframe *p_buf_vf_dst) {

    MMP_U8* image_src[3];
    MMP_U8* image_dst[3];
    MMP_U8 *line_src, *line_dst;
    MMP_S32 stride_src, stride_dst, stride;
    MMP_S32 i, h;
    MMP_S32 h_dst;
    
    image_src[0] = (MMP_U8*)p_buf_vf_src->get_buf_vir_addr_y();
    image_src[1] = (MMP_U8*)p_buf_vf_src->get_buf_vir_addr_u();
    image_src[2] = (MMP_U8*)p_buf_vf_src->get_buf_vir_addr_v();
    
    image_dst[0] = (MMP_U8*)p_buf_vf_dst->get_buf_vir_addr_y();
    image_dst[1] = (MMP_U8*)p_buf_vf_dst->get_buf_vir_addr_u();
    image_dst[2] = (MMP_U8*)p_buf_vf_dst->get_buf_vir_addr_v();
    
    for(i = 0; i < 3; i++) {

        line_src = image_src[i];
        line_dst = image_dst[i];
               
        stride_src = p_buf_vf_src->get_buf_stride(i);
        stride_dst = p_buf_vf_dst->get_buf_stride(i);
        if(stride_src>stride_dst) stride = stride_dst;
        else  stride = stride_src;

        h_dst = p_buf_vf_dst->get_buf_height(i);

        for(h = 0; h < h_dst; h++) {
            memcpy(line_dst, line_src, stride);
            line_src += stride_src;
            line_dst += stride_dst;
        }

    }
    return MMP_SUCCESS;
}
