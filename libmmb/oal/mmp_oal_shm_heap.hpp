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

#ifndef MMP_OAL_SHM_HEAP_HPP__
#define MMP_OAL_SHM_HEAP_HPP__

#include "mmp_oal_shm.hpp"


class mmp_oal_shm_heap : public mmp_oal_shm {

friend class mmp_oal_shm;

private:
	void* m_p_shm;
    MMP_BOOL m_is_create;

protected:
    
	mmp_oal_shm_heap(struct mmp_oal_shm_create_config* p_create_config);
	virtual ~mmp_oal_shm_heap();

	virtual MMP_RESULT open();
	virtual MMP_RESULT close(MMP_BOOL is_remove_from_system);

public:
    virtual void* get_shm_ptr() { return m_p_shm; }
    virtual MMP_S32 get_shm_size() { return m_create_config.size; }
    virtual MMP_BOOL is_create()  { return m_is_create; }
    virtual MMP_S32 get_attach_process_count();

};


#endif
