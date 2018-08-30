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

#include "mmp_oal_shm_win32.hpp"

#if (MMP_OS == MMP_OS_WIN32)

/**********************************************************
class members
**********************************************************/

mmp_oal_shm_win32::mmp_oal_shm_win32(struct mmp_oal_shm_create_config* p_create_config) : mmp_oal_shm(p_create_config)
,m_hShm(NULL)
,m_p_shm(NULL)
,m_is_create(MMP_TRUE)
{

}

mmp_oal_shm_win32::~mmp_oal_shm_win32() {

}


MMP_RESULT mmp_oal_shm_win32::open() {

	MMP_RESULT mmpResult = MMP_SUCCESS;
    MMP_CHAR keystring[32];

    sprintf(keystring, "%x", this->m_create_config.key);

    m_hShm = ::OpenFileMapping(FILE_MAP_WRITE|FILE_MAP_READ, TRUE, keystring);
    if(m_hShm != NULL) {
        m_p_shm = ::MapViewOfFile(m_hShm, FILE_MAP_WRITE|FILE_MAP_READ, 0, 0, 0);
        if(m_p_shm == NULL) {
            ::CloseHandle(m_hShm);
            m_hShm = NULL;
        }
        else {
            m_is_create = MMP_FALSE;
        }
    }
    
    if(m_p_shm == NULL) {
        
        SECURITY_ATTRIBUTES sa;
        
        sa.nLength = sizeof(SECURITY_ATTRIBUTES);
        sa.lpSecurityDescriptor = NULL;
        sa.bInheritHandle = TRUE;


        m_hShm = CreateFileMapping(INVALID_HANDLE_VALUE, &sa, PAGE_READWRITE, 0, this->m_create_config.size, keystring);
        if(m_hShm != NULL) {
            m_p_shm = ::MapViewOfFile(m_hShm, FILE_MAP_WRITE|FILE_MAP_READ, 0, 0, 0);
            if(m_p_shm == NULL) {
                ::CloseHandle(m_hShm);
                m_hShm = NULL;
            }
        }
    }

    if(m_p_shm == NULL) {
        mmpResult = MMP_FAILURE;
    }

	return mmpResult;
}

MMP_RESULT mmp_oal_shm_win32::close(MMP_BOOL is_remove_from_system) {
	
    if(m_hShm != NULL) {
        ::CloseHandle(m_hShm);
        m_hShm = NULL;
    }

	return MMP_SUCCESS;
}

#endif