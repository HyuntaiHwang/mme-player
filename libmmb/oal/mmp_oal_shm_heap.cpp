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

#include "mmp_oal_shm_heap.hpp"


/**********************************************************
class members
**********************************************************/

mmp_oal_shm_heap::mmp_oal_shm_heap(struct mmp_oal_shm_create_config* p_create_config) : mmp_oal_shm(p_create_config)
,m_p_shm(NULL)
,m_is_create(MMP_FALSE)
{

}

mmp_oal_shm_heap::~mmp_oal_shm_heap() {

}


MMP_RESULT mmp_oal_shm_heap::open() {

	MMP_RESULT mmpResult = MMP_SUCCESS;
    
    m_is_create = MMP_TRUE;
    m_p_shm = MMP_MALLOC(m_create_config.size);
    
    return mmpResult;
}

MMP_RESULT mmp_oal_shm_heap::close(MMP_BOOL is_remove_from_system) {
	
    if(m_p_shm != NULL) {
        MMP_FREE(m_p_shm);
        m_p_shm = NULL;
    }

    return MMP_SUCCESS;
}

MMP_S32 mmp_oal_shm_heap::get_attach_process_count() {
    
    return 1;
}

