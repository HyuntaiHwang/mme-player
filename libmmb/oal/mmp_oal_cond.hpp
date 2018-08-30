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

#ifndef MMP_OAL_COND_HPP__
#define MMP_OAL_COND_HPP__

#include "MmpDefine.h"

/*
	thread condition class
*/
class mmp_oal_mutex;

class mmp_oal_cond {

public:
	static class mmp_oal_cond* create_object(void);
	static MMP_ERRORTYPE destroy_object(class mmp_oal_cond* p_obj);

protected:
    
	mmp_oal_cond();
	virtual ~mmp_oal_cond();

	virtual MMP_ERRORTYPE open();
	virtual MMP_ERRORTYPE close();

public:
	virtual void signal() = 0;
	virtual void wait(class mmp_oal_mutex* p_mutex) = 0;
};

#endif
