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

#ifndef MMP_VPU_DEF_H__
#define MMP_VPU_DEF_H__

#include "MmpDefine.h"

#include <signal.h>		/* SIGIO */
#include <fcntl.h>		/* fcntl */
#include <pthread.h>
#include <sys/mman.h>		/* mmap */
#include <sys/ioctl.h>		/* fopen/fread */
#include <sys/errno.h>		/* fopen/fread */
#include <sys/types.h>
#include <sys/time.h>
#include "vpu.h"


#include "vpuapi.h"
#include "vpuapi_mme.h"
#include "vpurun.h"
#include "vpuapifunc.h"
#include "vpuhelper.h"

#	define SUPPORT_INTERRUPT
#	define VPU_BIT_REG_SIZE	(0x4000*VPU_MAX_NUM_VPU_CORE)
#	define VDI_SYSTEM_ENDIAN VDI_LITTLE_ENDIAN
#define VDI_NUM_LOCK_HANDLES				4

#ifdef SUPPORT_MULTI_CORE_IN_ONE_DRIVER
#define VPU_CORE_BASE_OFFSET 0x4000
#endif

#endif
