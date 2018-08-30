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

#ifndef MMP_BUFFER_MEDIA_HPP__
#define MMP_BUFFER_MEDIA_HPP__

#include "mmp_buffer.hpp"
#include "MmpPlayerDef.h"

class mmp_buffer_media {

friend class CLASS_BUFFER_MGR;

public:
    enum {
        SUCCESS = 0,
        RECONFIG,
        FAIL,
        FAIL_NOT_SUPPORT_IMAGE_FORMAT,
        NEED_MORE_DATA,
    };

    enum FLAG {
        FLAG_NULL     = 0,
        FLAG_CONFIGDATA     = (1<<0),
        FLAG_VIDEO_KEYFRAME = (1<<1),
        FLAG_VIDEO_CHUNK_REUSE = (1<<2),
        FLAG_RECONFIGDATA     = (1<<3),
    };

    
protected:

    enum {
        VIDEO_FRAME = 0,
        VIDEO_STREAM,
        AUDIO_FRAME,
        AUDIO_STREAM,
        IMAGE_FRAME,
        IMAGE_STREAM
    };
        
    MMP_MEDIA_ID m_producer_id;
    MMP_MEDIA_ID m_own_id;
    
    MMP_S32 m_type;
    enum mmp_buffer_media::FLAG m_flag;
    MMP_TICKS m_pts;
    MMP_U32 m_coding_duration;
    MMP_U32 m_preproc_duration;
    MMP_U32 m_postproc_duration;
    MMP_U32 m_coding_result;
    MMP_ADDR m_priv_data;
    
protected:
    mmp_buffer_media(MMP_S32 type, MMP_MEDIA_ID producer_id);
    virtual ~mmp_buffer_media();

public:
    inline void clear_producer() { m_producer_id = MEDIA_ID_UNKNOWN; }
    inline MMP_MEDIA_ID get_producer() { return m_producer_id; }

    inline void set_own(MMP_MEDIA_ID own_id) { m_own_id = own_id; }
    inline MMP_MEDIA_ID get_own() { return m_own_id; }
    inline void clear_own() { m_own_id = MEDIA_ID_UNKNOWN; }
    inline MMP_BOOL is_own(MMP_MEDIA_ID id) { return (m_own_id==id)?MMP_TRUE:MMP_FALSE; }
    
    inline void set_pts(MMP_TICKS pts) { m_pts = pts; }
    inline MMP_TICKS get_pts() { return m_pts; }
    
    inline void set_flag(enum FLAG flag) { m_flag = flag; }
    inline void or_flag(enum FLAG flag) { MMP_U32 f = (MMP_U32)m_flag;  f|= (MMP_U32)flag;  m_flag=(enum FLAG)f;}
    inline enum FLAG get_flag() { return m_flag; }
    inline MMP_BOOL is_flag_keyframe() { MMP_U32 f = (MMP_U32)m_flag;  return ( (f&=(MMP_U32)FLAG_VIDEO_KEYFRAME)==0 ) ? MMP_FALSE : MMP_TRUE; }
    
    inline void set_coding_dur(MMP_U32 dur) { m_coding_duration = dur; } 
    inline MMP_U32 get_coding_dur() { return m_coding_duration; }

    inline void set_preproc_dur(MMP_U32 dur) { m_preproc_duration = dur; } 
    inline MMP_U32 get_preproc_dur() { return m_preproc_duration; }
    
    inline void set_postproc_dur(MMP_U32 dur) { m_postproc_duration = dur; } 
    inline MMP_U32 get_postproc_dur() { return m_postproc_duration; }

    inline void set_coding_result(MMP_U32 res) { m_coding_result = res; }
    inline MMP_U32 get_coding_result(void) { return m_coding_result; }
    
    inline void set_priv_data(MMP_ADDR d) { m_priv_data = d; }
    inline MMP_ADDR get_priv_data(void) { return m_priv_data; }
        
    MMP_BOOL is_audio();
    MMP_BOOL is_video();
    MMP_BOOL is_image();
    enum MMP_MEDIATYPE get_media_type();
};

#endif

