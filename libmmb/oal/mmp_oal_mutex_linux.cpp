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

#include "mmp_oal_mutex_linux.hpp"

#if (MMP_OS == MMP_OS_LINUX)

#if (MMP_PLATFORM == MMP_PLATFORM_ANDROID)
#define MMP_OAL_MUTEX_INIT_VALUE __PTHREAD_MUTEX_INIT_VALUE
#else
#define MMP_OAL_MUTEX_INIT_VALUE PTHREAD_MUTEX_INITIALIZER
#endif

/**********************************************************
class members
**********************************************************/

mmp_oal_mutex_linux::mmp_oal_mutex_linux(MMP_U32 key) : mmp_oal_mutex(key) 
,m_is_init(MMP_FALSE)
{

}

mmp_oal_mutex_linux::~mmp_oal_mutex_linux() {

}


MMP_RESULT mmp_oal_mutex_linux::open() {

	MMP_RESULT ret = MMP_SUCCESS;

	pthread_mutexattr_init(&m_mutexattr);
	pthread_mutexattr_setpshared(&m_mutexattr, PTHREAD_PROCESS_SHARED);
		
	if(pthread_mutex_init(&m_mutex, &m_mutexattr) != 0) {
        ret = MMP_ErrorUndefined;
    }
    else {
        m_is_init = MMP_TRUE;
    }

	return ret;
}

MMP_RESULT mmp_oal_mutex_linux::close() {
	
	MMP_RESULT ret = MMP_SUCCESS;

    if(m_is_init == MMP_TRUE) {
	
		if (pthread_mutex_destroy(&m_mutex) != 0) {
			ret = MMP_ErrorUndefined;
		}

		pthread_mutexattr_destroy(&m_mutexattr);
	}

	return ret;
}

void mmp_oal_mutex_linux::lock() {
	
	pthread_mutex_lock(&m_mutex);
}
	
void mmp_oal_mutex_linux::unlock() {

	pthread_mutex_unlock(&m_mutex);
}

void* mmp_oal_mutex_linux::get_handle() {

	return (void*)&m_mutex;
}

#endif
