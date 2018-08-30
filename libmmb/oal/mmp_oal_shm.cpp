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

#include "mmp_oal_shm.hpp"
#include "mmp_oal_shm_win32.hpp"
#include "mmp_oal_shm_linux.hpp"
#include "mmp_oal_shm_heap.hpp"
#include "mmp_oal_shm_ashmem.hpp"
#include "mmp_oal_shm_androidheap.hpp"

/**********************************************************
create/destroy object
**********************************************************/

class mmp_oal_shm* mmp_oal_shm::create_object(MMP_U32 key, MMP_S32 size) {

    struct mmp_oal_shm_create_config create_config;

    create_config.type = mmp_oal_shm::DEFAULT;
    create_config.key = key;
    create_config.size = size;

    return mmp_oal_shm::create_object(&create_config);
}

class mmp_oal_shm* mmp_oal_shm::create_object(struct mmp_oal_shm_create_config* p_create_config) {

	class mmp_oal_shm* p_obj = NULL;

    switch(p_create_config->type) {
        case mmp_oal_shm::ANDROID_HEAP:
            //p_obj = new class mmp_oal_shm_androidheap(p_create_config);
            break;

        default:
            #if (MMP_PLATFORM == MMP_PLATFORM_WIN32)
	            p_obj = new class mmp_oal_shm_win32(p_create_config);

            #elif (MMP_PLATFORM == MMP_PLATFORM_ANDROID)
                p_obj = new class mmp_oal_shm_heap(p_create_config);
                //p_obj = new class mmp_oal_shm_ashmem(p_create_config);

            #elif (MMP_PLATFORM == MMP_PLATFORM_TIZEN)
                p_obj = new class mmp_oal_shm_linux(p_create_config);

            #else
            #error "ERROR : Select Platform "
            #endif
    }

	if(p_obj!=NULL) {
        
		if(p_obj->open() != MMP_ErrorNone)    
		{
			p_obj->close(MMP_FALSE);
			delete p_obj;
			p_obj = NULL;
		}
	}

    return p_obj;
}

MMP_ERRORTYPE mmp_oal_shm::destroy_object(class mmp_oal_shm* p_obj, MMP_BOOL is_remove_from_system) {

	if(p_obj != NULL) {
        p_obj->close(is_remove_from_system);
        delete p_obj;
    }

    return MMP_ErrorNone;
}

/**********************************************************
class members
**********************************************************/

mmp_oal_shm::mmp_oal_shm(struct mmp_oal_shm_create_config* p_create_config) :
m_create_config(*p_create_config)
{

}

mmp_oal_shm::~mmp_oal_shm() {

}

#if 0
MMP_ERRORTYPE mmp_oal_shm::open() {
	
	return MMP_ErrorNone;
}

MMP_ERRORTYPE mmp_oal_shm::close() {
	
	return MMP_ErrorNone;
}
#endif


