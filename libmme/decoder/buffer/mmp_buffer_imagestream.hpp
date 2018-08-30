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

#ifndef MMP_BUFFER_IMAGESTREAM_HPP__
#define MMP_BUFFER_IMAGESTREAM_HPP__

#include "mmp_buffer_media.hpp"

class mmp_buffer_imagestream : public mmp_buffer_media {

friend class CLASS_BUFFER_MGR;
private:
    class mmp_buffer* m_p_mmp_buffer;
    MMP_S32 m_stream_offset; /* ref. VPU RV */
    MMP_S32 m_stream_size;
    MMP_S32 m_used_byte;
    
    MMP_S32 m_pic_width;
    MMP_S32 m_pic_height;
    
    FILE* m_fp_imagefile;

private:
    mmp_buffer_imagestream(MMP_MEDIA_ID producer_id);
    virtual ~mmp_buffer_imagestream();
    
public:
    inline class mmp_buffer_addr get_buf_addr() { return m_p_mmp_buffer->get_buf_addr(); }
    inline MMP_U32 get_buf_vir_addr() { return m_p_mmp_buffer->get_buf_addr().m_vir_addr; }
    inline MMP_U32 get_buf_phy_addr() { return m_p_mmp_buffer->get_buf_addr().m_phy_addr; }
    inline MMP_S32 get_buf_size() { return m_p_mmp_buffer->get_buf_addr().m_size; }
    inline MMP_S32 get_buf_shared_fd() { return m_p_mmp_buffer->get_buf_addr().m_shared_fd; }
    
    inline void set_stream_offset(MMP_S32 offset) { m_stream_offset = offset; }
    inline MMP_S32 get_stream_offset() { return m_stream_offset; }
    inline void set_stream_size(MMP_S32 sz) { m_stream_size = sz; }
    inline MMP_S32 get_stream_size() { return m_stream_size; }
    inline MMP_U8* get_stream_real_ptr() { return (MMP_U8*)(m_p_mmp_buffer->get_buf_addr().m_vir_addr + (MMP_U32)m_stream_offset); }
    inline MMP_S32 get_stream_real_size() { return (m_stream_size-m_stream_offset); }

    inline void set_used_byte(MMP_S32 used_byte) { m_used_byte = used_byte; }
    inline MMP_S32 get_used_byte() { return m_used_byte; }


    inline void set_pic_width(MMP_S32 w) { m_pic_width = w; }
    inline MMP_S32 get_pic_width() { return m_pic_width; }

    inline void set_pic_height(MMP_S32 h) { m_pic_height = h; }
    inline MMP_S32 get_pic_height() { return m_pic_height; }

    inline FILE* get_fp_imagefile() { return m_fp_imagefile; }
    inline MMP_RESULT sync_buf() { return m_p_mmp_buffer->sync_buf(); }
};

#endif

