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

#ifndef MMP_CSC_HPP__
#define MMP_CSC_HPP__

#include "MmpDefine.h"
#include "mmp_buffer_videoframe.hpp"


class mmp_csc {

public:
    struct create_config {
	    int type;
    };

    enum {
        SW=0,
        NEON
    };

    static class mmp_csc* create_object(struct mmp_csc::create_config *p_create_config);
	static MMP_RESULT destroy_object(class mmp_csc* p_obj);

protected:
   
	mmp_csc();
	virtual ~mmp_csc();

	virtual MMP_RESULT open() = 0;
	virtual MMP_RESULT close() = 0;
    
public:

    virtual MMP_RESULT start() { return MMP_SUCCESS; }
	virtual MMP_RESULT stop() { return MMP_SUCCESS; }
    
    virtual MMP_RESULT convert(class mmp_buffer_videoframe* p_buf_vf_src, class mmp_buffer_videoframe* p_buf_vf_dst) = 0;

};

#endif

