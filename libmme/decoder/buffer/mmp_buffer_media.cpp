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

#include "mmp_buffer_media.hpp"

mmp_buffer_media::mmp_buffer_media(MMP_S32 type, MMP_MEDIA_ID producer_id) : 

m_type(type)

,m_producer_id(producer_id)
,m_own_id(producer_id)

,m_flag(mmp_buffer_media::FLAG_NULL)
,m_pts(0)
,m_coding_duration(0)
,m_coding_result(FAIL)
,m_priv_data(MMP_ADDR_NULL)
{

}

mmp_buffer_media::~mmp_buffer_media() {

}

MMP_BOOL mmp_buffer_media::is_audio() {
    
    MMP_BOOL bflag = MMP_FALSE;
    
    if( (m_type == AUDIO_STREAM)
        || (m_type == AUDIO_FRAME) ) {
        bflag = MMP_TRUE;
    }
    else {
        bflag = MMP_FALSE;
    }

    return bflag;
}
 
MMP_BOOL mmp_buffer_media::is_video() {
    
    MMP_BOOL bflag = MMP_FALSE;
    
    if( (m_type == VIDEO_STREAM)
        || (m_type == VIDEO_FRAME) ) {
        bflag = MMP_TRUE;
    }
    else {
        bflag = MMP_FALSE;
    }

    return bflag;
}

MMP_BOOL mmp_buffer_media::is_image() {
    
    MMP_BOOL bflag = MMP_FALSE;
    
    if( (m_type == IMAGE_STREAM)
        || (m_type == IMAGE_FRAME) ) {
        bflag = MMP_TRUE;
    }
    else {
        bflag = MMP_FALSE;
    }

    return bflag;
}

enum MMP_MEDIATYPE mmp_buffer_media::get_media_type() {

    enum MMP_MEDIATYPE mt;

    switch(m_type) {
        case VIDEO_STREAM:
        case VIDEO_FRAME:
            mt = MMP_MEDIATYPE_VIDEO;
            break;

        case AUDIO_STREAM:
        case AUDIO_FRAME:
            mt = MMP_MEDIATYPE_AUDIO;
            break;
        
        case IMAGE_STREAM:
        case IMAGE_FRAME:
            mt = MMP_MEDIATYPE_IMAGE;
            break;

        default:
            mt = MMP_MEDIATYPE_UNKNOWN;
            break;
    }

    return mt;
}

