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

#ifndef MMP_BUFFER_AUDIOSTREAM_HPP__
#define MMP_BUFFER_AUDIOSTREAM_HPP__

#include "mmp_buffer_media.hpp"

class mmp_buffer_audiostream : public mmp_buffer_media {

friend class CLASS_BUFFER_MGR;

public:
    enum {
        DEFAULT_MAX_STREAM_SIZE = MMP_BYTE_SIZE_256K
    };

private:
    class mmp_buffer* m_p_mmp_buffer;
    MMP_S32 m_stream_offset; /* ref. VPU RV */
    MMP_S32 m_stream_size;
        
    void* m_ffmpeg_codec_context;
    MMP_S32 m_ffmpeg_codec_context_size;

private:
    mmp_buffer_audiostream(MMP_MEDIA_ID producer_id);
    virtual ~mmp_buffer_audiostream();
    
public:
    inline class mmp_buffer_addr get_buf_addr() { return m_p_mmp_buffer->get_buf_addr(); }
    inline MMP_ADDR get_buf_vir_addr() { return m_p_mmp_buffer->get_buf_addr().m_vir_addr; }
    inline MMP_S32 get_buf_size() { return m_p_mmp_buffer->get_buf_addr().m_size; }
    
    inline void set_stream_offset(MMP_S32 offset) { m_stream_offset = offset; }
    inline void inc_stream_offset(MMP_S32 offset) { m_stream_offset += offset; if(m_stream_offset>m_stream_size) m_stream_offset=m_stream_size; }
    inline MMP_S32 get_stream_offset() { return m_stream_offset; }
    
    inline void set_stream_size(MMP_S32 sz) { m_stream_size = sz; }
    inline MMP_S32 get_stream_size() { return m_stream_size; }
    inline MMP_ADDR get_stream_real_addr() { return (MMP_ADDR)(m_p_mmp_buffer->get_buf_addr().m_vir_addr + (MMP_U32)m_stream_offset); }
    inline MMP_S32 get_stream_real_size() { return (m_stream_size-m_stream_offset); }
        
    void set_ffmpeg_codec_context(void* cc, MMP_S32 cc_sz);
    inline void* get_ffmpeg_codec_context() { return m_ffmpeg_codec_context; }

};

#endif

