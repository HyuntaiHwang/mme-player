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

#ifndef MMP_OAL_MUTEX_HPP__
#define MMP_OAL_MUTEX_HPP__

#include "mmp_oal_lock.hpp"

class mmp_oal_mutex : public mmp_oal_lock {

public:
	static class mmp_oal_mutex* create_object(MMP_U32 key = 0);
	static MMP_RESULT destroy_object(class mmp_oal_mutex* p_obj);

protected:
    MMP_U32 m_key;

protected:
    
	mmp_oal_mutex(MMP_U32 key);
	virtual ~mmp_oal_mutex();

	virtual MMP_RESULT open();
	virtual MMP_RESULT close();

public:
	virtual void lock() = 0;
	virtual void unlock() = 0;
	virtual void* get_handle() = 0;
};

#endif
