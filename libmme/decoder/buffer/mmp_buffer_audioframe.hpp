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

#ifndef MMP_BUFFER_AUDIOFRAME_HPP__
#define MMP_BUFFER_AUDIOFRAME_HPP__

#include "mmp_buffer_media.hpp"

class mmp_buffer_audioframe : public mmp_buffer_media {

friend class CLASS_BUFFER_MGR;

public:
    enum {
        DEFAULT_MAX_FRAME_SIZE = MMP_BYTE_SIZE_256K
    };

private:
    class mmp_buffer* m_p_mmp_buffer;
    MMP_S32 m_data_size;
    
    MMP_S32 m_samplerate;
    MMP_S32 m_channel;
    
private:
    mmp_buffer_audioframe(MMP_MEDIA_ID producer_id);
    virtual ~mmp_buffer_audioframe();
    
public:
    class mmp_buffer_addr get_buf_addr();
    
    MMP_ADDR get_buf_phy_addr();
    MMP_ADDR get_buf_vir_addr();
    MMP_S32 get_buf_shared_fd();
    MMP_S32 get_buf_size();

    inline void set_data_size(MMP_S32 sz) { m_data_size = sz; }
    inline MMP_S32 get_data_size() { return m_data_size; }

    inline void set_samplerate(MMP_S32 sp) { m_samplerate = sp; }
    inline MMP_S32 get_samplerate() { return m_samplerate; }
    
    inline void set_channel(MMP_S32 ch) { m_channel = ch; }
    inline MMP_S32 get_channel() { return m_channel; }
    
};

#endif

