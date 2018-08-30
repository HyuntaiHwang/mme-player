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

#ifndef __MMPGLOBAL_H__
#define __MMPGLOBAL_H__

#include "MmpDefine_Type.h"

#define MMP_DEBUG_LEVEL 1  // 0: No Debug Msg  1: Output Debug Msg

/**********************************************************************************
 SOC/Board/OS/Platfom Definition
***********************************************************************************/

/* SOC */
#define MMP_SOC_UNKNOWN                0
#define MMP_SOC_WIN32                  1
#define MMP_SOC_EXYNOS4412             4412
#define MMP_SOC_GDM7243V               7243

/* Board */
#define MMP_BOARD_UNKNOWN              0
#define MMP_BOARD_WIN32                1
#define MMP_BOARD_DIAMOND              4412
#define MMP_BOARD_GDM7243V             7243

/* OS */
#define MMP_OS_UNKNOWN                 0
#define MMP_OS_LINUX                   1
#define MMP_OS_WIN32                   2
#define MMP_OS_WINCE                   3

/* Platform */
#define MMP_PLATFORM_UNKNOWN           0
#define MMP_PLATFORM_ANDROID           1
#define MMP_PLATFORM_TIZEN             2
#define MMP_PLATFORM_BUILDROOT         3
#define MMP_PLATFORM_WIN32             10


#ifdef WIN32

#define MMP_OS       MMP_OS_WIN32
#define MMP_BOARD    MMP_BOARD_WIN32
#define MMP_SOC      MMP_SOC_WIN32
#define MMP_PLATFORM MMP_PLATFORM_WIN32
#define MMP_PLATFORM_VERSION           0x00

#else

#define MMP_SOC               MMP_TARGET_SOC         /* SOC      */
#define MMP_BOARD             MMP_TARGET_BOARD       /* Board    */ 
#define MMP_OS                MMP_TARGET_OS          /* OS       */
#define MMP_PLATFORM          MMP_TARGET_PLATFORM    /* Platform */
#define MMP_PLATFORM_VERSION  MMP_TARGET_PLATFORM_VERSION


#endif

/**********************************************************************************
  Platform/OS Configuration
***********************************************************************************/

#if( MMP_OS==MMP_OS_WINCE)  //WinCE 6.0 Kernel Level Config
#include "MmpOSDefine_WinCE.h"

#elif( MMP_OS==MMP_OS_WIN32) //Win32 App Level Config
#include "MmpOSDefine_Win32.h"

#elif( MMP_OS==MMP_OS_LINUX) //Nucleus App/Driver Config
#include "MmpOSDefine_Linux.h"

#else
#error "ERROR Select MMP_OS in MmpDefine.h"
#endif//#ifdef MMPDEBUG


/**********************************************************************************
  Debug Option Define
***********************************************************************************/

#define MMPZONE_INIT               1
#define MMPZONE_REGISTERS          1
#define MMPZONE_ERROR              1
#define MMPZONE_VERBOSE            1
#define MMPZONE_WARNING            1
#define MMPZONE_UNUSED             1
#define MMPZONE_MONITOR            1
#define MMPZONE_INFO               1
#define MMPZONE_ISR                0


#define MMP_UnusedParameter(x)  x = x


#define IF_SAFE_RELEASE(p)      { if(p) { (p)->Release(); (p)=NULL; } }


enum MMP_PIXELFORMAT {
    MMP_PIXELFORMAT_UNKNOWN=0,
    MMP_PIXELFORMAT_RGB565,
    MMP_PIXELFORMAT_RGB24,   //24Bit
    MMP_PIXELFORMAT_RGB32, //32Bit
    MMP_PIXELFORMAT_YUV420_PACKED, // samples are packed  together into macropixels which ar stored in a single array
    MMP_PIXELFORMAT_YUV420_PLANAR, // where each component is stored as a separate array
    MMP_PIXELFORMAT_YUV420_PHYADDR, // yuv420, physical address
    MMP_PIXELFORMAT_RGB565_PHYADDR,
    MMP_PIXELFORMAT_STREAM,
    MMP_PIXELFORMAT_SAMSUNG_NV12
};

enum MMP_ENCDEC {
    MMP_ENC = 0,
    MMP_DEC
};


#define MMP_MAX(a,b) ((a) > (b) ? (a) : (b))
#define MMP_MIN(a,b) ((a) > (b) ? (b) : (a))

#define MMP_MAX(a,b) ((a) > (b) ? (a) : (b))
#define MMP_MAX3(a,b,c) MMP_MAX(MMP_MAX(a,b),c)
#define MMP_MIN(a,b) ((a) > (b) ? (b) : (a))
#define MMP_MIN3(a,b,c) MMP_MIN(MMP_MIN(a,b),c)


#define MMP_SWAP_I16(v) (short)((((v)>>8)  & 0xff) | (((v)&0xff) << 8))
#define MMP_SWAP_I32(v) (int)((((v)>>24) & 0xff) | (((v)>>8) & 0xff00) | (((v)&0xff00) << 8) | (((v)&0xff) << 24))
#define MMP_SWAP_U16(v) (unsigned short)((((v)>>8)  & 0xff) | (((v)&0xff) << 8))
#define MMP_SWAP_U32(v) (unsigned int)((((v)>>24) & 0xff) | (((v)>>8) & 0xff00) | (((v)&0xff00) << 8) | (((v)&0xff) << 24))

#define MMP_PUT_BYTE(_p, _b) \
    *_p++ = (unsigned char)_b; 

#define MMP_PUT_BUFFER(_p, _buf, _len) \
    memcpy(_p, _buf, _len); \
    _p += _len;

#define MMP_PUT_LE32(_p, _var) \
    *_p++ = (unsigned char)((_var)>>0);  \
    *_p++ = (unsigned char)((_var)>>8);  \
    *_p++ = (unsigned char)((_var)>>16); \
    *_p++ = (unsigned char)((_var)>>24); 

#define MMP_PUT_BE32(_p, _var) \
    *_p++ = (unsigned char)((_var)>>24);  \
    *_p++ = (unsigned char)((_var)>>16);  \
    *_p++ = (unsigned char)((_var)>>8); \
    *_p++ = (unsigned char)((_var)>>0); 


#define MMP_PUT_LE16(_p, _var) \
    *_p++ = (unsigned char)((_var)>>0);  \
    *_p++ = (unsigned char)((_var)>>8);  


#define MMP_PUT_BE16(_p, _var) \
    *_p++ = (unsigned char)((_var)>>8);  \
    *_p++ = (unsigned char)((_var)>>0);  


#define MMP_RB32(x)                                 \
    (((MMP_U32)((const MMP_U8*)(x))[0] << 24) |     \
               (((const MMP_U8*)(x))[1] << 16) |    \
               (((const MMP_U8*)(x))[2] <<  8) |    \
                ((const MMP_U8*)(x))[3])


#define MMP_MKTAG(a,b,c,d) ((a) | ((b) << 8) | ((c) << 16) | ((unsigned)(d) << 24))
#define MMP_MKBETAG(a,b,c,d) ((d) | ((c) << 8) | ((b) << 16) | ((unsigned)(a) << 24))


#define MMPMAKEFOURCC(ch0, ch1, ch2, ch3)                           \
                ((MMP_U32)(MMP_U8)(ch0) | ((MMP_U32)(MMP_U8)(ch1) << 8) |   \
                ((MMP_U32)(MMP_U8)(ch2) << 16) | ((MMP_U32)(MMP_U8)(ch3) << 24 ))
#define MMPGETFOURCC(fourcc,ch) (MMP_U8)(fourcc>>(8*ch))
#define MMPGETFOURCCARG(fourcc)  MMPGETFOURCC(fourcc,0),MMPGETFOURCC(fourcc,1),MMPGETFOURCC(fourcc,2),MMPGETFOURCC(fourcc,3)
#define MMP_BYTE_ALIGN(x, align)   (((x) + (align) - 1) & ~((align) - 1))
#define MMP_BYTE_ALIGN_16B(x)   ((((x) + (1 <<  4) - 1) >>  4) <<  4)
#define MMP_BYTE_ALIGN_32B(x)   ((((x) + (1 <<  5) - 1) >>  5) <<  5)
#define MMP_BYTE_ALIGN_128B(x)  ((((x) + (1 <<  7) - 1) >>  7) <<  7)
#define MMP_BYTE_ALIGN_8KB(x)   ((((x) + (1 << 13) - 1) >> 13) << 13)

#if (MMP_OS==MMP_OS_WIN32 || MMP_OS==MMP_OS_WINCE)
#define MMP_SLEEP(mileSec) Sleep(mileSec)
#elif (MMP_OS==MMP_OS_NUCLEUS)
#define MMP_SLEEP(mileSec) OAL_TaskSleep(mileSec*1000);
#elif (MMP_OS==MMP_OS_LINUX)
#define MMP_SLEEP(mileSec) usleep(mileSec*1000);
#else
#error "ERROR: Define MMP_SLEEP"
#endif

#include "MmpDefine_Fourcc.h"
#include "MmpDefine_Audio.h"
#include "MmpDefine_Video.h"
#include "MmpDefine_OMX.h"

#define MMP_FILENAME_MAX_LEN 256

/* In, Out */
#define MMP_IN
#define MMP_OUT
#define MMP_INOUT

/* malloc/free/memset/memcpy */
#define MMP_MALLOC(x) malloc(x)
#define MMP_FREE(x) free(x)
#define MMP_MEMSET(ptr, v, sz) memset(ptr, v, sz)
#define MMP_MEMCPY(dst, src, sz) memcpy(dst, src, sz)

/* Byte Size Define */
#define MMP_BYTE_SIZE_1M   0x00100000 /* 1024*1024, 1048576 */
#define MMP_BYTE_SIZE_1K   0x400      /* 1*1024  0x400 */
#define MMP_BYTE_SIZE_4K   0x1000     /* 4*1024 = 4096     0x1000 */
#define MMP_BYTE_SIZE_8K   0x2000     /* 8*1024  = 8192    0x2000 */
#define MMP_BYTE_SIZE_16K  0x4000     /* 16*1024 = 16384   0x4000 */
#define MMP_BYTE_SIZE_32K  0x8000     /* 32*1024 = 32768   0x8000 */
#define MMP_BYTE_SIZE_64K  0x10000    /* 64*1024 = 65536   0x10000 */
#define MMP_BYTE_SIZE_128K 0x20000    /* 128*1024 = 131072 0x20000 */
#define MMP_BYTE_SIZE_256K 0x40000    /* 256*1024 = 262144 0x40000 */




enum MMP_PLAY_FORMAT
{
        MMP_PLAY_BACK,
        MMP_PLAY_FF,
        MMP_PLAY_REW,
        MMP_PLAY_RAND,
 };


/*****************************************************************************
MME Command 
*****************************************************************************/

struct driver_mme_io {
    unsigned int parm[16];
};

#define MME_DRIVER_NODE "/dev/mme"

/* crypto */
#define MME_IOC_CRYPTO_API_TEST_HW	_IOWR('M',  0, struct driver_mme_io)
#define MME_IOC_CRYPTO_API_TEST_SW	_IOWR('V',  8, struct driver_mme_io)

/* System Command */
#define MME_IOC_SYSTEM_GET_JIFFIES   _IOWR('S',  0x100, struct driver_mme_io)
#define MME_IOC_SYSTEM_PRINT_CLOCK   _IOWR('S',  0x101, struct driver_mme_io)
#define MME_IOC_SYSTEM_FORCE_CPU_STALL   _IOWR('S',  0x102, struct driver_mme_io) /* 강제로 CPU를 Stall 에 들어가게 한다. */
#define MME_IOC_SYSTEM_POWER_ON      _IOWR('S',  0x103, struct driver_mme_io) 
#define MME_IOC_SYSTEM_POWER_OFF     _IOWR('S',  0x104, struct driver_mme_io) 
#define MME_IOC_SYSTEM_CLOCK_ENABLE  _IOWR('S',  0x105, struct driver_mme_io) 
#define MME_IOC_SYSTEM_CLOCK_DISABLE _IOWR('S',  0x106, struct driver_mme_io) 
#define MME_IOC_SYSTEM_TEST_EXCEPTION _IOWR('S', 0x107, struct driver_mme_io) 
#define MME_IOC_SYSTEM_TEST_CLK       _IOWR('S', 0x108, struct driver_mme_io) 

/*****************************************************************************/


/*****************************************************************************
HW Codec
*****************************************************************************/

#if (MMP_SOC == MMP_SOC_WIN32)
#define MMP_HW_CODEC_VIDEO_VPU_ENABLE 1
#elif (MMP_SOC == MMP_SOC_GDM7243V)
#define MMP_HW_CODEC_VIDEO_VPU_ENABLE 1
#else
#define MMP_HW_CODEC_VIDEO_VPU_ENABLE 0
#endif


/*****************************************************************************
FFMpeg Version
*****************************************************************************/
#define FFMPEG_VERSION_2_6_1 0x020601   /* 2.6.1*/
#define FFMPEG_VERSION_n3_1_1 0x030101  /* n3.1.1*/
#define FFMPEG_DISABLE  0

#ifdef WIN32
#define FFMPEG_VERSION  FFMPEG_VERSION_n3_1_1
//#define FFMPEG_VERSION  FFMPEG_DISABLE

#else
#if (MMP_TARGET_FFMPEG == FFMPEG_DISABLE)
#define FFMPEG_VERSION  FFMPEG_DISABLE

#else
#define FFMPEG_VERSION  FFMPEG_VERSION_n3_1_1

#endif
#endif

#endif // #ifndef __MMPGLOBAL_H__

