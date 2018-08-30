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

#ifndef MMP_BUFFER_VIDEOFRAME_HPP__
#define MMP_BUFFER_VIDEOFRAME_HPP__

#include "mmp_buffer_media.hpp"

class mmp_buffer_videoframe : public mmp_buffer_media {

friend class CLASS_BUFFER_MGR;

public:
    struct ion_attach_config {
        MMP_MEDIA_ID producer_id; 
        enum MMP_FOURCC fourcc;
        MMP_S32 pic_width;
        MMP_S32 pic_height;
        MMP_S32 plane_count;
        MMP_S32 shared_fd[MMP_IMAGE_MAX_PLANE_COUNT]; 
        MMP_S32 ion_mem_offset[MMP_IMAGE_MAX_PLANE_COUNT]; 
        MMP_S32 stride[MMP_IMAGE_MAX_PLANE_COUNT];
        MMP_S32 alignheight[MMP_IMAGE_MAX_PLANE_COUNT];
        MMP_S32 plane_size[MMP_IMAGE_MAX_PLANE_COUNT];
        enum mmp_buffer::ION_FLAG ion_flag; /* Cache, Non-Cache */
        enum mmp_buffer::ION_HEAP ion_heap; /* Cache, Non-Cache */
    };

private:
    class mmp_buffer* m_p_mmp_buffer[MMP_IMAGE_MAX_PLANE_COUNT];

    MMP_S32 m_pic_width;
    MMP_S32 m_pic_height;
    MMP_S32 m_plane_count;
    enum MMP_FOURCC m_fourcc;
    enum MMP_BUFTYPE m_buftype;

    MMP_S32 m_buf_stride[MMP_IMAGE_MAX_PLANE_COUNT];
    MMP_S32 m_buf_height[MMP_IMAGE_MAX_PLANE_COUNT];
        
    MMP_U8* m_android_metadata_ptr;
    MMP_S32 m_android_metadata_size;

private:
    mmp_buffer_videoframe(MMP_MEDIA_ID producer_id, enum MMP_BUFTYPE buftype);
    virtual ~mmp_buffer_videoframe();
    
    MMP_ADDR get_buf_cal_addr_y(MMP_ADDR base_addr);
    MMP_ADDR get_buf_cal_addr_u(MMP_ADDR base_addr);
    MMP_ADDR get_buf_cal_addr_v(MMP_ADDR base_addr);
    
public:
    
    class mmp_buffer_addr get_buf_addr(MMP_S32 frame_id);
    
    void sync_buf(MMP_S32 frame_id);
    void sync_buf();

    MMP_ADDR get_buf_vir_addr(MMP_S32 frame_id=0);
    MMP_ADDR get_buf_vir_addr_y();
    MMP_ADDR get_buf_vir_addr_u();
    MMP_ADDR get_buf_vir_addr_v();
    MMP_ADDR get_buf_phy_addr(MMP_S32 frame_id=0);
    MMP_ADDR get_buf_phy_addr_y();
    MMP_ADDR get_buf_phy_addr_u();
    MMP_ADDR get_buf_phy_addr_v();
        
    MMP_S32 get_buf_shared_fd(MMP_S32 frame_id=0);
    MMP_S32 get_buf_total_size();
    MMP_S32 get_buf_size(MMP_S32 frame_id=0);
    MMP_S32 get_buf_size_luma();
    MMP_S32 get_buf_size_chroma();
        
    inline MMP_S32 get_stride_luma() { return m_buf_stride[MMP_YUV420_PLAINE_INDEX_Y]; }
    inline MMP_S32 get_stride_chroma() { return m_buf_stride[MMP_YUV420_PLAINE_INDEX_U]; }
    inline MMP_S32 get_buf_stride(MMP_S32 frame_id=0) { return m_buf_stride[frame_id]; }
    inline MMP_S32 get_buf_height(MMP_S32 frame_id=0) { return m_buf_height[frame_id]; }

    inline void set_buf_stride(MMP_S32 stride, MMP_S32 frame_id=0) { m_buf_stride[frame_id] = stride; }
    inline void set_buf_height(MMP_S32 h, MMP_S32 frame_id=0) { m_buf_height[frame_id] = h; }
    inline void set_buf_stride_luma(MMP_S32 stride) { m_buf_stride[MMP_YUV420_PLAINE_INDEX_Y] = stride; }
    inline void set_buf_stride_chroma(MMP_S32 stride) { m_buf_stride[MMP_YUV420_PLAINE_INDEX_U] = stride, m_buf_stride[MMP_YUV420_PLAINE_INDEX_V] = stride; }
    
    inline MMP_S32 get_pic_width() { return m_pic_width; }
    inline MMP_S32 get_pic_height() { return m_pic_height; }
    
    inline enum MMP_FOURCC get_fourcc() { return m_fourcc; }
    inline MMP_S32 get_plane_count() { return m_plane_count; }
    inline enum MMP_BUFTYPE get_buf_type() { return m_buftype; }

#if (MMP_PLATFORM == MMP_PLATFORM_WIN32)
    MMP_RESULT make_android_metadat_gralloc_source();
    inline MMP_U8* get_android_metadata_ptr() { return m_android_metadata_ptr; }
    inline MMP_S32 get_android_metadata_size() { return m_android_metadata_size; }
#endif

    MMP_RESULT save_bmp(const MMP_CHAR* filename);

public:

    static MMP_S32 s_get_stride(enum MMP_FOURCC fourcc, MMP_S32 pic_width, enum MMP_BUFTYPE buftype);
    static MMP_S32 s_get_height(enum MMP_FOURCC fourcc, MMP_S32 pic_height, enum MMP_BUFTYPE buftype);
    static MMP_S32 s_get_size(enum MMP_FOURCC fourcc, MMP_S32 pic_width, MMP_S32 pic_height, enum MMP_BUFTYPE buftype);

    static MMP_S32 s_get_y_stride(enum MMP_FOURCC fourcc, MMP_S32 pic_width, enum MMP_BUFTYPE buftype) { return mmp_buffer_videoframe::s_get_stride(fourcc, pic_width, buftype); }
    static MMP_S32 s_get_u_stride(enum MMP_FOURCC fourcc, MMP_S32 pic_width, enum MMP_BUFTYPE buftype);
    static MMP_S32 s_get_v_stride(enum MMP_FOURCC fourcc, MMP_S32 pic_width, enum MMP_BUFTYPE buftype) { return mmp_buffer_videoframe::s_get_u_stride(fourcc, pic_width, buftype); }

    static MMP_S32 s_get_y_height(enum MMP_FOURCC fourcc, MMP_S32 pic_height, enum MMP_BUFTYPE buftype) { return mmp_buffer_videoframe::s_get_height(fourcc, pic_height, buftype); }
    static MMP_S32 s_get_u_height(enum MMP_FOURCC fourcc, MMP_S32 pic_height, enum MMP_BUFTYPE buftype);
    static MMP_S32 s_get_v_height(enum MMP_FOURCC fourcc, MMP_S32 pic_height, enum MMP_BUFTYPE buftype){ return mmp_buffer_videoframe::s_get_u_height(fourcc, pic_height, buftype); }

    static MMP_S32 s_get_y_offset(enum MMP_FOURCC fourcc, MMP_S32 pic_width, MMP_S32 pic_height, enum MMP_BUFTYPE buftype);
    static MMP_S32 s_get_u_offset(enum MMP_FOURCC fourcc, MMP_S32 pic_width, MMP_S32 pic_height, enum MMP_BUFTYPE buftype);
    static MMP_S32 s_get_v_offset(enum MMP_FOURCC fourcc, MMP_S32 pic_width, MMP_S32 pic_height, enum MMP_BUFTYPE buftype);

    static MMP_RESULT csc_YUV420toYUV420(class mmp_buffer_videoframe *p_buf_vf_src, class mmp_buffer_videoframe *p_buf_vf_dst);
};

#endif

