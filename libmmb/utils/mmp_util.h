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

#ifndef MMP_UTIL_H__
#define MMP_UTIL_H__

#include "MmpDefine.h"

#ifdef __cplusplus
extern "C" {
#endif


MMP_U32  exynos4412_clock_apll_cal(MMP_U32 APLL_CON0);

void kernel_io_system_print_clock();


/**************************************************************************************
  PCM Dump Tool
**************************************************************************************/
void* pcm_dump_write_create(MMP_CHAR* pcm_filename, MMP_S32 sr, MMP_S32 ch, MMP_S32 bps);
MMP_RESULT pcm_dump_write_data(void* hdl, MMP_U8* data, MMP_S32 datasz);

void* pcm_dump_read_create(MMP_CHAR* pcm_filename);
MMP_RESULT pcm_dump_read_data(void* hdl, MMP_U8* data, MMP_S32 datasz, MMP_S32* readsz);

MMP_RESULT pcm_dump_destroy(void* hdl);

MMP_S32 pcm_dump_get_sr(void* hdl); /* sample rate*/
MMP_S32 pcm_dump_get_ch(void* hdl); /* channel */
MMP_S32 pcm_dump_get_bps(void* hdl); /* bits per sample */
MMP_S32 pcm_dump_get_abps(void* hdl); /* avg byte per sec */
MMP_S64 pcm_dump_get_play_dur(void* hdl); /* play duration */


/**************************************************************************************
  YUV Dump Tool
**************************************************************************************/
void* yuv_dump_write_create(MMP_CHAR* name, MMP_S32 width, MMP_S32 height, enum MMP_FOURCC fourcc);
MMP_RESULT yuv_dump_destroy(void* hdl);
MMP_RESULT yuv_dump_write_data(void* hdl, MMP_U8* data, MMP_S32 datasz);
MMP_RESULT yuv_dump_write_YUV420M(void* hdl, 
                                  MMP_U8* Y, MMP_U8* U, MMP_U8* V, 
                                  MMP_S32 y_stride, MMP_S32 u_stride, MMP_S32 v_stride
                                  );

#ifdef __cplusplus
}
#endif

#endif

