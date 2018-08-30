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

#include "mmp_buffer_heap_attach.hpp"
#include "MmpUtil.hpp"


mmp_buffer_heap_attach::mmp_buffer_heap_attach(struct mmp_buffer_create_config *p_create_config) : mmp_buffer(p_create_config)

{
    
}

mmp_buffer_heap_attach::~mmp_buffer_heap_attach() {

}

MMP_RESULT mmp_buffer_heap_attach::open() {

    MMP_RESULT mmpResult = MMP_FAILURE;

    this->m_buf_addr.m_phy_addr = 0;
    this->m_buf_addr.m_shared_fd = -1;
    this->m_buf_addr.m_vir_addr = this->m_create_config.attach_vir_addr;
    
    if(this->m_buf_addr.m_vir_addr != 0) {
        
        this->m_buf_addr.m_shared_fd = MMP_BUFFER_HEAP_FD_BASE + mmp_buffer::s_instance_index;
        this->m_buf_addr.m_phy_addr = 0;

        mmpResult = MMP_SUCCESS;
    }

    return mmpResult;
}

MMP_RESULT mmp_buffer_heap_attach::close() {

    this->m_buf_addr.m_shared_fd = -1;
    this->m_buf_addr.m_vir_addr = 0;

    return MMP_SUCCESS;
}

