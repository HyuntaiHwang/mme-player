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

#include "mmp_buffer_ion.hpp"

#include "ion.h"

#include <sys/mman.h>
#include <signal.h>		/* SIGIO */
#include <fcntl.h>		/* fcntl */
#include <pthread.h>
#include <sys/mman.h>		/* mmap */
#include <sys/ioctl.h>		/* fopen/fread */
#include <sys/errno.h>		/* fopen/fread */
#include <sys/types.h>
#include <sys/time.h>

#include "MmpUtil.hpp"

#define ION_ALLOC_DEBUG 0
#define ION_FREE_DEBUG 0 

#if (MMP_OS==MMP_OS_WIN32)
#undef open
#endif

mmp_buffer_ion::mmp_buffer_ion(struct mmp_buffer_create_config *p_create_config) : mmp_buffer(p_create_config)
{
    
}

mmp_buffer_ion::~mmp_buffer_ion() {

}

MMP_RESULT mmp_buffer_ion::open() {

    int ion_fd, ret;
    MMP_RESULT mmpResult = MMP_FAILURE;
    unsigned int ion_alloc_flags = 0;

    ion_fd = ion_open();
    if(ion_fd >= 0) {
        
        if(this->is_cached() == MMP_TRUE)  ion_alloc_flags = ION_FLAG_CACHED;
        else ion_alloc_flags = 0;

        //ion_alloc_flags |= ION_FLAG_CACHED_NEEDS_SYNC;
        //ion_alloc_flags |= ION_FLAG_CACHED;
        ret = ion_alloc_fd(ion_fd, this->m_buf_addr.m_size, 0, ION_HEAP_CARVEOUT_MASK,  ion_alloc_flags, (int*)&this->m_buf_addr.m_shared_fd);
        if(ret < 0) {
            /* error */
            this->m_buf_addr.m_shared_fd = -1;
            MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[mmp_buffer_ion::open] FAIL: ion_alloc_fd(ion_fd=%d) sz=%d"), ion_fd, this->m_buf_addr.m_size ));
        }
        else {
            this->m_buf_addr.m_vir_addr = (unsigned int)MMP_DRIVER_MMAP(NULL, this->m_buf_addr.m_size, (PROT_READ | PROT_WRITE), MAP_SHARED, this->m_buf_addr.m_shared_fd, 0);
            mmpResult = MMP_SUCCESS;

            this->m_buf_addr.m_phy_addr = this->get_phy_addr_from_shared_fd(this->m_buf_addr.m_shared_fd);

            MMPDEBUGMSG(ION_ALLOC_DEBUG, (TEXT("[mmp_buffer_ion::open] fd=%d vir_addr=0x%08x phy_addr=0x%08x sz=%d "), 
                 this->m_buf_addr.m_shared_fd,
                 this->m_buf_addr.m_vir_addr,   
                 this->m_buf_addr.m_phy_addr,
                 this->m_buf_addr.m_size
                 ));
        }
        ion_close(ion_fd);
    }

    return mmpResult;
}

MMP_RESULT mmp_buffer_ion::close() {

    if(this->m_buf_addr.m_shared_fd >= 0) {

        MMP_DRIVER_MUNMAP((void*)this->m_buf_addr.m_vir_addr, this->m_buf_addr.m_size);
        ion_close(this->m_buf_addr.m_shared_fd);

        MMPDEBUGMSG(ION_FREE_DEBUG, (TEXT("[mmp_buffer_ion::close] fd=%d vir_addr=0x%08x phy_addr=0x%08x sz=%d "), 
                         this->m_buf_addr.m_shared_fd,
                         this->m_buf_addr.m_vir_addr,   
                         this->m_buf_addr.m_phy_addr,
                         this->m_buf_addr.m_size
                         ));

        this->m_buf_addr.m_shared_fd = -1;
        this->m_buf_addr.m_vir_addr = 0;
    }


    return MMP_SUCCESS;
}

MMP_RESULT mmp_buffer_ion::sync_buf() {
    
    MMP_S32 ion_fd;

    ion_fd = ion_open();
    if(ion_fd >= 0) {
        if(this->m_buf_addr.m_shared_fd >= 0) {
            ion_sync_fd(ion_fd, this->m_buf_addr.m_shared_fd);
        }
        ion_close(ion_fd);
    }
    return MMP_SUCCESS;
}