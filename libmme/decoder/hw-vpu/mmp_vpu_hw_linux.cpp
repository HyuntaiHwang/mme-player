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

#include "mmp_vpu_hw_linux.hpp"

#include <signal.h>		/* SIGIO */
#include <fcntl.h>		/* fcntl */
#include <pthread.h>
#include <sys/mman.h>		/* mmap */
#include <sys/ioctl.h>		/* fopen/fread */
#include <sys/errno.h>		/* fopen/fread */
#include <sys/types.h>
#include <sys/time.h>

#include "mmp_env_mgr.hpp"

#ifdef WIN32
#define LINUX_DRIVER_OPEN  kernel_driver_open
#define LINUX_DRIVER_CLOSE kernel_driver_close
#define LINUX_DRIVER_WRITE kernel_driver_write
#define LINUX_DRIVER_MMAP  kernel_driver_mmap
#define LINUX_DRIVER_MUNMAP  kernel_driver_munmap
#define LINUX_DRIVER_IOCTL kernel_driver_ioctl

extern int kernel_driver_open(const char* drvname, int flag);
extern int kernel_driver_close(int fd);
extern int kernel_driver_write(int fd, char* buf, int bufsize);
extern void* kernel_driver_mmap(void *addr, size_t length, int prot, int flags, int fd, unsigned int offset);
extern int kernel_driver_munmap(void* start, size_t length);
extern int kernel_driver_ioctl(int d, unsigned long request, void* arg);

#else
#define LINUX_DRIVER_OPEN  ::open
#define LINUX_DRIVER_CLOSE ::close
#define LINUX_DRIVER_WRITE ::write
#define LINUX_DRIVER_MMAP  ::mmap
#define LINUX_DRIVER_MUNMAP ::munmap
#define LINUX_DRIVER_IOCTL ::ioctl

#endif

mmp_vpu_hw_linux::mmp_vpu_hw_linux() {

    
    m_fd = LINUX_DRIVER_OPEN("/dev/vpu", O_RDWR);

    MMPDEBUGMSG(1, (TEXT("[mmp_vpu_hw_linux::mmp_vpu_hw_linux] m_fd=%d  "), m_fd));
}

mmp_vpu_hw_linux::~mmp_vpu_hw_linux() {
    if(m_fd >= 0) {
        LINUX_DRIVER_CLOSE(m_fd);
    }
}

MMP_RESULT mmp_vpu_hw_linux::hw_write(char* buf, int bufsize) {

    MMP_RESULT mmpResult = MMP_FAILURE;

    if(m_fd >= 0) {
        if( LINUX_DRIVER_WRITE(m_fd, buf, bufsize) == bufsize) {
            mmpResult = MMP_SUCCESS;
        }
	}

    return mmpResult;
}

MMP_RESULT mmp_vpu_hw_linux::hw_ioctl(unsigned long request, void* arg) {

    int ret;
    MMP_RESULT mmpResult = MMP_FAILURE;

    if(m_fd >= 0) {
        ret = LINUX_DRIVER_IOCTL(m_fd, request, arg);
        if(ret == 0) {
            mmpResult = MMP_SUCCESS;
        }
    }

    return mmpResult;
}

void* mmp_vpu_hw_linux::hw_mmap(void *addr, size_t length, int prot, int flags, unsigned int offset) {

    void* ptr = NULL;
    void* ptr1 = NULL;
    
    //printf("[mmp_vpu_hw_linux::hw_mmap] 0000 m_fd=%d addr=0x%08x length=0x%08x offset=0x%08x \n", m_fd, addr, length, offset);
    
    if(m_fd >= 0) {

#ifdef WIN32
        ptr = LINUX_DRIVER_MMAP(addr, length, prot, flags, m_fd, offset);
#else
        if(offset!=0) {
            ptr = LINUX_DRIVER_MMAP(NULL, length, prot, flags, m_fd, 4096);
        }
        else {   
            ptr = LINUX_DRIVER_MMAP(addr, length, prot, flags, m_fd, offset);
        }
#endif
        //ptr = LINUX_DRIVER_MMAP((void*)offset, length, prot, flags, m_fd, 0);
    }
    //printf("[mmp_vpu_hw_linux::hw_mmap] 1111 m_fd=%d addr=0x%08x length=0x%08x offset=0x%08x ptr=0x%08x \n", m_fd, addr, length, offset, ptr);

#if 0
    offset = 0xA0000000;
    ptr1 = LINUX_DRIVER_MMAP(addr, length, prot, flags, m_fd, offset);
    printf("[mmp_vpu_hw_linux::hw_mmap] m_fd=%d addr=0x%08x length=0x%08x offset=0x%08x ptr1=0x%08x \n", m_fd, addr, length, offset, ptr1);

    offset = 0x80000000;
    ptr1 = LINUX_DRIVER_MMAP(addr, length, prot, flags, m_fd, offset);
    printf("[mmp_vpu_hw_linux::hw_mmap] m_fd=%d addr=0x%08x length=0x%08x offset=0x%08x ptr1=0x%08x \n", m_fd, addr, length, offset, ptr1);
#endif

    return ptr;;
}

MMP_RESULT mmp_vpu_hw_linux::hw_munmap(void* start, size_t length) {

    int ret;
    MMP_RESULT mmpResult = MMP_FAILURE;

    if(m_fd >= 0) {
        ret = LINUX_DRIVER_MUNMAP(start, length);
        if(ret == 0) {
            mmpResult = MMP_SUCCESS;
        }
    }

    return mmpResult;
}

