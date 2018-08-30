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

#include "mmp_buffer_mgr.h"
#include "mmp_buffer_mgr.hpp"
#include "mmp_buffer_mgr_ex1.hpp"

#if (MMP_HW_CODEC_VIDEO_VPU_ENABLE == 1)
#include "vdi.h"
#endif

/**********************************************************
create/destroy instance
**********************************************************/

class mmp_buffer_mgr* mmp_buffer_mgr::s_p_instance = NULL;

MMP_RESULT mmp_buffer_mgr::create_instance(void) {

    MMP_RESULT mmpResult = MMP_FAILURE;
	class mmp_buffer_mgr* p_obj = NULL;

    if(mmp_buffer_mgr::s_p_instance == NULL) {

	    p_obj = new class CLASS_BUFFER_MGR();
	    if(p_obj!=NULL) {
		    if(p_obj->open( ) != MMP_ErrorNone)    
		    {
			    p_obj->close();
			    delete p_obj;
			    p_obj = NULL;
		    }
	    }

        if(p_obj != NULL) {
            mmp_buffer_mgr::s_p_instance = p_obj;
            mmpResult = MMP_SUCCESS;
        }

    }

    return mmpResult;
}

MMP_RESULT mmp_buffer_mgr::destroy_instance() {

    class mmp_buffer_mgr* p_obj = mmp_buffer_mgr::s_p_instance;

	if(p_obj != NULL) {
        p_obj->close();
        delete p_obj;
    }

    return MMP_SUCCESS;
}

class mmp_buffer_mgr* mmp_buffer_mgr::get_instance() {

    if(mmp_buffer_mgr::s_p_instance == NULL) {
        mmp_buffer_mgr::create_instance();
    }

    return mmp_buffer_mgr::s_p_instance;
}

/**********************************************************
 extern C  API 
**********************************************************/

#if (MMP_HW_CODEC_VIDEO_VPU_ENABLE == 1)
int mmp_buffer_mgr_alloc_vpu_buffer(MMP_S32 buf_size, void* vpu_buffer_ptr) {
    
    class mmp_buffer* p_mmp_buf;
    class mmp_buffer_addr buf_addr;
    int iret = -1;
    vpu_buffer_t* vb = (vpu_buffer_t*)vpu_buffer_ptr;

    p_mmp_buf = mmp_buffer_mgr::get_instance()->alloc_dma_buffer(buf_size);
    if(p_mmp_buf != NULL) {
        
        buf_addr = p_mmp_buf->get_buf_addr();

        vb->base = buf_addr.m_vir_addr;
        vb->phys_addr = buf_addr.m_phy_addr;
        vb->size = buf_addr.m_size;
        vb->virt_addr = buf_addr.m_vir_addr;

        iret = 0;
    }

    return iret;
}

int mmp_buffer_mgr_free_vpu_buffer(void* vpu_buffer_ptr) {

    class mmp_buffer_addr buf_addr;
    int iret;
    MMP_RESULT mmpResult;
    vpu_buffer_t* vb = (vpu_buffer_t*)vpu_buffer_ptr;

    buf_addr.m_vir_addr =  vb->base;
    buf_addr.m_shared_fd = -1;
    buf_addr.m_phy_addr =  vb->phys_addr;
    buf_addr.m_size = vb->size;
    buf_addr.m_vir_addr = vb->virt_addr;
    buf_addr.m_type = mmp_buffer::ION;

    mmpResult = mmp_buffer_mgr::get_instance()->free_buffer(buf_addr);
    if(mmpResult == MMP_SUCCESS) {
        iret = 0;
    }
    else {
        iret = -1;
    }

    return iret;
}
#endif



/**********************************************************
class members
**********************************************************/

mmp_buffer_mgr::mmp_buffer_mgr() {

}

mmp_buffer_mgr::~mmp_buffer_mgr() {

}

