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

#include "mmp_oal_task_win32.hpp"

#if (MMP_OS == MMP_OS_WIN32)

/**********************************************************
class members
**********************************************************/

mmp_oal_task_win32::mmp_oal_task_win32(struct mmp_oal_task_create_config* p_create_config) : mmp_oal_task(p_create_config),
m_hThread(NULL)
{

}

mmp_oal_task_win32::~mmp_oal_task_win32() {

}


MMP_ERRORTYPE mmp_oal_task_win32::open() {

	MMP_ERRORTYPE ret = MMP_ErrorNone;

	//m_hThread = ::CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)this->m_create_config.service, this->m_create_config.arg, CREATE_SUSPENDED, NULL);
    m_hThread = ::CreateThread(NULL, 1024*32, (LPTHREAD_START_ROUTINE)this->m_create_config.service, this->m_create_config.arg, 0, NULL);

    //SetThreadPriority(m_hThread, THREAD_PRIORITY_TIME_CRITICAL);

	return ret;
}

MMP_ERRORTYPE mmp_oal_task_win32::close() {
	
	if(m_hThread != NULL) {
		::WaitForSingleObject(m_hThread, INFINITE);
		::CloseHandle(m_hThread);
	}

	return MMP_ErrorNone;
}

void mmp_oal_task_win32::suspend() {
	::SuspendThread(m_hThread);
}
	
void mmp_oal_task_win32::resume() {
	::ResumeThread(m_hThread);	
}

#endif