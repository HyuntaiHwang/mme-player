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

#ifndef MMPDECODERVIDEO_V4L2_MFC_HPP__
#define MMPDECODERVIDEO_V4L2_MFC_HPP__

#include "MmpDecoderVideo.hpp"
#include "mmp_buffer_videoframe.hpp"
#include "v4l2_video_mfc_api.h"
#include "TemplateList.hpp"

class CMmpDecoderVideo_V4L2_MFC : public CMmpDecoderVideo
{
friend class CMmpDecoderVideo;

private:
    enum{
     MAX_BUF_COUNT_SRC=4,
     MAX_BUF_COUNT_DST=32,
     MAX_PLANE_COUNT=3
    };

private:
    FILE* m_fp_dump;

    MMP_S32 m_input_stream_count;
    MMP_S32 m_decoded_picture_count;

    MMP_S32 m_pic_width;
    MMP_S32 m_pic_height;

    MMP_BOOL m_is_dsi;
    MMP_S32 m_v4l2_fd;
    MMP_U32 m_v4l2_fourcc_src;
    MMP_U32 m_v4l2_fourcc_dst;
    MMP_S32 m_framesize_src;
    MMP_S32 m_framesize_dst;
    
    MMP_S32 m_buf_src_real_count;
    MMP_S32 m_buf_dst_real_count;

    struct v4l2_buffer m_buf_src[MAX_BUF_COUNT_SRC];
    struct v4l2_buffer m_buf_dst[MAX_BUF_COUNT_DST];

    struct v4l2_plane m_planes_src[MAX_BUF_COUNT_SRC][MAX_PLANE_COUNT];
    struct v4l2_plane m_planes_dst[MAX_BUF_COUNT_DST][MAX_PLANE_COUNT];

    unsigned char* m_p_src_buf[MAX_BUF_COUNT_SRC];
    unsigned char* m_p_dst_buf[MAX_BUF_COUNT_DST][MAX_PLANE_COUNT];
    
    MMP_BOOL m_is_deque_src[MAX_BUF_COUNT_SRC];
    
    int m_last_src_dq_index;
    int m_last_dst_dq_index;

    TMinHeap<MMP_S64> m_minheap_pts;

protected:
    CMmpDecoderVideo_V4L2_MFC(struct CMmpDecoderVideo::create_config *p_create_config);
    virtual ~CMmpDecoderVideo_V4L2_MFC();

    virtual MMP_RESULT Open();
    virtual MMP_RESULT Close();

private:
    virtual MMP_RESULT DecodeDSI(class mmp_buffer_videostream* p_buf_videostream, MMP_BOOL *is_reconfig);

public:
    virtual MMP_RESULT DecodeAu(class mmp_buffer_videostream* p_buf_videostream, class mmp_buffer_videoframe** pp_buf_videoframe, MMP_BOOL *is_reconfig);
    
};

#endif

