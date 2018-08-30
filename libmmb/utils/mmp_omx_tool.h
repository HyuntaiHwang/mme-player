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

#ifndef MMP_OMX_TOOL_H__
#define MMP_OMX_TOOL_H__

#include "MmpDefine.h"

#ifdef __cplusplus
extern "C" {
#endif

/* OMX Component Supprot */
MMP_S32 mmp_get_omx_component_count(void);
const MMP_CHAR* mmp_get_omx_component_name(MMP_S32 idx);
const MMP_CHAR* mmp_get_omx_component_mime(MMP_S32 idx);
enum MMP_MEDIATYPE mmp_get_omx_component_get_media_type(MMP_S32 idx);
MMP_BOOL mmp_get_omx_component_is_encoder(MMP_S32 idx);
enum MMP_FOURCC mmp_get_omx_component_get_fourcc(MMP_S32 idx);
MMP_U32 mmp_get_omx_component_omx_coding_type(MMP_S32 idx);

/* OMX Format Support */
OMX_COLOR_FORMATTYPE mmp_get_omxformat_from_fourcc(enum MMP_FOURCC fourcc);
enum MMP_FOURCC mmp_get_fourcc_from_omxformat(OMX_COLOR_FORMATTYPE omxfmt);

/* Video Decoder Support Profile */
OMX_ERRORTYPE mmp_get_omx_vidoe_decoder_prifle_level(enum MMP_FOURCC fourcc, OMX_VIDEO_PARAM_PROFILELEVELTYPE *profileLevel);

#ifdef __cplusplus
}
#endif


#endif
