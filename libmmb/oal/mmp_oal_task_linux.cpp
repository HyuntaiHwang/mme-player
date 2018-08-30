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

#include "mmp_oal_task_linux.hpp"

#if (MMP_OS == MMP_OS_LINUX)

/**********************************************************
class members
**********************************************************/

mmp_oal_task_linux::mmp_oal_task_linux(struct mmp_oal_task_create_config* p_create_config) : mmp_oal_task(p_create_config),
m_task_hdl(0)
{

}

mmp_oal_task_linux::~mmp_oal_task_linux() {

}


MMP_ERRORTYPE mmp_oal_task_linux::open() {

	MMP_ERRORTYPE ret = MMP_ErrorNone;
	int iret;
	
	pthread_attr_init(&m_attr);
	// use default stack size
	//pthread_attr_setstack(&m_attr, &m_stack, 1024*256);

	iret = pthread_create(&m_task_hdl, &m_attr, (void* (*)(void*))this->m_create_config.service, this->m_create_config.arg);
	if(iret != 0) {
		m_task_hdl = 0;
		ret = MMP_ErrorInsufficientResources;
	}

	return ret;
}

MMP_ERRORTYPE mmp_oal_task_linux::close() {
	
	void* result_t=NULL;

	if(m_task_hdl != 0) {
		
	    pthread_join(m_task_hdl, &result_t);
	}

	return MMP_ErrorNone;
}

void mmp_oal_task_linux::suspend() {
	
}
	
void mmp_oal_task_linux::resume() {
	
}

#endif