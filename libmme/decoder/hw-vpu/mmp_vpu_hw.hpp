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

#ifndef MMP_VPU_HW_HPP__
#define MMP_VPU_HW_HPP__

#include "MmpDefine.h"


class mmp_vpu_hw {

private:
    static class mmp_vpu_hw* s_p_instance;

public:
    static MMP_RESULT create_instance();
	static MMP_RESULT destroy_instance();

public:
    virtual ~mmp_vpu_hw() {}

    static class mmp_vpu_hw* get_instance() { return s_p_instance; }

    virtual MMP_RESULT hw_write(char* buf, int bufsize) = 0;
    virtual MMP_RESULT hw_ioctl(unsigned long request, void* arg) = 0;
    virtual void* hw_mmap(void *addr, size_t length, int prot, int flags, unsigned int offset) = 0;
    virtual MMP_RESULT hw_munmap(void* start, size_t length) = 0;

    virtual int get_fd() { return -1;}
};

#ifdef __cplusplus
extern "C" {
#endif

    MMP_RESULT mmp_vpu_hw_write(char* buf, int bufsize);
    //MMP_RESULT mmp_vpu_hw_ioctl(unsigned long request, void* arg);
    //void* mmp_vpu_hw_mmap(void *addr, size_t length, int prot, int flags, unsigned int offset);
    //MMP_RESULT mmp_vpu_hw_munmap(void* start, size_t length);

#ifdef __cplusplus
}
#endif

#endif

