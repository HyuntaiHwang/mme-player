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

#include "mmp_oal_mutex.hpp"
#include "mmp_oal_mutex_win32.hpp"
#include "mmp_oal_mutex_linux.hpp"
#if (MMP_PLATFORM != MMP_PLATFORM_ANDROID)
#include "mmp_oal_mutex_linux_sema.hpp"
#endif

/**********************************************************
create/destroy object
**********************************************************/

class mmp_oal_mutex* mmp_oal_mutex::create_object(MMP_U32 key) {

	class mmp_oal_mutex* p_obj = NULL;

#if (MMP_OS == MMP_OS_WIN32)
	p_obj = new class mmp_oal_mutex_win32(key);
#elif (MMP_OS == MMP_OS_LINUX)
	p_obj = new class mmp_oal_mutex_linux(key);
    //p_obj = new class mmp_oal_mutex_linux_sema(key);
#else
#error "ERROR : Select OS "
#endif
	if(p_obj!=NULL) {
        
		if(p_obj->open( ) != MMP_ErrorNone)    
		{
			p_obj->close();
			delete p_obj;
			p_obj = NULL;
		}
	}

    return p_obj;
}

MMP_ERRORTYPE mmp_oal_mutex::destroy_object(class mmp_oal_mutex* p_obj) {

	if(p_obj != NULL) {
        p_obj->close();
        delete p_obj;
    }

    return MMP_ErrorNone;
}

/**********************************************************
class members
**********************************************************/

mmp_oal_mutex::mmp_oal_mutex(MMP_U32 key) :
m_key(key)
{

}

mmp_oal_mutex::~mmp_oal_mutex() {

}


MMP_ERRORTYPE mmp_oal_mutex::open() {
	
	return MMP_ErrorNone;
}

MMP_ERRORTYPE mmp_oal_mutex::close() {
	
	return MMP_ErrorNone;
}


