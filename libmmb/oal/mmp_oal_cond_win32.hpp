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

#ifndef MMP_OAL_COND_WIN32_HPP__
#define MMP_OAL_COND_WIN32_HPP__

#include "mmp_oal_cond.hpp"

#if (MMP_OS == MMP_OS_WIN32)

class mmp_oal_cond_win32 : public mmp_oal_cond {

friend class mmp_oal_cond;

private:
	HANDLE m_hSema;
	HANDLE m_hMutexInternal;
	CRITICAL_SECTION m_cs_waiter_count;
	MMP_S32 m_waitersCount;
	
protected:
    
	mmp_oal_cond_win32();
	virtual ~mmp_oal_cond_win32();

	virtual MMP_ERRORTYPE open();
	virtual MMP_ERRORTYPE close();

public:
	virtual void signal();
	virtual void wait(class mmp_oal_mutex* p_mutex);	
};


#endif 
#endif
