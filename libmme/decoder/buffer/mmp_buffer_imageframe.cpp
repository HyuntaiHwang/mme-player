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

#include "mmp_buffer_imageframe.hpp"


/**********************************************************
class member
**********************************************************/

mmp_buffer_imageframe::mmp_buffer_imageframe(MMP_MEDIA_ID producer_id) : mmp_buffer_media(IMAGE_FRAME, producer_id)

,m_pic_width(0)
,m_pic_height(0)
,m_plane_count(0)
,m_fourcc(MMP_FOURCC_VIDEO_UNKNOWN)
{
    MMP_S32 i;

    for(i = 0; i < MMP_IMAGE_MAX_PLANE_COUNT; i++) {
        m_p_mmp_buffer[i] = NULL;
    }

}

mmp_buffer_imageframe::~mmp_buffer_imageframe() {

}

class mmp_buffer_addr mmp_buffer_imageframe::get_buf_addr(MMP_S32 frame_id) {

    class mmp_buffer_addr buf_addr;

    if( (frame_id >= 0) && (frame_id < m_plane_count) ) {
        buf_addr = this->m_p_mmp_buffer[frame_id]->get_buf_addr();
    }
    
    return buf_addr;
}

MMP_ADDR mmp_buffer_imageframe::get_buf_vir_addr(MMP_S32 frame_id) {

    MMP_ADDR vir_addr = (MMP_ADDR)0;

    if( (frame_id >= 0) && (frame_id < m_plane_count) ) {
        vir_addr = this->m_p_mmp_buffer[frame_id]->get_vir_addr();
    }
    
    return vir_addr;
}

MMP_U32 mmp_buffer_imageframe::get_buf_phy_addr(MMP_S32 frame_id) {

    MMP_U32 addr = 0;

    if( (frame_id >= 0) && (frame_id < m_plane_count) ) {
        addr = this->m_p_mmp_buffer[frame_id]->get_phy_addr();
    }
    
    return addr;
}

MMP_S32 mmp_buffer_imageframe::get_buf_shared_fd(MMP_S32 frame_id) {

    MMP_S32 shared_fd = -1;

    if( (frame_id >= 0) && (frame_id < m_plane_count) ) {
        shared_fd = this->m_p_mmp_buffer[frame_id]->get_buf_shared_fd();
    }
    
    return shared_fd;
}

MMP_S32 mmp_buffer_imageframe::get_buf_size(MMP_S32 frame_id) {

    MMP_S32 buf_size = 0;

    if( (frame_id >= 0) && (frame_id < m_plane_count) ) {
        buf_size = this->m_p_mmp_buffer[frame_id]->get_buf_size();
    }
    
    return buf_size;
}
