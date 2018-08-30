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

#ifndef MMP_VIDEO_TOOL_H__
#define MMP_VIDEO_TOOL_H__

#include "MmpDefine.h"

class mmp_video_tool {

public:
    
    static MMP_RESULT make_video_packet_header(MMP_S32 pic_width, MMP_S32 pic_height, MMP_S32 fps, MMP_S32 bitrate, 
                                               void* dsi_data, MMP_S32 dsi_size, 
                                               void* ffmpeg_hdr, MMP_S32 ffmpeg_hdr_size,
                                               MMP_ADDR dst_buffer);
    static MMP_BOOL is_valid_video_packet_header(MMP_ADDR pkt_addr, MMP_S32 buf_size);

    static const MMP_U8 * find_start_code(const MMP_U8 *p, const MMP_U8 *end,   MMP_U32 *state);
};

#endif
