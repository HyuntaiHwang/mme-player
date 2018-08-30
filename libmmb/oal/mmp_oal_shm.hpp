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

/* shared memory */
#ifndef MMP_OAL_SHM_HPP__
#define MMP_OAL_SHM_HPP__

#include "MmpDefine.h"

struct mmp_oal_shm_create_config {
    MMP_U32 type;
	MMP_U32 key;
	MMP_S32 size;
};

class mmp_oal_shm  {

public:
    enum {
        DEFAULT=0,
        ANDROID_HEAP=99
    };

public:
    static class mmp_oal_shm* create_object(MMP_U32 key, MMP_S32 size);
	static class mmp_oal_shm* create_object(struct mmp_oal_shm_create_config* p_create_config);
	static MMP_RESULT destroy_object(class mmp_oal_shm* p_obj, MMP_BOOL is_remove_from_system);

protected:
    struct mmp_oal_shm_create_config m_create_config;

protected:
    
	mmp_oal_shm(struct mmp_oal_shm_create_config* p_create_config);
	virtual ~mmp_oal_shm();

	virtual MMP_RESULT open() = 0;
	virtual MMP_RESULT close(MMP_BOOL is_remove_from_system) = 0;

public:
	virtual void* get_shm_ptr() = 0;
    virtual MMP_S32 get_shm_size() = 0;
    virtual MMP_BOOL is_create() = 0;
    virtual MMP_S32 get_attach_process_count() = 0;
};

#endif
