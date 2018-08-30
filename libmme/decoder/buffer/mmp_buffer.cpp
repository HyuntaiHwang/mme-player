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

#include "mmp_buffer.hpp"
#include "mmp_buffer_ion.hpp"
#include "mmp_buffer_ion_attach.hpp"
#include "mmp_buffer_heap.hpp"
#include "mmp_buffer_heap_attach.hpp"
#include "mmp_buffer_anw43_attach.hpp"
#include "mmp_buffer_meta.hpp"
#include "mmp_vpu_dev.hpp"

/**********************************************************
create/destroy object
**********************************************************/

class mmp_buffer* mmp_buffer::create_object(struct mmp_buffer_create_config *p_create_config) {

	class mmp_buffer* p_obj = NULL;
    
    switch(p_create_config->type) {
    
        case mmp_buffer::ION:
            p_obj = new class mmp_buffer_ion(p_create_config);
            break;
    
        case mmp_buffer::ION_ATTACH:
            p_obj = new class mmp_buffer_ion_attach(p_create_config);
            break;
    
        case mmp_buffer::HEAP:
            p_obj = new class mmp_buffer_heap(p_create_config);
            break;

        case mmp_buffer::HEAP_ATTACH:
            p_obj = new class mmp_buffer_heap_attach(p_create_config);
            break;

        case mmp_buffer::ANDROID_NATIVE_WINDOW_ATTACH:
            p_obj = new class mmp_buffer_anw43_attach(p_create_config);
            break;

        case mmp_buffer::META:
            p_obj = new class mmp_buffer_meta(p_create_config);
            break;

    }
    
	if(p_obj!=NULL) {
        
		if(p_obj->open() != MMP_SUCCESS) {
			p_obj->close();
			delete p_obj;
			p_obj = NULL;
		}
	}

    return p_obj;
}

MMP_RESULT mmp_buffer::destroy_object(class mmp_buffer* p_obj) {

	if(p_obj != NULL) {
        p_obj->close();
        delete p_obj;
    }

    return MMP_SUCCESS;
}


class mmp_buffer_addr operator+(class mmp_buffer_addr &A, MMP_U32 value) {

    class mmp_buffer_addr B;

    B.m_vir_addr = A.m_vir_addr + value;
    B.m_phy_addr = A.m_phy_addr + value;

    B.m_size = A.m_size - value;
    B.m_shared_fd = A.m_shared_fd;

    return B;
}

/**********************************************************
class member
**********************************************************/

MMP_U32 mmp_buffer::s_instance_index = 0;  

mmp_buffer::mmp_buffer(struct mmp_buffer_create_config *p_create_config) :
m_create_config(*p_create_config) 
,m_buf_addr(0,0,p_create_config->size,-1)
{
    mmp_buffer::s_instance_index++;

    m_buf_addr.m_type = p_create_config->type;
}

mmp_buffer::~mmp_buffer() {

}


MMP_ADDR mmp_buffer::get_phy_addr_from_shared_fd(MMP_S32 shared_fd) {

    return mmp_vpu_dev::get_instance()->get_phy_addr_from_shared_fd(shared_fd);
}

MMP_ADDR mmp_buffer::get_vir_addr_from_phy_addr(MMP_U32 phy_addr, MMP_S32 size) {

    return mmp_vpu_dev::get_instance()->get_vir_addr_from_phy_addr(phy_addr, size);
}