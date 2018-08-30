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

#include "mmp_buffer_audioframe.hpp"


/**********************************************************
class member
**********************************************************/

mmp_buffer_audioframe::mmp_buffer_audioframe(MMP_MEDIA_ID producer_id) : mmp_buffer_media(AUDIO_FRAME, producer_id)
,m_p_mmp_buffer(NULL)
,m_samplerate(0)
,m_channel(0)
{
 
}

mmp_buffer_audioframe::~mmp_buffer_audioframe() {

}

class mmp_buffer_addr mmp_buffer_audioframe::get_buf_addr() {

    class mmp_buffer_addr buf_addr;
buf_addr = this->m_p_mmp_buffer->get_buf_addr();
    return buf_addr;
}

MMP_ADDR mmp_buffer_audioframe::get_buf_vir_addr() {

    MMP_ADDR vir_addr = MMP_ADDR_NULL;

    vir_addr = (MMP_ADDR)this->m_p_mmp_buffer->get_vir_addr();
    
    return vir_addr;
}

MMP_ADDR mmp_buffer_audioframe::get_buf_phy_addr() {

    MMP_ADDR addr = MMP_ADDR_NULL;

    addr = this->m_p_mmp_buffer->get_phy_addr();
    
    return addr;
}

MMP_S32 mmp_buffer_audioframe::get_buf_shared_fd() {

    MMP_S32 shared_fd = -1;

    shared_fd = this->m_p_mmp_buffer->get_buf_shared_fd();
    
    return shared_fd;
}

    
MMP_S32 mmp_buffer_audioframe::get_buf_size() {

    MMP_S32 buf_sz = 0;

    buf_sz = this->m_p_mmp_buffer->get_buf_size();
    
    return buf_sz;
}
