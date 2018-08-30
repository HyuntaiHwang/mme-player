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

#include "mmp_oal_cond_win32.hpp"
#include "mmp_oal_mutex.hpp"

#if (MMP_OS == MMP_OS_WIN32)

/**********************************************************
class members
**********************************************************/

mmp_oal_cond_win32::mmp_oal_cond_win32() :
m_hSema(NULL)
,m_hMutexInternal(NULL)
,m_waitersCount(0)
{
	InitializeCriticalSection(&m_cs_waiter_count);
}

mmp_oal_cond_win32::~mmp_oal_cond_win32() {

	DeleteCriticalSection(&m_cs_waiter_count);
}


MMP_ERRORTYPE mmp_oal_cond_win32::open() {

	MMP_ERRORTYPE ret = MMP_ErrorNone;

/*
	HANDLE CreateSemaphore(  LPSECURITY_ATTRIBUTES lpSemaphoreAttributes, 
							LONG lInitialCount,   
							LONG lMaximumCount,   
							LPCTSTR lpName ); 
*/
	m_hSema = CreateSemaphore(NULL, 0, 0x7fffffff, NULL);
	if(m_hSema == NULL) {
		ret = MMP_ErrorInsufficientResources;
	}

	/*
	HANDLE CreateMutex(   LPSECURITY_ATTRIBUTES lpMutexAttributes, 
						BOOL bInitialOwner,   
						LPCTSTR lpName );
	*/
	if(ret == MMP_ErrorNone) {
	
		m_hMutexInternal = CreateMutex(NULL, FALSE, NULL);
		if(m_hMutexInternal == NULL) {
			ret = MMP_ErrorInsufficientResources;
		}
	}
	
	return ret;
}

MMP_ERRORTYPE mmp_oal_cond_win32::close() {
	

	if(m_hSema != NULL) {
		::CloseHandle(m_hSema);
		m_hSema = NULL;
	}

	if(m_hMutexInternal != NULL) {
		::CloseHandle(m_hMutexInternal);
		m_hMutexInternal = NULL;
	}
	
	return MMP_ErrorNone;
}

void mmp_oal_cond_win32::signal() {
	
	bool haveWaiters;

	::WaitForSingleObject(this->m_hMutexInternal, INFINITE); //Wait to get  MUTEX Ownership.

	EnterCriticalSection(&m_cs_waiter_count);
	haveWaiters = (this->m_waitersCount>0)?true:false;
	LeaveCriticalSection(&m_cs_waiter_count);

	if(haveWaiters) {
        ReleaseSemaphore(m_hSema, 1, 0);
	}

	/*
		When the owning thread no longer needs to own the mutex object, 
		it calls the ReleaseMutex function. 
	*/
	::ReleaseMutex(this->m_hMutexInternal); 
}
	
void mmp_oal_cond_win32::wait(class mmp_oal_mutex* p_mutex) {
	
	DWORD res;
	HANDLE hMutex;

	hMutex = (HANDLE)p_mutex->get_handle();

	// Increment the wait count, avoiding race conditions.
    EnterCriticalSection(&m_cs_waiter_count);
    this->m_waitersCount++;
    LeaveCriticalSection(&m_cs_waiter_count);
    
	res =  SignalObjectAndWait(hMutex, m_hSema, INFINITE, FALSE); //Signal hMutex, and Wait Semaphore

	EnterCriticalSection(&m_cs_waiter_count);
    this->m_waitersCount--;
    LeaveCriticalSection(&m_cs_waiter_count);
    
    WaitForSingleObject(m_hMutexInternal, INFINITE);

	// Release the internal and grab the external.
    ReleaseMutex(m_hMutexInternal);
    WaitForSingleObject(hMutex, INFINITE);

}

#endif