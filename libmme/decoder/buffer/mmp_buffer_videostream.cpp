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

#include "mmp_buffer_videostream.hpp"


/**********************************************************
class member
**********************************************************/

mmp_buffer_videostream::mmp_buffer_videostream(MMP_MEDIA_ID producer_id) : mmp_buffer_media(VIDEO_STREAM, producer_id)

,m_p_mmp_buffer(NULL)
,m_stream_offset(0)
,m_stream_size(0)

,m_ffmpeg_codec_context(NULL)
,m_ffmpeg_codec_context_size(0)

,m_dsi_buffer(NULL)
,m_dsi_size(0)
,m_dsi_alloc_size(0)

,m_header_buffer(NULL)
,m_header_size(0)
,m_header_alloc_size(0)
{

}

mmp_buffer_videostream::~mmp_buffer_videostream() {

    if(m_ffmpeg_codec_context != NULL) {
        free(m_ffmpeg_codec_context);
    }

    if(m_dsi_buffer != NULL) {
        MMP_FREE(m_dsi_buffer);
        m_dsi_buffer = NULL;
        m_dsi_size = 0;
        m_dsi_alloc_size = 0;
    }

    if(m_header_buffer != NULL) {
        MMP_FREE(m_header_buffer);
        m_header_buffer = NULL;
        m_header_size = 0;
        m_header_alloc_size = 0;
    }

}

void mmp_buffer_videostream::set_ffmpeg_codec_context(void* cc, MMP_S32 cc_sz) {

    if(m_ffmpeg_codec_context_size < cc_sz) {
        if(m_ffmpeg_codec_context != NULL) {
            free(m_ffmpeg_codec_context);
        }
        m_ffmpeg_codec_context = malloc(cc_sz);
        m_ffmpeg_codec_context_size = cc_sz;
    }

    memcpy(m_ffmpeg_codec_context, cc, cc_sz);
}

MMP_RESULT mmp_buffer_videostream::alloc_dsi_buffer(MMP_S32 dsi_alloc_size) {
    
    MMP_RESULT mmpResult = MMP_FAILURE;

    if( (dsi_alloc_size <= m_dsi_alloc_size) && (m_dsi_buffer!=NULL) ) {

        m_dsi_size = dsi_alloc_size;
        MMP_MEMSET(m_dsi_buffer, 0x00, dsi_alloc_size);
        mmpResult = MMP_SUCCESS;
    }
    else  {

        if(m_dsi_buffer != NULL) {
            MMP_FREE(m_dsi_buffer);
            m_dsi_buffer = NULL;
            m_dsi_size = 0;
            m_dsi_alloc_size = 0;
        }

        m_dsi_buffer = MMP_MALLOC(dsi_alloc_size);
        if(m_dsi_buffer == NULL) {
            mmpResult = MMP_FAILURE;
            m_dsi_size = 0;
            m_dsi_alloc_size = 0;
        }
        else {
            MMP_MEMSET(m_dsi_buffer, 0x00, dsi_alloc_size);
            m_dsi_size = dsi_alloc_size;
            m_dsi_alloc_size = dsi_alloc_size;
            mmpResult = MMP_SUCCESS;
        }
    }
    
    return mmpResult;
}

MMP_RESULT mmp_buffer_videostream::alloc_header_buffer(MMP_S32 header_alloc_size) {
    
    MMP_RESULT mmpResult = MMP_FAILURE;

    if( (header_alloc_size <= m_header_alloc_size) && (m_header_buffer!=NULL) ) {

        m_header_size = header_alloc_size;
        MMP_MEMSET(m_header_buffer, 0x00, header_alloc_size);
        mmpResult = MMP_SUCCESS;
    }
    else  {

        if(m_header_buffer != NULL) {
            MMP_FREE(m_header_buffer);
            m_header_buffer = NULL;
            m_header_size = 0;
            m_header_alloc_size = 0;
        }

        m_header_buffer = MMP_MALLOC(header_alloc_size);
        if(m_header_buffer == NULL) {
            mmpResult = MMP_FAILURE;
            m_header_size = 0;
            m_header_alloc_size = 0;
        }
        else {
            MMP_MEMSET(m_header_buffer, 0x00, header_alloc_size);
            m_header_size = header_alloc_size;
            m_header_alloc_size = header_alloc_size;
            mmpResult = MMP_SUCCESS;
        }
    }
    
    return mmpResult;
}

