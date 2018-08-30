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

#ifndef MMP_VPU_HW_FFMPEG_HPP__
#define MMP_VPU_HW_FFMPEG_HPP__

#include "mmp_vpu_hw.hpp"

class mmp_vpu_hw_ffmpeg : public mmp_vpu_hw {

public:
    virtual MMP_RESULT hw_write(char* buf, int bufsize);
    virtual MMP_RESULT hw_ioctl(unsigned long request, void* arg);
    virtual void* hw_mmap(void *addr, size_t length, int prot, int flags, unsigned int offset);
    virtual MMP_RESULT hw_munmap(void* start, size_t length);
};

#endif

