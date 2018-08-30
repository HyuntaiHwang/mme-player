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

#include "mmp_video_tool.hpp"



MMP_RESULT mmp_video_tool::make_video_packet_header(MMP_S32 pic_width, MMP_S32 pic_height, MMP_S32 fps, MMP_S32 bitrate, 
                                               void* dsi_data, MMP_S32 dsi_size, 
                                               void* ffmpeg_hdr, MMP_S32 ffmpeg_hdr_size,
                                               MMP_ADDR dst_buffer) {

    struct mmp_video_packet_header* p_video_pkt_hdr = (struct mmp_video_packet_header*)dst_buffer;
    MMP_ADDR data;

    /* set hdr */
    p_video_pkt_hdr->key = MMP_VIDEO_PACKET_HEADER_KEY;
    p_video_pkt_hdr->packet_hdr_size = sizeof(struct mmp_video_packet_header);
    
    /* set prop */
    p_video_pkt_hdr->pic_width = pic_width;
    p_video_pkt_hdr->pic_height = pic_height;
    p_video_pkt_hdr->fps = fps;
    p_video_pkt_hdr->bitrate = bitrate;

    /* set dsi */
    p_video_pkt_hdr->dsi_size = dsi_size;
    p_video_pkt_hdr->dsi_pos = MMP_BYTE_ALIGN_16B(p_video_pkt_hdr->packet_hdr_size);
    data = dst_buffer + p_video_pkt_hdr->dsi_pos;
    if(p_video_pkt_hdr->dsi_size > 0) {
        memcpy((void*)data, dsi_data, p_video_pkt_hdr->dsi_size);
    }


    /* set ffmpeg hdr */
    p_video_pkt_hdr->ffmpeg_hdr_size = ffmpeg_hdr_size;
    p_video_pkt_hdr->ffmpeg_hdr_pos = MMP_BYTE_ALIGN_16B(p_video_pkt_hdr->dsi_pos + p_video_pkt_hdr->dsi_size);
    data = dst_buffer + p_video_pkt_hdr->ffmpeg_hdr_pos;
    if(p_video_pkt_hdr->ffmpeg_hdr_size > 0) {
        memcpy((void*)data, ffmpeg_hdr, p_video_pkt_hdr->ffmpeg_hdr_size);
    }

    /* set pkt size */
    p_video_pkt_hdr->packet_size =  p_video_pkt_hdr->ffmpeg_hdr_pos + p_video_pkt_hdr->ffmpeg_hdr_size;
    
    return MMP_SUCCESS;
}

MMP_BOOL mmp_video_tool::is_valid_video_packet_header(MMP_ADDR pkt_addr, MMP_S32 buf_size) {

    MMP_BOOL is_valid = MMP_TRUE;
    struct mmp_video_packet_header video_pkt_hdr;
    
    /* check buf size */
    if( buf_size >= (int)sizeof(struct mmp_video_packet_header)) {
        memcpy(&video_pkt_hdr, (void*)pkt_addr, sizeof(struct mmp_video_packet_header));
    }
    else {
        is_valid = MMP_FALSE;
    }

    /* check key */
    if( (is_valid == MMP_TRUE) && (video_pkt_hdr.key != MMP_VIDEO_PACKET_HEADER_KEY) ) {
        is_valid = MMP_FALSE;
    }

    /* check pkt hdr size */
    if( (is_valid == MMP_TRUE) && (video_pkt_hdr.packet_hdr_size != sizeof(struct mmp_video_packet_header) ) ) {
        is_valid = MMP_FALSE;
    }

    /* check pkt size */
    if( (is_valid == MMP_TRUE) && (video_pkt_hdr.packet_size !=  (video_pkt_hdr.ffmpeg_hdr_pos + video_pkt_hdr.ffmpeg_hdr_size) ) ) {
        is_valid = MMP_FALSE;
    }

    return is_valid;
}


const MMP_U8 * mmp_video_tool::find_start_code(const MMP_U8 *p, const MMP_U8 *end,  MMP_U32 *state)
{
    int i;

    //av_assert0(p <= end);
    if (p >= end)
        return end;

    for (i = 0; i < 3; i++) {
        MMP_U32 tmp = *state << 8;
        *state = tmp + *(p++);
        if (tmp == 0x100 || p == end)
            return p;
    }

    while (p < end) {
        if      (p[-1] > 1      ) p += 3;
        else if (p[-2]          ) p += 2;
        else if (p[-3]|(p[-1]-1)) p++;
        else {
            p++;
            break;
        }
    }

    p = MMP_MIN(p, end) - 4;
    *state = MMP_RB32(p);

    return p + 4;
}
