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

#ifndef MMP_OAL_TASK_LINUX_HPP__
#define MMP_OAL_TASK_LINUX_HPP__

#include "mmp_oal_task.hpp"

#if (MMP_OS == MMP_OS_LINUX)

class mmp_oal_task_linux : public mmp_oal_task {

friend class mmp_oal_task;

private:
	pthread_t m_task_hdl;
	pthread_attr_t m_attr;
	void* m_stack;
	
protected:
    
	mmp_oal_task_linux(struct mmp_oal_task_create_config* p_create_config);
	virtual ~mmp_oal_task_linux();

	virtual MMP_ERRORTYPE open();
	virtual MMP_ERRORTYPE close();

public:
	virtual void suspend();
	virtual void resume();	
};


#endif 
#endif
