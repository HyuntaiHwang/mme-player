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

#include "mmp_oal_mutex_win32.hpp"

#if (MMP_OS == MMP_OS_WIN32)

/**********************************************************
class members
**********************************************************/

mmp_oal_mutex_win32::mmp_oal_mutex_win32(MMP_U32 key) : mmp_oal_mutex(key)
,m_hMutex(NULL)
{

}

mmp_oal_mutex_win32::~mmp_oal_mutex_win32() {

}


MMP_ERRORTYPE mmp_oal_mutex_win32::open() {

	MMP_RESULT ret = MMP_ErrorNone;
    char mutex_name[32];

    if(m_key == 0) {
        m_hMutex = CreateMutex(NULL, FALSE, NULL);
    }
    else {
        sprintf(mutex_name, "mmp_oal_mutex_win32-0x%08x", m_key);
	    m_hMutex = CreateMutex(NULL, FALSE, mutex_name);
    }
	if(m_hMutex == NULL) {
		ret = MMP_ErrorInsufficientResources;
	}


	return ret;
}

MMP_ERRORTYPE mmp_oal_mutex_win32::close() {
	
	if(m_hMutex != NULL) {
		::CloseHandle(m_hMutex);
		m_hMutex = NULL;
	}

	return MMP_ErrorNone;
}

void mmp_oal_mutex_win32::lock() {
	WaitForSingleObject(m_hMutex, INFINITE);
}
	
void mmp_oal_mutex_win32::unlock() {
	::ReleaseMutex(m_hMutex);	
}

void* mmp_oal_mutex_win32::get_handle() {
	
	return m_hMutex;
}

#endif