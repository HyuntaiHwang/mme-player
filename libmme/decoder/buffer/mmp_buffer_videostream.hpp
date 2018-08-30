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

#ifndef MMP_BUFFER_VIDEOSTREAM_HPP__
#define MMP_BUFFER_VIDEOSTREAM_HPP__

#include "mmp_buffer_media.hpp"

class mmp_buffer_videostream : public mmp_buffer_media {

friend class CLASS_BUFFER_MGR;

public:
    enum {
        DEFAULT_MAX_STREAM_SIZE = MMP_BYTE_SIZE_1M
    };
        
private:
    class mmp_buffer* m_p_mmp_buffer;
    MMP_S32 m_stream_offset; /* ref. VPU RV */
    MMP_S32 m_stream_size;
    
    MMP_S32 m_player_framerate;
    MMP_S32 m_player_bitrate;
    MMP_S32 m_pic_width;
    MMP_S32 m_pic_height;
    
    void* m_ffmpeg_codec_context;
    MMP_S32 m_ffmpeg_codec_context_size;

    void* m_dsi_buffer;
    MMP_S32 m_dsi_size;
    MMP_S32 m_dsi_alloc_size;

    void* m_header_buffer;
    MMP_S32 m_header_size;
    MMP_S32 m_header_alloc_size;

private:
    mmp_buffer_videostream(MMP_MEDIA_ID producer_id);
    virtual ~mmp_buffer_videostream();
    
public:
    inline class mmp_buffer_addr get_buf_addr() { return m_p_mmp_buffer->get_buf_addr(); }
    inline MMP_U32 get_buf_vir_addr() { return m_p_mmp_buffer->get_buf_addr().m_vir_addr; }
    inline MMP_S32 get_buf_size() { return m_p_mmp_buffer->get_buf_addr().m_size; }
    inline MMP_S32 get_buf_shared_fd() { return m_p_mmp_buffer->get_buf_shared_fd(); }
    
    inline void set_stream_offset(MMP_S32 offset) { m_stream_offset = offset; }
    inline void inc_stream_offset(MMP_S32 offset) { m_stream_offset += offset; if(m_stream_offset>m_stream_size) m_stream_offset=m_stream_size; }
    inline MMP_S32 get_stream_offset() { return m_stream_offset; }
    inline void set_stream_size(MMP_S32 sz) { m_stream_size = sz; }
    inline MMP_S32 get_stream_size() { return m_stream_size; }
    inline MMP_U8* get_stream_real_ptr() { return (MMP_U8*)(m_p_mmp_buffer->get_buf_addr().m_vir_addr + (MMP_U32)m_stream_offset); }
    inline MMP_S32 get_stream_real_size() { return (m_stream_size-m_stream_offset); }
        
    inline void set_player_framerate(MMP_S32 fr) { m_player_framerate = fr; }
    inline MMP_S32 get_player_framerate() { return m_player_framerate; }

    inline void set_player_bitrate(MMP_S32 bitrate) { m_player_bitrate = bitrate; }
    inline MMP_S32 get_player_bitrate() { return m_player_bitrate; }

    inline void set_pic_width(MMP_S32 w) { m_pic_width = w; }
    inline MMP_S32 get_pic_width() { return m_pic_width; }

    inline void set_pic_height(MMP_S32 h) { m_pic_height = h; }
    inline MMP_S32 get_pic_height() { return m_pic_height; }

    void set_ffmpeg_codec_context(void* cc, MMP_S32 cc_sz);
    inline void* get_ffmpeg_codec_context() { return m_ffmpeg_codec_context; }

    MMP_RESULT alloc_dsi_buffer(MMP_S32 dsi_alloc_size);
    inline void* get_dsi_buffer() { return m_dsi_buffer; }
    inline MMP_S32 get_dsi_size() { return m_dsi_size; }
    inline void set_dsi_size(MMP_S32 dsi_size) { m_dsi_size = dsi_size; }

    MMP_RESULT alloc_header_buffer(MMP_S32 header_alloc_size);
    inline void* get_header_buffer() { return m_header_buffer; }
    inline MMP_S32 get_header_size() { return m_header_size; }
    inline void set_header_size(MMP_S32 hdr_size) { m_header_size = hdr_size; }
};

#endif

