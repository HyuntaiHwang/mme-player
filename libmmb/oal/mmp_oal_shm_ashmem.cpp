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

#include "mmp_oal_shm_ashmem.hpp"
#include "cutils/ashmem.h"


#define ASHMEM_NAME "mmetest_ashmem"
#define ASHMEM_KEY  0x9829

/**********************************************************
class members
**********************************************************/

mmp_oal_shm_ashmem::mmp_oal_shm_ashmem(struct mmp_oal_shm_create_config* p_create_config) : mmp_oal_shm(p_create_config)
,m_fd(-1)
,m_shm_buf(NULL)
{

}

mmp_oal_shm_ashmem::~mmp_oal_shm_ashmem() {

}


MMP_RESULT mmp_oal_shm_ashmem::open() {

	MMP_RESULT mmpResult = MMP_FAILURE;
    char ashname[256];

    sprintf(ashname, "ashmem-%08x", m_create_config.key );

    m_fd = ashmem_create_region(ashname, m_create_config.size);
    if(m_fd >= 0) {
        m_shm_buf = MMP_DRIVER_MMAP(0, m_create_config.size, PROT_READ|PROT_WRITE, MAP_SHARED, m_fd, 0);
        if(m_shm_buf!=NULL) {
            mmpResult = MMP_SUCCESS;
        }
    }
            
    return mmpResult;
}

MMP_RESULT mmp_oal_shm_ashmem::close(MMP_BOOL is_remove_from_system) {
	
    if(m_shm_buf!=NULL) {
        MMP_DRIVER_MUNMAP(m_shm_buf, m_create_config.size);
        m_shm_buf = NULL;
    }

    if(m_fd >= 0) {
        MMP_DRIVER_CLOSE(m_fd);
        m_fd = -1;
    }

    return MMP_SUCCESS;
}

MMP_S32 mmp_oal_shm_ashmem::get_attach_process_count() {
    
    MMP_S32 proc_count = 0;
    return proc_count;
}

