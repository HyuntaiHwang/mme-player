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

#include "mmp_oal_cond_linux.hpp"
#include "mmp_oal_mutex.hpp"

#if (MMP_OS == MMP_OS_LINUX)

/**********************************************************
class members
**********************************************************/

mmp_oal_cond_linux::mmp_oal_cond_linux() 
{
	if(1) { //type == SHARED) {
        pthread_condattr_t attr;
        pthread_condattr_init(&attr);
        pthread_condattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
        pthread_cond_init(&mCond, &attr);
        pthread_condattr_destroy(&attr);
    } else {
        pthread_cond_init(&mCond, NULL);
    }
}

mmp_oal_cond_linux::~mmp_oal_cond_linux() {

	pthread_cond_destroy(&mCond);
}


MMP_ERRORTYPE mmp_oal_cond_linux::open() {

	MMP_ERRORTYPE ret = MMP_ErrorNone;

	

	return ret;
}

MMP_ERRORTYPE mmp_oal_cond_linux::close() {
	

	
	return MMP_ErrorNone;
}

void mmp_oal_cond_linux::signal() {
	
	pthread_cond_signal(&mCond);
}
	
void mmp_oal_cond_linux::wait(class mmp_oal_mutex* p_mutex) {
	
	pthread_mutex_t* p_mutex_t;

	p_mutex_t = (pthread_mutex_t*)p_mutex->get_handle();

	pthread_cond_wait(&mCond, p_mutex_t);

}

#endif