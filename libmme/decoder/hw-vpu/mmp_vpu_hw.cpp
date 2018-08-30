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

#include "mmp_vpu_hw.hpp"
#include "mmp_vpu_hw_ffmpeg.hpp"
#include "mmp_vpu_hw_linux.hpp"

class mmp_vpu_hw* mmp_vpu_hw::s_p_instance = NULL;

MMP_RESULT mmp_vpu_hw::create_instance() {

    class mmp_vpu_hw* p_obj;
    MMP_RESULT mmpResult = MMP_FAILURE;

    if(mmp_vpu_hw::s_p_instance == NULL) {

        //p_obj = new mmp_vpu_hw_ffmpeg;
        p_obj = new mmp_vpu_hw_linux;
        if(p_obj != NULL) {
            mmp_vpu_hw::s_p_instance = p_obj;
        }
    }

    if(mmp_vpu_hw::s_p_instance != NULL) {
        mmpResult = MMP_SUCCESS;
    }

    return mmpResult;
}

    
MMP_RESULT mmp_vpu_hw::destroy_instance() {

    MMP_RESULT mmpResult = MMP_FAILURE;

    if(mmp_vpu_hw::s_p_instance != NULL) {
        delete mmp_vpu_hw::s_p_instance;
        mmp_vpu_hw::s_p_instance = NULL;
    }

    if(mmp_vpu_hw::s_p_instance == NULL) {
        mmpResult = MMP_SUCCESS;
    }

    return mmpResult;
}




MMP_RESULT mmp_vpu_hw_write(char* buf, int bufsize) {
    
    return mmp_vpu_hw::get_instance()->hw_write(buf, bufsize);
}

