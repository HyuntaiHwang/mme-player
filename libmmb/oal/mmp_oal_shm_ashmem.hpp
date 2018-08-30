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

#ifndef MMP_OAL_SHM_ASHMEM_HPP__
#define MMP_OAL_SHM_ASHMEM_HPP__

#include "mmp_oal_shm.hpp"

class mmp_oal_shm_ashmem : public mmp_oal_shm {

friend class mmp_oal_shm;

private:
    MMP_S32 m_fd;
    void* m_shm_buf;
	
protected:
    
	mmp_oal_shm_ashmem(struct mmp_oal_shm_create_config* p_create_config);
	virtual ~mmp_oal_shm_ashmem();

	virtual MMP_RESULT open();
	virtual MMP_RESULT close(MMP_BOOL is_remove_from_system);

public:
    virtual void* get_shm_ptr() { return (void*)m_shm_buf; }
    virtual MMP_S32 get_shm_size() { return m_create_config.size; }
    virtual MMP_BOOL is_create()  { return MMP_FALSE; }
    virtual MMP_S32 get_attach_process_count();

};


#endif
