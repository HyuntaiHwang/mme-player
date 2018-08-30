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

#ifndef MMPDEFINE_TYPE_H__
#define MMPDEFINE_TYPE_H__

#define MMP_LONGLONG_BIT    64
#define MMP_LONGLONG_MIN    (-9223372036854775807LL
#define MMP_LONGLONG_MAX    9223372036854775807LL
#define MMP_ULONGLONG_MAX   18446744073709551615ULL

#define	MMP_UINT_MAX	0xffffffffU	/* max value for an unsigned int */
#define	MMP_INT_MAX		0x7fffffff	/* max value for an int */
#define	MMP_INT_MIN		(-0x7fffffff-1)	/* min value for an int */

#define	MMP_USHRT_MAX	0xffffU		/* max value for an unsigned short */
#define	MMP_SHRT_MAX	0x7fff		/* max value for a short */
#define MMP_SHRT_MIN    (-0x7fff-1)     /* min value for a short */


/** MMP_U8 is an 8 bit unsigned quantity that is byte aligned */
typedef unsigned char MMP_U8;

/** MMP_S8 is an 8 bit signed quantity that is byte aligned */
typedef signed char MMP_S8;

/** MMP_U16 is a 16 bit unsigned quantity that is 16 bit word aligned */
typedef unsigned short MMP_U16;

/** MMP_S16 is a 16 bit signed quantity that is 16 bit word aligned */
typedef signed short MMP_S16;

/** MMP_U32 is a 32 bit unsigned quantity that is 32 bit word aligned */
typedef unsigned long MMP_U32;

/** MMP_S32 is a 32 bit signed quantity that is 32 bit word aligned */
typedef signed long MMP_S32;


#ifdef WIN32
/** OMX_U64 is a 64 bit unsigned quantity that is 64 bit word aligned */   
typedef unsigned __int64  MMP_U64;

/** OMX_S64 is a 64 bit signed quantity that is 64 bit word aligned */
typedef signed   __int64  MMP_S64;

#else

/** OMX_U64 is a 64 bit unsigned quantity that is 64 bit word aligned */
typedef unsigned long long MMP_U64;

/** OMX_S64 is a 64 bit signed quantity that is 64 bit word aligned */
typedef signed long long MMP_S64;

#endif

typedef MMP_S64   MMP_TICKS;
typedef MMP_U32   MMP_ADDR;
typedef MMP_U32   MMP_MEDIA_ID;
#define MEDIA_ID_UNKNOWN  0xFFFFFFFF

#define MMP_ADDR_NULL 0

typedef int MMP_BOOL;
#define MMP_TRUE  1
#define MMP_FALSE 0

typedef void* MMP_PTR;
typedef char* MMP_STRING;
typedef unsigned char* MMP_BYTE;
typedef char MMP_CHAR;

/** The MMP_ERRORTYPE enumeration defines the standard OMX Errors.  These 
 *  errors should cover most of the common failure cases.  However, 
 *  vendors are free to add additional error messages of their own as 
 *  long as they follow these rules:
 *  1.  Vendor error messages shall be in the range of 0x90000000 to
 *      0x9000FFFF.
 *  2.  Vendor error messages shall be defined in a header file provided
 *      with the component.  No error messages are allowed that are
 *      not defined.
 */
typedef enum MMP_ERRORTYPE
{
  MMP_ErrorNone = 0,

  /** There were insufficient resources to perform the requested operation */
  MMP_ErrorInsufficientResources = (MMP_S32) 0x80001000,

  /** There was an error, but the cause of the error could not be determined */
  MMP_ErrorUndefined = (MMP_S32) 0x80001001,

  /** The component name string was not valid */
  MMP_ErrorInvalidComponentName = (MMP_S32) 0x80001002,

  /** No component with the specified name string was found */
  MMP_ErrorComponentNotFound = (MMP_S32) 0x80001003,

  /** The component specified did not have a "MMP_ComponentInit" or
      "MMP_ComponentDeInit entry point */
  MMP_ErrorInvalidComponent = (MMP_S32) 0x80001004,

  /** One or more parameters were not valid */
  MMP_ErrorBadParameter = (MMP_S32) 0x80001005,

  /** The requested function is not implemented */
  MMP_ErrorNotImplemented = (MMP_S32) 0x80001006,

  /** The buffer was emptied before the next buffer was ready */
  MMP_ErrorUnderflow = (MMP_S32) 0x80001007,

  /** The buffer was not available when it was needed */
  MMP_ErrorOverflow = (MMP_S32) 0x80001008,

  /** The hardware failed to respond as expected */
  MMP_ErrorHardware = (MMP_S32) 0x80001009,

  /** The component is in the state MMP_StateInvalid */
  MMP_ErrorInvalidState = (MMP_S32) 0x8000100A,

  /** Stream is found to be corrupt */
  MMP_ErrorStreamCorrupt = (MMP_S32) 0x8000100B,

  /** Ports being connected are not compatible */
  MMP_ErrorPortsNotCompatible = (MMP_S32) 0x8000100C,

  /** Resources allocated to an idle component have been
      lost resulting in the component returning to the loaded state */
  MMP_ErrorResourcesLost = (MMP_S32) 0x8000100D,

  /** No more indicies can be enumerated */
  MMP_ErrorNoMore = (MMP_S32) 0x8000100E,

  /** The component detected a version mismatch */
  MMP_ErrorVersionMismatch = (MMP_S32) 0x8000100F,

  /** The component is not ready to return data at this time */
  MMP_ErrorNotReady = (MMP_S32) 0x80001010,

  /** There was a timeout that occurred */
  MMP_ErrorTimeout = (MMP_S32) 0x80001011,

  /** This error occurs when trying to transition into the state you are already in */
  MMP_ErrorSameState = (MMP_S32) 0x80001012,

  /** Resources allocated to an executing or paused component have been 
      preempted, causing the component to return to the idle state */
  MMP_ErrorResourcesPreempted = (MMP_S32) 0x80001013, 

  /** A non-supplier port sends this error to the IL client (via the EventHandler callback) 
      during the allocation of buffers (on a transition from the LOADED to the IDLE state or
      on a port restart) when it deems that it has waited an unusually long time for the supplier 
      to send it an allocated buffer via a UseBuffer call. */
  MMP_ErrorPortUnresponsiveDuringAllocation = (MMP_S32) 0x80001014,

  /** A non-supplier port sends this error to the IL client (via the EventHandler callback) 
      during the deallocation of buffers (on a transition from the IDLE to LOADED state or 
      on a port stop) when it deems that it has waited an unusually long time for the supplier 
      to request the deallocation of a buffer header via a FreeBuffer call. */
  MMP_ErrorPortUnresponsiveDuringDeallocation = (MMP_S32) 0x80001015,

  /** A supplier port sends this error to the IL client (via the EventHandler callback) 
      during the stopping of a port (either on a transition from the IDLE to LOADED 
      state or a port stop) when it deems that it has waited an unusually long time for 
      the non-supplier to return a buffer via an EmptyThisBuffer or FillThisBuffer call. */
  MMP_ErrorPortUnresponsiveDuringStop = (MMP_S32) 0x80001016,

  /** Attempting a state transtion that is not allowed */
  MMP_ErrorIncorrectStateTransition = (MMP_S32) 0x80001017,

  /* Attempting a command that is not allowed during the present state. */
  MMP_ErrorIncorrectStateOperation = (MMP_S32) 0x80001018, 

  /** The values encapsulated in the parameter or config structure are not supported. */
  MMP_ErrorUnsupportedSetting = (MMP_S32) 0x80001019,

  /** The parameter or config indicated by the given index is not supported. */
  MMP_ErrorUnsupportedIndex = (MMP_S32) 0x8000101A,

  /** The port index supplied is incorrect. */
  MMP_ErrorBadPortIndex = (MMP_S32) 0x8000101B,

  /** The port has lost one or more of its buffers and it thus unpopulated. */
  MMP_ErrorPortUnpopulated = (MMP_S32) 0x8000101C,

  /** Component suspended due to temporary loss of resources */
  MMP_ErrorComponentSuspended = (MMP_S32) 0x8000101D,

  /** Component suspended due to an inability to acquire dynamic resources */
  MMP_ErrorDynamicResourcesUnavailable = (MMP_S32) 0x8000101E,

  /** When the macroblock error reporting is enabled the component returns new error 
  for every frame that has errors */
  MMP_ErrorMbErrorsInFrame = (MMP_S32) 0x8000101F,

  /** A component reports this error when it cannot parse or determine the format of an input stream. */
  MMP_ErrorFormatNotDetected = (MMP_S32) 0x80001020, 

  /** The content open operation failed. */
  MMP_ErrorContentPipeOpenFailed = (MMP_S32) 0x80001021,

  /** The content creation operation failed. */
  MMP_ErrorContentPipeCreationFailed = (MMP_S32) 0x80001022,

  /** Separate table information is being used */
  MMP_ErrorSeperateTablesUsed = (MMP_S32) 0x80001023,

  /** Tunneling is unsupported by the component*/
  MMP_ErrorTunnelingUnsupported = (MMP_S32) 0x80001024,

  MMP_ErrorKhronosExtensions = (MMP_S32)0x8F000000, /**< Reserved region for introducing Khronos Standard Extensions */ 
  MMP_ErrorVendorStartUnused = (MMP_S32)0x90000000, /**< Reserved region for introducing Vendor Extensions */


  MMP_ErrorVendorAlreadyExistObject = (MMP_S32)0x90000001, 
  MMP_ErrorVendorNullPointer  = (MMP_S32)0x90000002, 

  MMP_ErrorMax = 0x7FFFFFFF

}MMP_RESULT;

//typedef int MMP_RESULT;
#define MMP_SUCCESS                     MMP_ErrorNone
#define MMP_FAILURE                     MMP_ErrorUndefined
#define MMP_ERROR_MEMORY_ALLOC          MMP_ErrorInsufficientResources
#define MMP_ERROR_ALREADY_EXIST_OBJECT  MMP_ErrorVendorAlreadyExistObject
#define MMP_ERROR_NULL_POINTER          MMP_ErrorVendorNullPointer
#define MMP_ERROR_INVALID_ARGS          MMP_ErrorBadParameter
#define MMP_ERROR_NOT_SUPPORT           MMP_ErrorUnsupportedSetting

struct mmp_date_and_time
{
    MMP_U16 year;
    MMP_U8 month;
    MMP_U8 day_of_month;
    MMP_U8 day_of_week; /* Sun(0) Mon(1) Tue(2) Wed(3) Thr(4) Fri(5) Sat(7) */
    MMP_U8 hour; 
    MMP_U8 minute; 
    MMP_U8 second; 
    MMP_U16 milesec; 
};

/* /proc/meminfo */
struct mmp_system_meminfo
{
    MMP_U32 MemTotal; //:        1695232 kB
    MMP_U32 MemFree; //:         1138580 kB
    MMP_U32 Buffers; //:            8148 kB
    MMP_U32 Cached; //:           334320 kB
    MMP_U32 SwapCached; //:            0 kB
    MMP_U32 Active; //:           176472 kB
    MMP_U32 Inactive; //:         291496 kB
    MMP_U32 Active_anon; //:     125520 kB
    MMP_U32 Inactive_anon; //:      204 kB
    MMP_U32 Active_file; //:      50952 kB
    MMP_U32 Inactive_file; //:   291292 kB
    MMP_U32 Unevictable; //:           0 kB
    MMP_U32 Mlocked; //:               0 kB
    MMP_U32 HighTotal; //:       1342464 kB
    MMP_U32 HighFree; //:         856732 kB
    MMP_U32 LowTotal; //:         352768 kB
    MMP_U32 LowFree; //:          281848 kB
    MMP_U32 SwapTotal; //:             0 kB
    MMP_U32 SwapFree; //:              0 kB
    MMP_U32 Dirty; //:                 0 kB
    MMP_U32 Writeback; //:             0 kB
    MMP_U32 AnonPages; //:        125488 kB
    MMP_U32 Mapped; //:            59168 kB
    MMP_U32 Shmem; //:               236 kB
    MMP_U32 Slab; //:              14264 kB
    MMP_U32 SReclaimable; //:       5856 kB
    MMP_U32 SUnreclaim; //:         8408 kB
    MMP_U32 KernelStack; //:        3768 kB
    MMP_U32 PageTables; //:         5036 kB
    MMP_U32 NFS_Unstable; //:          0 kB
    MMP_U32 Bounce; //:                0 kB
    MMP_U32 WritebackTmp; //:          0 kB
    MMP_U32 CommitLimit; //:      847616 kB
    MMP_U32 Committed_AS; //:    3075032 kB
    MMP_U32 VmallocTotal; //:     122880 kB
    MMP_U32 VmallocUsed; //:       45312 kB
    MMP_U32 VmallocChunk; //:      30596 kB
};

/* /proc/stat cpu usage*/
struct mmp_system_cpu_usage {

    MMP_S32 cpu_count;
    
    MMP_U32 percent[4]; 
    
    MMP_U32 jiffies_user[4];
    MMP_U32 jiffies_nice_user[4];
    MMP_U32 jiffies_system[4];
    MMP_U32 jiffies_idle[4];
};

/*rect/size/point */
struct mmp_point {
    MMP_S32 x;
    MMP_S32 y;
};

struct mmp_size {
    MMP_S32 cx;
    MMP_S32 cy;
};

struct mmp_rect {
    MMP_S32 left;
    MMP_S32 right;
    MMP_S32 top;
    MMP_S32 bottom;
};

enum MMP_BUFTYPE {
    MMP_BUFTYPE_SW=0,
    MMP_BUFTYPE_HW
};

enum MMP_MEDIATYPE {
    MMP_MEDIATYPE_UNKNOWN=-1,
    MMP_MEDIATYPE_AUDIO=0,
    MMP_MEDIATYPE_VIDEO,
    MMP_MEDIATYPE_IMAGE,
    MMP_MEDIATYPE_MAX
};


#endif