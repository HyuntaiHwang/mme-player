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

#ifndef MMP_OAL_MUTEX_LINUX_SEMA_HPP__
#define MMP_OAL_MUTEX_LINUX_SEMA_HPP__

#include "mmp_oal_mutex.hpp"

#if (MMP_OS == MMP_OS_LINUX)

#include <semaphore.h>

class mmp_oal_mutex_linux_sema : public mmp_oal_mutex {

friend class mmp_oal_mutex;

private:
	sem_t *m_p_sem;
    sem_t m_sem;
    int m_sem_id;
	
protected:
    
	mmp_oal_mutex_linux_sema(MMP_U32 key);
	virtual ~mmp_oal_mutex_linux_sema();

	virtual MMP_ERRORTYPE open();
	virtual MMP_ERRORTYPE close();

public:
	virtual void lock();
	virtual void unlock();	
	virtual void* get_handle();
};

#endif
#endif
