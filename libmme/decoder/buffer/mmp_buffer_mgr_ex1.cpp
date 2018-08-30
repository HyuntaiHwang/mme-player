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

#include "mmp_buffer_mgr_ex1.hpp"
#include "MmpUtil.hpp"
#include "mmp_lock.hpp"

mmp_buffer_mgr_ex1::mmp_buffer_mgr_ex1() :
m_p_mutex(NULL)
{

}

mmp_buffer_mgr_ex1::~mmp_buffer_mgr_ex1() {

}

MMP_RESULT mmp_buffer_mgr_ex1::open() {

    MMP_RESULT mmpResult = MMP_SUCCESS;

    MMPDEBUGMSG(1, (TEXT("[mmp_buffer_mgr_ex1::open]  ")));

    m_p_mutex = mmp_oal_mutex::create_object();
    if(m_p_mutex == NULL) {
        mmpResult = MMP_FAILURE;    
    }

    return mmpResult;
}

MMP_RESULT mmp_buffer_mgr_ex1::close() {

    bool flag;
    class mmp_buffer* p_mmp_buffer;

    flag = m_list_buffer.GetFirst(p_mmp_buffer);
    while(flag) {
        mmp_buffer::destroy_object(p_mmp_buffer);
        flag = m_list_buffer.GetNext(p_mmp_buffer);
    }
    m_list_buffer.Del_All();

    if(m_p_mutex != NULL) {
        mmp_oal_mutex::destroy_object(m_p_mutex);
        m_p_mutex = NULL;
    }

    MMPDEBUGMSG(1, (TEXT("[mmp_buffer_mgr_ex1::close]  ")));

    return MMP_SUCCESS;
}

class mmp_buffer* mmp_buffer_mgr_ex1::alloc_dma_buffer(MMP_S32 buffer_size) {

    class mmp_lock autolock(m_p_mutex);

    struct mmp_buffer_create_config buffer_create_config;
    class mmp_buffer* p_mmp_buffer;

    memset(&buffer_create_config, 0x00, sizeof(buffer_create_config));
    buffer_create_config.type = mmp_buffer::ION;
    buffer_create_config.size = buffer_size;

    p_mmp_buffer = mmp_buffer::create_object(&buffer_create_config);
    if(p_mmp_buffer != NULL) {
        m_list_buffer.Add(p_mmp_buffer);
    }

    return p_mmp_buffer;
}

class mmp_buffer* mmp_buffer_mgr_ex1::attach_dma_buffer(class mmp_buffer_addr buf_addr) {

    class mmp_lock autolock(m_p_mutex);

    struct mmp_buffer_create_config buffer_create_config;
    class mmp_buffer* p_mmp_buffer;

    buffer_create_config.type = mmp_buffer::ION_ATTACH;
    buffer_create_config.size = buf_addr.m_size;
    buffer_create_config.attach_shared_fd = buf_addr.m_shared_fd;
    buffer_create_config.attach_phy_addr = buf_addr.m_phy_addr;
    buffer_create_config.attach_offset = 0;

    p_mmp_buffer = mmp_buffer::create_object(&buffer_create_config);
    if(p_mmp_buffer != NULL) {
        m_list_buffer.Add(p_mmp_buffer);
    }

    return p_mmp_buffer;
}

MMP_RESULT mmp_buffer_mgr_ex1::free_buffer(class mmp_buffer* p_mmp_buffer) {

    class mmp_lock autolock(m_p_mutex);


    MMP_RESULT mmpResult = MMP_FAILURE;
    bool flag;
    class mmp_buffer* p_mmp_buffer_temp;
    MMP_S32 idx;

    idx = 0;
    flag = m_list_buffer.GetFirst(p_mmp_buffer_temp);
    while(flag) {
        
        if(p_mmp_buffer_temp == p_mmp_buffer) {
             mmp_buffer::destroy_object(p_mmp_buffer);
             mmpResult = MMP_SUCCESS;
             break;
        }
        flag = m_list_buffer.GetNext(p_mmp_buffer_temp);
        idx++;
    }
    m_list_buffer.Del(idx);
    
    return MMP_FAILURE;
}

MMP_RESULT mmp_buffer_mgr_ex1::free_buffer(class mmp_buffer_addr buf_addr) {

    class mmp_lock autolock(m_p_mutex);


    MMP_RESULT mmpResult = MMP_FAILURE;
    bool flag;
    class mmp_buffer* p_mmp_buffer_temp;
    MMP_S32 idx;

    idx = 0;
    flag = m_list_buffer.GetFirst(p_mmp_buffer_temp);
    while(flag) {
        
        if( (p_mmp_buffer_temp->get_phy_addr() == buf_addr.m_phy_addr) 
            && (p_mmp_buffer_temp->get_vir_addr() == buf_addr.m_vir_addr) 
            )
        {
                         
             mmp_buffer::destroy_object(p_mmp_buffer_temp);
             mmpResult = MMP_SUCCESS;
             break;
        }
        flag = m_list_buffer.GetNext(p_mmp_buffer_temp);
        idx++;
    }
    m_list_buffer.Del(idx);
    
    return MMP_FAILURE;
}

class mmp_buffer* mmp_buffer_mgr_ex1::get_buffer(MMP_S32 shared_fd) {

    class mmp_lock autolock(m_p_mutex);

    MMP_RESULT mmpResult = MMP_FAILURE;
    bool flag;
    class mmp_buffer* p_mmp_buffer = NULL;
    class mmp_buffer* p_mmp_buffer_temp;

    flag = m_list_buffer.GetFirst(p_mmp_buffer_temp);
    while(flag) {
        
        if(p_mmp_buffer_temp->get_buf_shared_fd() == shared_fd) {
             p_mmp_buffer = p_mmp_buffer_temp;
             break;
        }
        flag = m_list_buffer.GetNext(p_mmp_buffer_temp);
    }
    
    return p_mmp_buffer;
}

class mmp_buffer_addr mmp_buffer_mgr_ex1::get_buffer_addr(MMP_S32 shared_fd) {

    class mmp_buffer* p_mmp_buf;
    class mmp_buffer_addr buf_addr;

    p_mmp_buf = this->get_buffer(shared_fd);
    if(p_mmp_buf != NULL) {
        buf_addr = p_mmp_buf->get_buf_addr();
    }

    return buf_addr;
}

class mmp_buffer_videoframe* mmp_buffer_mgr_ex1::alloc_media_videoframe(MMP_MEDIA_ID producer_id, MMP_S32 pic_width, MMP_S32 pic_height, enum MMP_FOURCC fourcc, 
                                                                        MMP_U32 type, MMP_S32 *shared_ion_fd, MMP_S32 *ion_mem_offset, enum mmp_buffer::ION_HEAP ion_heap, 
                                                                        MMP_U32 buf_flag ) {

    class mmp_buffer* p_mmp_buf;
    struct mmp_buffer_create_config buffer_create_config;
    MMP_S32 err_cnt = 0;
    
    class mmp_buffer_videoframe* p_mmp_videoframe = NULL;
    MMP_RESULT mmpResult = MMP_SUCCESS;
    MMP_S32 plane_size[MMP_IMAGE_MAX_PLANE_COUNT]= {0};
    MMP_S32 stride[MMP_IMAGE_MAX_PLANE_COUNT] = {0};
    MMP_S32 buffer_height_arr[MMP_IMAGE_MAX_PLANE_COUNT] = {0};
    MMP_S32 buffer_width, buffer_height, luma_size, chroma_size;
    MMP_S32 i;
    enum MMP_BUFTYPE buftype = MMP_BUFTYPE_SW;

    if( (type == mmp_buffer::ION) 
        || (type == mmp_buffer::ION_ATTACH) ) {
        buftype = MMP_BUFTYPE_HW;
    }

    if(type == mmp_buffer::ION_ATTACH) {
        if( (shared_ion_fd == NULL) || (ion_mem_offset == NULL) ) {
            /* Error  */
            err_cnt++;
        }
    }

    if(err_cnt == 0) {
        
        p_mmp_videoframe = new class mmp_buffer_videoframe(producer_id, buftype);
        if(p_mmp_videoframe != NULL) {
        
            p_mmp_videoframe->m_fourcc = fourcc;
            p_mmp_videoframe->m_pic_width = pic_width;
            p_mmp_videoframe->m_pic_height = pic_height;
            
            switch(fourcc) {

                
                case MMP_FOURCC_IMAGE_YUV420M:  /* V4L2_PIX_FMT_YUV420M 12  YUV 4:2:0  3P non contiguous planes - Y, Cb, Cr */
                    p_mmp_videoframe->m_plane_count = 3;        

                    buffer_width = MMP_VIDEO_FRAME_STRIDE_ALIGN(p_mmp_videoframe->m_pic_width);
                    buffer_height = MMP_VIDEO_FRAME_HEIGHT_ALIGN(p_mmp_videoframe->m_pic_height);
                    
                    stride[0] = buffer_width;
                    stride[1] = buffer_width>>1;
                    stride[2] = buffer_width>>1;
                
                    buffer_height_arr[0] = buffer_height;
                    buffer_height_arr[1] = buffer_height>>1;
                    buffer_height_arr[2] = buffer_height>>1;
                    
                    luma_size = buffer_width*buffer_height;
                    chroma_size = luma_size>>2;

                    plane_size[0] = luma_size;
                    plane_size[1] = chroma_size;
                    plane_size[2] = chroma_size;

                    break;
                    
                    
                
                case MMP_FOURCC_IMAGE_YUV420M_META:  /* YUV420M Meta Data */
                    type = mmp_buffer::META;
                
                case MMP_FOURCC_IMAGE_YUV420: /* V4L2_PIX_FMT_YVU420 12  YVU 4:2:0  3P contigous */
                case MMP_FOURCC_IMAGE_YVU420: /* V4L2_PIX_FMT_YVU420 12  YVU 4:2:0  3P contigous */
                case MMP_FOURCC_IMAGE_NV12:
                case MMP_FOURCC_IMAGE_NV21:
                    p_mmp_videoframe->m_plane_count = 1;        
                    stride[0] = mmp_buffer_videoframe::s_get_y_stride(p_mmp_videoframe->m_fourcc, pic_width, buftype);
                    buffer_height_arr[0] = mmp_buffer_videoframe::s_get_y_height(p_mmp_videoframe->m_fourcc, pic_height, buftype);

                    stride[1] = mmp_buffer_videoframe::s_get_u_stride(p_mmp_videoframe->m_fourcc, pic_width, buftype);
                    buffer_height_arr[1] = mmp_buffer_videoframe::s_get_u_height(p_mmp_videoframe->m_fourcc, pic_height, buftype);

                    stride[2] = mmp_buffer_videoframe::s_get_v_stride(p_mmp_videoframe->m_fourcc, pic_width, buftype);
                    buffer_height_arr[2] = mmp_buffer_videoframe::s_get_v_height(p_mmp_videoframe->m_fourcc, pic_height, buftype);

                    plane_size[0] = mmp_buffer_videoframe::s_get_size(p_mmp_videoframe->m_fourcc, pic_width, pic_height, buftype);
                    break;
                                    
                case MMP_FOURCC_IMAGE_NV12MT:
                    
                    p_mmp_videoframe->m_plane_count = 2;        

                    buffer_width = MMP_VIDEO_FRAME_STRIDE_ALIGN(p_mmp_videoframe->m_pic_width);
                    buffer_height = MMP_VIDEO_FRAME_HEIGHT_ALIGN(p_mmp_videoframe->m_pic_height);
                    
                    stride[0] = buffer_width;
                    stride[1] = buffer_width;
                    
                    buffer_height_arr[0] = buffer_height;
                    buffer_height_arr[1] = buffer_height;
                    
                    luma_size = buffer_width*buffer_height;
                    chroma_size = luma_size;

                    plane_size[0] = luma_size;
                    plane_size[1] = chroma_size;
                    
                    break;

                case MMP_FOURCC_IMAGE_BGR888:
                case MMP_FOURCC_IMAGE_RGB888:
                case MMP_FOURCC_IMAGE_ABGR8888:
                case MMP_FOURCC_IMAGE_ARGB8888:
                    p_mmp_videoframe->m_plane_count = 1;        
                    stride[0] = mmp_buffer_videoframe::s_get_stride(p_mmp_videoframe->m_fourcc, pic_width, buftype);
                    buffer_height_arr[0] = mmp_buffer_videoframe::s_get_height(p_mmp_videoframe->m_fourcc, pic_height, buftype);

                    stride[1] = 0;
                    buffer_height_arr[1] = 0;

                    stride[2] = 0;
                    buffer_height_arr[2] = 0;

                    plane_size[0] = mmp_buffer_videoframe::s_get_size(p_mmp_videoframe->m_fourcc, pic_width, pic_height, buftype);
                    break;

                default:
                    mmpResult = MMP_FAILURE;
                    MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[mmp_buffer_mgr_ex1::attach_media_videoframe] FAIL: not supported video format %c%c%c%c "), MMPGETFOURCCARG(p_mmp_videoframe->m_fourcc) ));
                    break;
            }

            /* alloc mmp buffer per plane */
            if(mmpResult == MMP_SUCCESS) {

                for(i = 0; i < p_mmp_videoframe->m_plane_count; i++) {
                    
                    buffer_create_config.type = type;
                    buffer_create_config.size = plane_size[i];

                    if(type == mmp_buffer::ION_ATTACH) {
                        buffer_create_config.attach_shared_fd = shared_ion_fd[i];
                        buffer_create_config.attach_phy_addr = 0;
                        buffer_create_config.attach_offset = ion_mem_offset[i];
                    }
                    buffer_create_config.flag = buf_flag;
                    buffer_create_config.ion_heap_type = ion_heap;

                    p_mmp_buf = mmp_buffer::create_object(&buffer_create_config);
                    if(p_mmp_buf == NULL) {
                        mmpResult = MMP_FAILURE;
                        break;
                    }
                    else {
                        p_mmp_videoframe->m_p_mmp_buffer[i] = p_mmp_buf;
                        
                    }
                }

                if(mmpResult == MMP_SUCCESS) {
                    for(i = 0; i < MMP_IMAGE_MAX_PLANE_COUNT; i++) {
                        p_mmp_videoframe->m_buf_stride[i] = stride[i];
                        p_mmp_videoframe->m_buf_height[i] = buffer_height_arr[i];
                    }
                }


            }

        }
        else {
            /* FAIL: new instance */
            mmpResult = MMP_FAILURE;
        }

        if(mmpResult == MMP_SUCCESS) {
        
            m_p_mutex->lock();

            /* register mmp buffer */
            for(i = 0; i < p_mmp_videoframe->m_plane_count; i++) {
                m_list_buffer.Add(p_mmp_videoframe->m_p_mmp_buffer[i]);
            }

            m_p_mutex->unlock();

        }
        else {
            
            if(p_mmp_videoframe != NULL) {
        
                for(i = 0; i < p_mmp_videoframe->m_plane_count; i++) {
                    if(p_mmp_videoframe->m_p_mmp_buffer[i] != NULL) {
                        mmp_buffer::destroy_object(p_mmp_videoframe->m_p_mmp_buffer[i]);
                    }
                }

                delete p_mmp_videoframe;
                p_mmp_videoframe = NULL;
            }
        }

    }
    
    return p_mmp_videoframe;
}

class mmp_buffer_videoframe* mmp_buffer_mgr_ex1::alloc_media_videoframe(MMP_MEDIA_ID producer_id, MMP_S32 pic_width, MMP_S32 pic_height, enum MMP_FOURCC fourcc, MMP_U32 buf_type, MMP_U32 buf_flag) {
    
    return this->alloc_media_videoframe(producer_id, pic_width, pic_height, fourcc, buf_type, NULL, NULL, mmp_buffer::ION_HEAP_DEFAULT, buf_flag);
}

class mmp_buffer_videoframe* mmp_buffer_mgr_ex1::attach_media_videoframe(struct mmp_buffer_videoframe::ion_attach_config* p_config) {
    
    class mmp_buffer* p_mmp_buf;
    struct mmp_buffer_create_config buffer_create_config;
    MMP_S32 err_cnt = 0;
    
    class mmp_buffer_videoframe* p_mmp_videoframe = NULL;
    MMP_RESULT mmpResult = MMP_SUCCESS;
    MMP_S32 i;
    
    if(err_cnt == 0) {
        p_mmp_videoframe = new class mmp_buffer_videoframe(p_config->producer_id, MMP_BUFTYPE_HW);
        if(p_mmp_videoframe != NULL) {
        
            p_mmp_videoframe->m_fourcc = p_config->fourcc;
            p_mmp_videoframe->m_pic_width = p_config->pic_width;
            p_mmp_videoframe->m_pic_height = p_config->pic_height;
            p_mmp_videoframe->m_plane_count = p_config->plane_count;
                        
            /* alloc mmp buffer per plane */
            for(i = 0; i < p_mmp_videoframe->m_plane_count; i++) {
                    
                buffer_create_config.type = mmp_buffer::ION_ATTACH;
                buffer_create_config.size = p_config->plane_size[i];

                //if(type == mmp_buffer::ION_ATTACH) {
                buffer_create_config.attach_shared_fd = p_config->shared_fd[i];
                buffer_create_config.attach_phy_addr = 0;
                buffer_create_config.attach_offset = p_config->ion_mem_offset[i];
                //}
                buffer_create_config.flag = p_config->ion_flag;
                buffer_create_config.ion_heap_type = p_config->ion_heap;

                p_mmp_buf = mmp_buffer::create_object(&buffer_create_config);
                if(p_mmp_buf == NULL) {
                    mmpResult = MMP_FAILURE;
                    break;
                }
                else {
                    p_mmp_videoframe->m_p_mmp_buffer[i] = p_mmp_buf;
                }
            }

            if(mmpResult == MMP_SUCCESS) {
                for(i = 0; i < MMP_IMAGE_MAX_PLANE_COUNT; i++) {
                    p_mmp_videoframe->m_buf_stride[i] = p_config->stride[i];
                    p_mmp_videoframe->m_buf_height[i] = p_config->alignheight[i];
                }
            }
        }
        else {
            /* FAIL: new instance */
            mmpResult = MMP_FAILURE;
        }

        if(mmpResult == MMP_SUCCESS) {
        
            m_p_mutex->lock();

            /* register mmp buffer */
            for(i = 0; i < p_mmp_videoframe->m_plane_count; i++) {
                m_list_buffer.Add(p_mmp_videoframe->m_p_mmp_buffer[i]);
            }

            m_p_mutex->unlock();

        }
        else {
            
            if(p_mmp_videoframe != NULL) {
        
                for(i = 0; i < p_mmp_videoframe->m_plane_count; i++) {
                    if(p_mmp_videoframe->m_p_mmp_buffer[i] != NULL) {
                        mmp_buffer::destroy_object(p_mmp_videoframe->m_p_mmp_buffer[i]);
                    }
                }

                delete p_mmp_videoframe;
                p_mmp_videoframe = NULL;
            }
        }

    }
    
    return p_mmp_videoframe;
}

class mmp_buffer_videoframe* mmp_buffer_mgr_ex1::attach_media_videoframe(MMP_MEDIA_ID producer_id, 
                                                                         MMP_S32 *shared_ion_fd, MMP_S32 *ion_mem_offset, enum mmp_buffer::ION_HEAP ion_heap,
                                                                         MMP_S32 pic_width, MMP_S32 pic_height, 
                                                                         enum MMP_FOURCC fourcc) {

    return this->alloc_media_videoframe(producer_id, pic_width, pic_height, fourcc, mmp_buffer::ION_ATTACH, shared_ion_fd, ion_mem_offset, ion_heap, mmp_buffer::FLAG_NONE);
}

/* Android Native Window Buffer */
class mmp_buffer_videoframe* mmp_buffer_mgr_ex1::attach_media_videoframe_ANW(MMP_MEDIA_ID producer_id, MMP_ADDR anw_addr, MMP_S32 pic_width, MMP_S32 pic_height, enum MMP_FOURCC fourcc) {


    class mmp_buffer* p_mmp_buf;
    struct mmp_buffer_create_config buffer_create_config;
    MMP_S32 err_cnt = 0;

    class mmp_buffer_videoframe* p_mmp_videoframe = NULL;
    MMP_RESULT mmpResult = MMP_SUCCESS;
    MMP_S32 i, buffer_width, buffer_height;
    
    if(err_cnt == 0) {
        
        p_mmp_videoframe = new class mmp_buffer_videoframe(producer_id, MMP_BUFTYPE_HW);
        if(p_mmp_videoframe != NULL) {

            p_mmp_videoframe->m_fourcc = fourcc;
            p_mmp_videoframe->m_pic_width = pic_width;
            p_mmp_videoframe->m_pic_height = pic_height;
            p_mmp_videoframe->m_plane_count = 1;

            buffer_width = MMP_VIDEO_FRAME_STRIDE_ALIGN(pic_width);
            buffer_height = MMP_VIDEO_FRAME_HEIGHT_ALIGN(pic_height);

            buffer_create_config.type = mmp_buffer::ANDROID_NATIVE_WINDOW_ATTACH;
            buffer_create_config.size = buffer_width * buffer_height;
            buffer_create_config.attach_vir_addr = anw_addr;
            buffer_create_config.attach_shared_fd = -1;
            buffer_create_config.attach_phy_addr = 0;
            buffer_create_config.attach_offset = 0;

            p_mmp_buf = mmp_buffer::create_object(&buffer_create_config);
            if(p_mmp_buf == NULL) {
                mmpResult = MMP_FAILURE;
            }
            else {
                p_mmp_videoframe->m_p_mmp_buffer[0] = p_mmp_buf;
                p_mmp_videoframe->m_buf_stride[0] = buffer_width;
                p_mmp_videoframe->m_buf_height[0] = buffer_height;
            }
        }
        else {
            /* FAIL: new instance */
            mmpResult = MMP_FAILURE;
        }

        if(mmpResult == MMP_SUCCESS) {
        
            m_p_mutex->lock();

            /* register mmp buffer */
            for(i = 0; i < p_mmp_videoframe->m_plane_count; i++) {
                m_list_buffer.Add(p_mmp_videoframe->m_p_mmp_buffer[i]);
            }

            m_p_mutex->unlock();

        }
        else {
            
            if(p_mmp_videoframe != NULL) {
        
                for(i = 0; i < p_mmp_videoframe->m_plane_count; i++) {
                    if(p_mmp_videoframe->m_p_mmp_buffer[i] != NULL) {
                        mmp_buffer::destroy_object(p_mmp_videoframe->m_p_mmp_buffer[i]);
                    }
                }

                delete p_mmp_videoframe;
                p_mmp_videoframe = NULL;
            }
        }

    }
    
    return p_mmp_videoframe;
}

/* Android Native Window Buffer */
class mmp_buffer_videoframe* mmp_buffer_mgr_ex1::attach_media_videoframe(MMP_MEDIA_ID producer_id, MMP_ADDR addr, MMP_S32 pic_width, MMP_S32 pic_height, enum MMP_FOURCC fourcc, enum MMP_BUFTYPE buftype) {

    class mmp_buffer_videoframe* p_buf_vf = NULL;

    p_buf_vf = this->attach_media_videoframe(producer_id, 
                                        addr, 
                                        addr + mmp_buffer_videoframe::s_get_u_offset(fourcc, pic_width, pic_height, buftype), 
                                        addr + mmp_buffer_videoframe::s_get_v_offset(fourcc, pic_width, pic_height, buftype), 
                                        mmp_buffer_videoframe::s_get_y_stride(fourcc, pic_width, buftype), mmp_buffer_videoframe::s_get_u_stride(fourcc, pic_width, buftype), mmp_buffer_videoframe::s_get_v_stride(fourcc, pic_width, buftype),
                                        mmp_buffer_videoframe::s_get_y_height(fourcc, pic_height, buftype), mmp_buffer_videoframe::s_get_u_height(fourcc, pic_height, buftype), mmp_buffer_videoframe::s_get_v_height(fourcc, pic_height, buftype),
                                        pic_width, pic_height, fourcc, buftype );
    
    return p_buf_vf;
}

class mmp_buffer_videoframe* mmp_buffer_mgr_ex1::attach_media_videoframe(MMP_MEDIA_ID producer_id, 
                                                                         MMP_ADDR y_addr, MMP_ADDR u_addr, MMP_ADDR v_addr, 
                                                                         MMP_S32 y_stride, MMP_S32 u_stride, MMP_S32 v_stride,
                                                                         MMP_S32 y_alignheight, MMP_S32 u_alignheight, MMP_S32 v_alignheight,
                                                                         MMP_S32 pic_width, MMP_S32 pic_height, enum MMP_FOURCC fourcc,
                                                                         enum MMP_BUFTYPE buftype
                                                                         ) {

    class mmp_buffer* p_mmp_buf;
    struct mmp_buffer_create_config buffer_create_config;
    MMP_S32 err_cnt = 0;
    
    class mmp_buffer_videoframe* p_mmp_videoframe = NULL;
    MMP_RESULT mmpResult = MMP_SUCCESS;
    MMP_ADDR vir_addr[MMP_IMAGE_MAX_PLANE_COUNT];
    MMP_S32 stride[MMP_IMAGE_MAX_PLANE_COUNT];
    MMP_S32 buffer_height_arr[MMP_IMAGE_MAX_PLANE_COUNT];
    MMP_S32 i;
    
    if(err_cnt == 0) {
        
        p_mmp_videoframe = new class mmp_buffer_videoframe(producer_id, buftype);
        if(p_mmp_videoframe != NULL) {
        
            p_mmp_videoframe->m_fourcc = fourcc;
            p_mmp_videoframe->m_pic_width = pic_width;
            p_mmp_videoframe->m_pic_height = pic_height;

            switch(fourcc) {

                case MMP_FOURCC_IMAGE_YUV420M: 
                    p_mmp_videoframe->m_plane_count = 3;        

                    vir_addr[0] = y_addr;
                    vir_addr[1] = u_addr;
                    vir_addr[2] = v_addr;

                    stride[0] = y_stride;
                    stride[1] = u_stride;
                    stride[2] = v_stride;
                
                    buffer_height_arr[0] = y_alignheight;
                    buffer_height_arr[1] = u_alignheight;
                    buffer_height_arr[2] = v_alignheight;
                    break;

                case MMP_FOURCC_IMAGE_YUV420: 
                case MMP_FOURCC_IMAGE_YVU420: 

                case MMP_FOURCC_IMAGE_ABGR8888: 
                case MMP_FOURCC_IMAGE_ARGB8888: 
                case MMP_FOURCC_IMAGE_BGR888: 
                case MMP_FOURCC_IMAGE_RGB888: 
                
                    p_mmp_videoframe->m_plane_count = 1;

                    vir_addr[0] = y_addr;
                    vir_addr[1] = u_addr;
                    vir_addr[2] = v_addr;

                    stride[0] = y_stride;
                    stride[1] = u_stride;
                    stride[2] = v_stride;
                
                    buffer_height_arr[0] = y_alignheight;
                    buffer_height_arr[1] = u_alignheight;
                    buffer_height_arr[2] = v_alignheight;
                    
                    break;
                    
                case MMP_FOURCC_IMAGE_ANDROID_OPAQUE:

                    p_mmp_videoframe->m_plane_count = 1;

                    vir_addr[0] = y_addr;
                    vir_addr[1] = 0;
                    vir_addr[2] = 0;

                    stride[0] = MMP_IMAGE_ANDROID_OPAQUE_SIZE;
                    stride[1] = 0;
                    stride[2] = 0;
                
                    buffer_height_arr[0] = 1;
                    buffer_height_arr[1] = 0;
                    buffer_height_arr[2] = 0;
                    
                    break;

                default:
                    mmpResult = MMP_FAILURE;
                    MMPDEBUGMSG(MMPZONE_ERROR, (TEXT(MMP_CTAG" FAIL: not supported video format %c%c%c%c "), MMP_CNAME, MMPGETFOURCCARG(p_mmp_videoframe->m_fourcc) ));
                    break;
            }

            /* alloc mmp buffer per plane */
            if(mmpResult == MMP_SUCCESS) {

                for(i = 0; i < p_mmp_videoframe->m_plane_count; i++) {
                    
                    buffer_create_config.type = mmp_buffer::HEAP_ATTACH;
                    buffer_create_config.size = stride[i]*buffer_height_arr[i];
                    buffer_create_config.attach_vir_addr = vir_addr[i];
                    buffer_create_config.attach_shared_fd = -1;
                    buffer_create_config.attach_phy_addr = 0;
                    buffer_create_config.attach_offset = 0;

                    p_mmp_buf = mmp_buffer::create_object(&buffer_create_config);
                    if(p_mmp_buf == NULL) {
                        mmpResult = MMP_FAILURE;
                        break;
                    }
                    else {
                        p_mmp_videoframe->m_p_mmp_buffer[i] = p_mmp_buf;
                        p_mmp_videoframe->m_buf_stride[i] = stride[i];
                        p_mmp_videoframe->m_buf_height[i] = buffer_height_arr[i];
                    }
                }

                for(i = 0; i < MMP_IMAGE_MAX_PLANE_COUNT; i++) {
                    p_mmp_videoframe->m_buf_stride[i] = stride[i];
                    p_mmp_videoframe->m_buf_height[i] = buffer_height_arr[i];
                }

            }

        }
        else {
            /* FAIL: new instance */
            mmpResult = MMP_FAILURE;
        }

        if(mmpResult == MMP_SUCCESS) {
        
            m_p_mutex->lock();

            /* register mmp buffer */
            for(i = 0; i < p_mmp_videoframe->m_plane_count; i++) {
                m_list_buffer.Add(p_mmp_videoframe->m_p_mmp_buffer[i]);
            }

            m_p_mutex->unlock();

        }
        else {
            
            if(p_mmp_videoframe != NULL) {
        
                for(i = 0; i < p_mmp_videoframe->m_plane_count; i++) {
                    if(p_mmp_videoframe->m_p_mmp_buffer[i] != NULL) {
                        mmp_buffer::destroy_object(p_mmp_videoframe->m_p_mmp_buffer[i]);
                    }
                }

                delete p_mmp_videoframe;
                p_mmp_videoframe = NULL;
            }
        }

    }
    
    return p_mmp_videoframe;
}
    

class mmp_buffer_videostream* mmp_buffer_mgr_ex1::alloc_media_videostream(MMP_MEDIA_ID producer_id, MMP_S32 stream_max_size, MMP_U32 buf_type, MMP_U8* p_stream_data) {

    MMP_S32 err_cnt = 0;
    class mmp_buffer* p_mmp_buf;
    struct mmp_buffer_create_config buffer_create_config;
    class mmp_buffer_videostream* p_mmp_videostream = NULL;

    if(buf_type == mmp_buffer::HEAP_ATTACH) {
        if(p_stream_data==NULL) {
            err_cnt++;        
        }
    }
        
    if(err_cnt == 0) {
        p_mmp_videostream = new class mmp_buffer_videostream(producer_id);
        if(p_mmp_videostream != NULL) {

            memset(&buffer_create_config, 0x00, sizeof(buffer_create_config));
                
            buffer_create_config.type = buf_type;
            buffer_create_config.size = stream_max_size + MMP_VIDEO_STREAM_BUF_EXTRA_SIZE;

            if(buf_type == mmp_buffer::HEAP_ATTACH) {
                buffer_create_config.attach_vir_addr = (MMP_U32)p_stream_data;
                buffer_create_config.attach_offset = 0;
            }

            p_mmp_buf = mmp_buffer::create_object(&buffer_create_config);
            if(p_mmp_buf != NULL) {

                m_p_mutex->lock();
        
                p_mmp_videostream->m_p_mmp_buffer = p_mmp_buf;
                m_list_buffer.Add(p_mmp_videostream->m_p_mmp_buffer);
                
                m_p_mutex->unlock();
            }
        }
    }
    
    return p_mmp_videostream;
}

class mmp_buffer_videostream* mmp_buffer_mgr_ex1::alloc_media_videostream(MMP_MEDIA_ID producer_id, MMP_S32 stream_max_size, MMP_U32 buf_type) {

    return this->alloc_media_videostream(producer_id, stream_max_size, buf_type, NULL);
}

class mmp_buffer_videostream* mmp_buffer_mgr_ex1::attach_media_videostream(MMP_MEDIA_ID producer_id, MMP_U8* p_stream_data, MMP_S32 stream_size) {
    return this->alloc_media_videostream(producer_id, stream_size, mmp_buffer::HEAP_ATTACH, p_stream_data);
}

class mmp_buffer_imagestream* mmp_buffer_mgr_ex1::alloc_media_imagestream(MMP_MEDIA_ID producer_id, MMP_S32 stream_max_size, MMP_U32 buf_type) {

    MMP_S32 err_cnt = 0;
    class mmp_buffer* p_mmp_buf;
    struct mmp_buffer_create_config buffer_create_config;
    class mmp_buffer_imagestream* p_mmp_imagestream = NULL;

    //if(buf_type == mmp_buffer::HEAP_ATTACH) {
    //    if(p_stream_data==NULL) {
    //        err_cnt++;        
    //    }
    //}
        
    if(err_cnt == 0) {
        p_mmp_imagestream = new class mmp_buffer_imagestream(producer_id);
        if(p_mmp_imagestream != NULL) {

            memset(&buffer_create_config, 0x00, sizeof(buffer_create_config));
                
            buffer_create_config.type = buf_type;
            buffer_create_config.size = stream_max_size;

            //if(buf_type == mmp_buffer::HEAP_ATTACH) {
            //    buffer_create_config.attach_vir_addr = (MMP_U32)p_stream_data;
            //    buffer_create_config.attach_offset = 0;
            //}

            p_mmp_buf = mmp_buffer::create_object(&buffer_create_config);
            if(p_mmp_buf != NULL) {

                m_p_mutex->lock();
        
                p_mmp_imagestream->m_p_mmp_buffer = p_mmp_buf;
                m_list_buffer.Add(p_mmp_imagestream->m_p_mmp_buffer);
                
                m_p_mutex->unlock();
            }
        }
    }
    
    return p_mmp_imagestream;
}

class mmp_buffer_imagestream* mmp_buffer_mgr_ex1::alloc_media_imagestream(MMP_MEDIA_ID producer_id, MMP_CHAR* image_file_name, MMP_U32 buf_type) {

    FILE* fp;
    class mmp_buffer_imagestream* p_buf_imagestream = NULL;
    MMP_S32 file_size, rdsz;

    fp = fopen(image_file_name, "rb");
    if(fp != NULL) {
        fseek(fp, 0, SEEK_END);
        file_size = ftell(fp);
        fseek(fp, 0, SEEK_SET);

        p_buf_imagestream = this->alloc_media_imagestream(producer_id, MMP_BYTE_ALIGN(file_size, 1024), buf_type);
        if(p_buf_imagestream != NULL) {
            rdsz = fread((void*)p_buf_imagestream->get_buf_vir_addr(), 1, file_size, fp);
            if(rdsz != file_size) {
                this->free_media_buffer(p_buf_imagestream);
                p_buf_imagestream = NULL;
            }
            else {
                p_buf_imagestream->sync_buf();
                p_buf_imagestream->set_stream_size(file_size);
                p_buf_imagestream->m_fp_imagefile = fp;
            }
        }
    }
    
    return p_buf_imagestream;
}

class mmp_buffer_imageframe* mmp_buffer_mgr_ex1::alloc_media_imageframe(MMP_MEDIA_ID producer_id, MMP_S32 pic_width, MMP_S32 pic_height, enum MMP_FOURCC fourcc, 
                                                                        MMP_U32 type, MMP_S32 *shared_ion_fd, MMP_S32 *ion_mem_offset) {

    class mmp_buffer* p_mmp_buf;
    struct mmp_buffer_create_config buffer_create_config;
    MMP_S32 err_cnt = 0;

    class mmp_buffer_imageframe* p_mmp_imageframe = NULL;
    MMP_RESULT mmpResult = MMP_SUCCESS;
    MMP_S32 buffer_stride[MMP_IMAGE_MAX_PLANE_COUNT];
    MMP_S32 buffer_height_arr[MMP_IMAGE_MAX_PLANE_COUNT];
    MMP_S32 i;

    if(type == mmp_buffer::ION_ATTACH) {
        if( (shared_ion_fd == NULL) || (ion_mem_offset == NULL) ) {
            /* Error  */
            err_cnt++;
        }
    }

    if(err_cnt == 0) {
        
        p_mmp_imageframe = new class mmp_buffer_imageframe(producer_id);
        if(p_mmp_imageframe != NULL) {
        
            p_mmp_imageframe->m_fourcc = fourcc;
            p_mmp_imageframe->m_pic_width = pic_width;
            p_mmp_imageframe->m_pic_height = pic_height;
            
            
            switch(p_mmp_imageframe->m_fourcc) {

                case MMP_FOURCC_IMAGE_YVU420M: 
                case MMP_FOURCC_IMAGE_YUV420M: 
                    p_mmp_imageframe->m_plane_count = 3;        

                    buffer_stride[0] = MMP_VIDEO_FRAME_STRIDE_ALIGN(p_mmp_imageframe->m_pic_width);
                      /* Note : gdm_dss suppose that chroma stride is half of luma stride */
                    buffer_stride[1] = buffer_stride[0]>>1;//MMP_VIDEO_FRAME_STRIDE_ALIGN(p_mmp_imageframe->m_pic_width/2);
                    buffer_stride[2] = buffer_stride[1];
                
                    buffer_height_arr[0] = MMP_VIDEO_FRAME_HEIGHT_ALIGN(p_mmp_imageframe->m_pic_height);
                    buffer_height_arr[1] = MMP_VIDEO_FRAME_HEIGHT_ALIGN(p_mmp_imageframe->m_pic_height/2);
                    buffer_height_arr[2] = buffer_height_arr[1];
                    
                    break;

#if 0
                case MMP_FOURCC_IMAGE_YCbCr422_P2:  /* 16 bit Y/CbCr 4:2:2 Plane 2, V4L2_PIX_FMT_NV16 */
                case MMP_FOURCC_IMAGE_YCrCb422_P2:  /* 16 bit Y/CrCb 4:2:2 Plane 2, V4L2_PIX_FMT_NV61 */ 
    
                    p_mmp_imageframe->m_plane_count = 2;        

                    buffer_stride[0] = MMP_VIDEO_FRAME_STRIDE_ALIGN(p_mmp_imageframe->m_pic_width);
                    buffer_stride[1] = buffer_stride[0];
                                    
                    buffer_height_arr[0] = MMP_VIDEO_FRAME_HEIGHT_ALIGN(p_mmp_imageframe->m_pic_height);
                    buffer_height_arr[1] = buffer_height_arr[0];
                    
                    break;
#endif
               
               case MMP_FOURCC_IMAGE_YUV444_P1:
                    p_mmp_imageframe->m_plane_count = 1;

                    buffer_stride[0] = MMP_VIDEO_FRAME_STRIDE_ALIGN(p_mmp_imageframe->m_pic_width*3);
                    buffer_height_arr[0] = MMP_VIDEO_FRAME_HEIGHT_ALIGN(p_mmp_imageframe->m_pic_height);
                    
                    break;

               case MMP_FOURCC_IMAGE_GREY:  /*  8  Greyscale     */
                    p_mmp_imageframe->m_plane_count = 1;

                    buffer_stride[0] = MMP_VIDEO_FRAME_STRIDE_ALIGN(p_mmp_imageframe->m_pic_width);
                    buffer_height_arr[0] = MMP_VIDEO_FRAME_HEIGHT_ALIGN(p_mmp_imageframe->m_pic_height);
                    
                    break;

               case MMP_FOURCC_IMAGE_ARGB8888:  /* RGB 32 Bit*/
                    p_mmp_imageframe->m_plane_count = 1;
                    
                    buffer_stride[0] = MMP_BYTE_ALIGN(p_mmp_imageframe->m_pic_width*4, 4);
                    buffer_height_arr[0] = MMP_VIDEO_FRAME_HEIGHT_ALIGN(p_mmp_imageframe->m_pic_height);
                    
                    break;

                case MMP_FOURCC_IMAGE_BGR888:
                case MMP_FOURCC_IMAGE_RGB888:  /* RGB 24 Bit*/
                    p_mmp_imageframe->m_plane_count = 1;
                    
                    buffer_stride[0] = MMP_BYTE_ALIGN(p_mmp_imageframe->m_pic_width*3, 4);
                    buffer_height_arr[0] = MMP_VIDEO_FRAME_HEIGHT_ALIGN(p_mmp_imageframe->m_pic_height);
                    
                    break;

                default:
                    mmpResult = MMP_FAILURE;
                    MMPDEBUGMSG(MMPZONE_ERROR, (TEXT(MMP_CTAG" FAIL: not supported video format %c%c%c%c "), MMP_CNAME,
                                              MMPGETFOURCCARG(p_mmp_imageframe->m_fourcc) ));
                    break;
            }

            /* alloc mmp buffer per plane */
            if(mmpResult == MMP_SUCCESS) {

                for(i = 0; i < p_mmp_imageframe->m_plane_count; i++) {
                    
                    buffer_create_config.type = type;
                    buffer_create_config.size = buffer_stride[i] * buffer_height_arr[i];

                    if(type == mmp_buffer::ION_ATTACH) {
                        buffer_create_config.attach_shared_fd = shared_ion_fd[i];
                        buffer_create_config.attach_phy_addr = 0;
                        buffer_create_config.attach_offset = ion_mem_offset[i];
                    }

                    p_mmp_buf = mmp_buffer::create_object(&buffer_create_config);
                    if(p_mmp_buf == NULL) {
                        mmpResult = MMP_FAILURE;
                        break;
                    }
                    else {
                        p_mmp_imageframe->m_p_mmp_buffer[i] = p_mmp_buf;
                        p_mmp_imageframe->m_buf_stride[i] = buffer_stride[i];
                        p_mmp_imageframe->m_buf_height[i] = buffer_height_arr[i];
                    }
                }
            }

        }
        else {
            /* FAIL: new instance */
            mmpResult = MMP_FAILURE;
        }

        if(mmpResult == MMP_SUCCESS) {
        
            m_p_mutex->lock();

            /* register mmp buffer */
            for(i = 0; i < p_mmp_imageframe->m_plane_count; i++) {
                m_list_buffer.Add(p_mmp_imageframe->m_p_mmp_buffer[i]);
            }

            m_p_mutex->unlock();

        }
        else {
            
            if(p_mmp_imageframe != NULL) {
        
                for(i = 0; i < p_mmp_imageframe->m_plane_count; i++) {
                    if(p_mmp_imageframe->m_p_mmp_buffer[i] != NULL) {
                        mmp_buffer::destroy_object(p_mmp_imageframe->m_p_mmp_buffer[i]);
                    }
                }

                delete p_mmp_imageframe;
                p_mmp_imageframe = NULL;
            }
        }

    }
    
    return p_mmp_imageframe;
}

class mmp_buffer_imageframe* mmp_buffer_mgr_ex1::alloc_media_imageframe(MMP_MEDIA_ID producer_id, MMP_S32 pic_width, MMP_S32 pic_height, enum MMP_FOURCC fourcc) {
    
    return this->alloc_media_imageframe(producer_id, pic_width, pic_height, fourcc, mmp_buffer::ION, NULL, NULL);
}

class mmp_buffer_imageframe* mmp_buffer_mgr_ex1::attach_media_imageframe(MMP_MEDIA_ID producer_id, MMP_S32 *shared_ion_fd, MMP_S32 *ion_mem_offset, MMP_S32 pic_width, MMP_S32 pic_height, enum MMP_FOURCC fourcc) {

    return this->alloc_media_imageframe(producer_id, pic_width, pic_height, fourcc, mmp_buffer::ION_ATTACH, shared_ion_fd, ion_mem_offset);
}

class mmp_buffer_audiostream* mmp_buffer_mgr_ex1::alloc_media_audiostream(MMP_MEDIA_ID producer_id, MMP_S32 buf_size, MMP_U32 buf_type, MMP_ADDR stream_buf) {

    MMP_S32 err_cnt = 0;
    class mmp_buffer* p_mmp_buf;
    struct mmp_buffer_create_config buffer_create_config;
    class mmp_buffer_audiostream* p_buf_audiostream = NULL;

    if(buf_type == mmp_buffer::HEAP_ATTACH) {
        if(stream_buf == MMP_ADDR_NULL) {
            err_cnt++;        
        }
    }
        
    if(err_cnt == 0) {
        p_buf_audiostream = new class mmp_buffer_audiostream(producer_id);
        if(p_buf_audiostream != NULL) {

            memset(&buffer_create_config, 0x00, sizeof(buffer_create_config));
                
            buffer_create_config.type = buf_type;
            buffer_create_config.size = buf_size;

            if(buf_type == mmp_buffer::HEAP_ATTACH) {
                buffer_create_config.attach_vir_addr = stream_buf;
                buffer_create_config.attach_offset = 0;
            }

            p_mmp_buf = mmp_buffer::create_object(&buffer_create_config);
            if(p_mmp_buf != NULL) {

                m_p_mutex->lock();
        
                p_buf_audiostream->m_p_mmp_buffer = p_mmp_buf;
                m_list_buffer.Add(p_buf_audiostream->m_p_mmp_buffer);
                
                m_p_mutex->unlock();
            }
        }
    }
    
    return p_buf_audiostream;
}

class mmp_buffer_audiostream* mmp_buffer_mgr_ex1::alloc_media_audiostream(MMP_MEDIA_ID producer_id, MMP_S32 buf_size, MMP_U32 buf_type) {

    return this->mmp_buffer_mgr_ex1::alloc_media_audiostream(producer_id, buf_size, buf_type, MMP_ADDR_NULL);
}

class mmp_buffer_audiostream* mmp_buffer_mgr_ex1::attach_media_audiostream(MMP_MEDIA_ID producer_id, MMP_U8* p_stream_data, MMP_S32 stream_size) {

    return this->mmp_buffer_mgr_ex1::alloc_media_audiostream(producer_id, stream_size, mmp_buffer::HEAP_ATTACH, (MMP_ADDR)p_stream_data);
}

class mmp_buffer_audioframe* mmp_buffer_mgr_ex1::alloc_media_audioframe(MMP_MEDIA_ID producer_id, MMP_S32 buf_size, MMP_U32 buf_type, MMP_ADDR frame_buf) {

    MMP_S32 err_cnt = 0;
    class mmp_buffer* p_mmp_buf;
    struct mmp_buffer_create_config buffer_create_config;
    class mmp_buffer_audioframe* p_buf_audioframe = NULL;

    if(buf_type == mmp_buffer::HEAP_ATTACH) {
        if(frame_buf == MMP_ADDR_NULL) {
            err_cnt++;        
        }
    }
        
    if(err_cnt == 0) {
        p_buf_audioframe = new class mmp_buffer_audioframe(producer_id);
        if(p_buf_audioframe != NULL) {

            memset(&buffer_create_config, 0x00, sizeof(buffer_create_config));
                
            buffer_create_config.type = buf_type;
            buffer_create_config.size = buf_size;

            if(buf_type == mmp_buffer::HEAP_ATTACH) {
                buffer_create_config.attach_vir_addr = frame_buf;
                buffer_create_config.attach_offset = 0;
            }

            p_mmp_buf = mmp_buffer::create_object(&buffer_create_config);
            if(p_mmp_buf != NULL) {

                m_p_mutex->lock();
        
                p_buf_audioframe->m_p_mmp_buffer = p_mmp_buf;
                m_list_buffer.Add(p_buf_audioframe->m_p_mmp_buffer);
                
                m_p_mutex->unlock();
            }
        }
    }
    
    return p_buf_audioframe;
}

class mmp_buffer_audioframe* mmp_buffer_mgr_ex1::alloc_media_audioframe(MMP_MEDIA_ID producer_id, MMP_S32 buf_size, MMP_U32 buf_type) {

    return this->alloc_media_audioframe(producer_id, buf_size, buf_type, MMP_ADDR_NULL);
}

class mmp_buffer_audioframe* mmp_buffer_mgr_ex1::attach_media_audioframe(MMP_MEDIA_ID producer_id, MMP_ADDR buf_addr, MMP_S32 buf_size) {

    return this->alloc_media_audioframe(producer_id, buf_size, mmp_buffer::HEAP_ATTACH, buf_addr);
}


MMP_RESULT mmp_buffer_mgr_ex1::free_media_buffer(class mmp_buffer_media* p_buf_media) {
    
    MMP_S32 i;
    
    if(p_buf_media != NULL) {
    
        switch(p_buf_media->m_type) {

            case mmp_buffer_media::VIDEO_FRAME :
                {
                    class mmp_buffer_videoframe* p_mmp_videoframe = (class mmp_buffer_videoframe*)p_buf_media;

                    for(i = 0; i < p_mmp_videoframe->m_plane_count; i++) {
                        this->free_buffer(p_mmp_videoframe->m_p_mmp_buffer[i]);
                    }
                
                }
                break;

            case mmp_buffer_media::VIDEO_STREAM :
                {
                    class mmp_buffer_videostream* p_mmp_videostream = (class mmp_buffer_videostream*)p_buf_media;

                    this->free_buffer(p_mmp_videostream->m_p_mmp_buffer);
                }
                break;

            case mmp_buffer_media::IMAGE_FRAME :
                {
                    class mmp_buffer_imageframe* p_mmp_imageframe = (class mmp_buffer_imageframe*)p_buf_media;

                    for(i = 0; i < p_mmp_imageframe->m_plane_count; i++) {
                        this->free_buffer(p_mmp_imageframe->m_p_mmp_buffer[i]);
                    }
                
                }
                break;

            case mmp_buffer_media::IMAGE_STREAM :
                {
                    class mmp_buffer_imagestream* p_mmp_imagestream = (class mmp_buffer_imagestream*)p_buf_media;

                    this->free_buffer(p_mmp_imagestream->m_p_mmp_buffer);
                }
                break;

            case mmp_buffer_media::AUDIO_STREAM :
                {
                    class mmp_buffer_audiostream* p_mmp_audiostream = (class mmp_buffer_audiostream*)p_buf_media;

                    this->free_buffer(p_mmp_audiostream->m_p_mmp_buffer);
                }
                break;

            case mmp_buffer_media::AUDIO_FRAME :
                {
                    class mmp_buffer_audioframe* p_mmp_audioframe = (class mmp_buffer_audioframe*)p_buf_media;

                    this->free_buffer(p_mmp_audioframe->m_p_mmp_buffer);
                }
                break;
        }

        delete p_buf_media;
    }

    return MMP_SUCCESS;
}

void mmp_buffer_mgr_ex1::print_info() {

    class mmp_lock autolock(m_p_mutex);
    
    MMP_S32 idx;
    MMP_RESULT mmpResult = MMP_FAILURE;
    bool flag;
    class mmp_buffer* p_mmp_buffer = NULL;
    class mmp_buffer_addr buf_addr;
    MMP_S32 alloc_size[mmp_buffer::TYPE_MAX] = { 0, 0, 0};
    MMP_S32 buf_count[mmp_buffer::TYPE_MAX] = { 0, 0, 0};
    
    
    idx = 0;
    flag = m_list_buffer.GetFirst(p_mmp_buffer);
    while(flag) {
        buf_addr = p_mmp_buffer->get_buf_addr();
        MMPDEBUGMSG(1, (TEXT("\t%d. type=%d vir=0x%08x phy=0x%08x sz=%d fd=(%d 0x%08x)"), 
                      idx, buf_addr.m_type, 
                      buf_addr.m_vir_addr, buf_addr.m_phy_addr, buf_addr.m_size,
                      buf_addr.m_shared_fd, buf_addr.m_shared_fd
                      ));

        buf_count[buf_addr.m_type] ++;
        alloc_size[buf_addr.m_type] +=  buf_addr.m_size;
        idx++;
        flag = m_list_buffer.GetNext(p_mmp_buffer);
    }
    
    MMPDEBUGMSG(1, (TEXT("ION - cnt=%d   sz=%d "), buf_count[mmp_buffer::ION], alloc_size[mmp_buffer::ION] ));
    MMPDEBUGMSG(1, (TEXT("HEAP- cnt=%d   sz=%d "), buf_count[mmp_buffer::HEAP], alloc_size[mmp_buffer::HEAP] ));

}

