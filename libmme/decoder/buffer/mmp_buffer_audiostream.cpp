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

#include "mmp_buffer_audiostream.hpp"


/**********************************************************
class member
**********************************************************/

mmp_buffer_audiostream::mmp_buffer_audiostream(MMP_MEDIA_ID producer_id) : mmp_buffer_media(AUDIO_STREAM, producer_id)

,m_p_mmp_buffer(NULL)
,m_stream_offset(0)
,m_stream_size(0)

,m_ffmpeg_codec_context(NULL)
,m_ffmpeg_codec_context_size(0)
{

}

mmp_buffer_audiostream::~mmp_buffer_audiostream() {

    if(m_ffmpeg_codec_context != NULL) {
        MMP_FREE(m_ffmpeg_codec_context);
        m_ffmpeg_codec_context = NULL;
    }

}

void mmp_buffer_audiostream::set_ffmpeg_codec_context(void* cc, MMP_S32 cc_sz) {

    if(m_ffmpeg_codec_context_size < cc_sz) {
        if(m_ffmpeg_codec_context != NULL) {
            free(m_ffmpeg_codec_context);
        }
        m_ffmpeg_codec_context = MMP_MALLOC(cc_sz);
        m_ffmpeg_codec_context_size = cc_sz;
    }

    memcpy(m_ffmpeg_codec_context, cc, cc_sz);
}
