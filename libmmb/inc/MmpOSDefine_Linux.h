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

#ifndef MMPOSDEFINE_LINUX_H__
#define MMPOSDEFINE_LINUX_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <signal.h>
#include <stdarg.h>
#include <sys/time.h>
#include <unistd.h>
#include <limits.h>

#include <pthread.h>
#include <assert.h>

#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/mman.h>


//#ifdef __cplusplus
//#include <typeinfo>
//#endif

#define MMP_ALIGN_8 
#define MMP_DLL_EXPORT //extern _declspec(dllexport)

#if (MMP_PLATFORM == MMP_PLATFORM_ANDROID)
#include <utils/Log.h>
#endif

#define MMPDEBUGMSG(cond,printf_exp) do { if(cond) CMmpUtil_Printf printf_exp; }while(0);

#ifdef __cplusplus

#if (MMP_PLATFORM == MMP_PLATFORM_ANDROID)
#define MMP_CLASS_NAME ""
#else
#define MMP_CLASS_NAME typeid(*this).name()
#endif

#define MMP_CLASS_FUNC __func__
#define MMPDEBUGTRACE printf("!!!MmpTrace!!!   [%s::%s]  %d \n\r", MMP_CLASS_NAME, MMP_CLASS_FUNC, __LINE__ )
#endif


typedef char* MMPSTR;
typedef char MMPCHAR;
typedef char WCHAR;
typedef char TCHAR;

#define __MmpApiCall  //__stdcall //__stdcall __cdecl __fastcall


#define TEXT(x) (char*)x


#define MMP_FILE_DEVIDER '/'

#ifdef __cplusplus
#define MMP_DRIVER_OPEN    ::open
#define MMP_DRIVER_CLOSE   ::close
#define MMP_DRIVER_WRITE   ::write
#define MMP_DRIVER_READ    ::read
#define MMP_DRIVER_MMAP    ::mmap
#define MMP_DRIVER_MUNMAP  ::munmap
#define MMP_DRIVER_IOCTL   ::ioctl

#else
#define MMP_DRIVER_OPEN    open
#define MMP_DRIVER_CLOSE   close
#define MMP_DRIVER_WRITE   write
#define MMP_DRIVER_READ    read
#define MMP_DRIVER_MMAP    mmap
#define MMP_DRIVER_MUNMAP  munmap
#define MMP_DRIVER_IOCTL   ioctl
#endif


/* Class Name */
#if (MMP_PLATFORM == MMP_PLATFORM_ANDROID)
#define MMP_CNAME MMP_CLASS_FUNC  /* Class Name Tag */
#define MMP_CTAG  "[%s]" /* Class Name Tag */
#else
#define MMP_CNAME MMP_CLASS_NAME,MMP_CLASS_FUNC  /* Class Name Tag */
#define MMP_CTAG  "[%s::%s]" /* Class Name Tag */
#endif

/* Assert */
#define MMP_ASSERT assert

#ifdef __cplusplus
extern "C" {
#endif

MMP_RESULT CMmpUtil_Printf(char* lpszFormat, ... );

#ifdef __cplusplus
}
#endif

#endif

