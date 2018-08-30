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

#ifndef MMP_CSC_SW_HPP__
#define MMP_CSC_SW_HPP__

#include "mmp_csc.hpp"

class mmp_csc_sw : public mmp_csc {

friend class mmp_csc;

protected:
    mmp_csc_sw();
    ~mmp_csc_sw();

    virtual MMP_RESULT open();
	virtual MMP_RESULT close();
  
public:
    virtual MMP_RESULT convert(class mmp_buffer_videoframe* p_buf_vf_src, class mmp_buffer_videoframe* p_buf_vf_dst);
};

#endif

