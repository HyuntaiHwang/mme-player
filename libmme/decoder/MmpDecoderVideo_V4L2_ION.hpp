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

#ifndef MMPDECODERVIDEO_V4L2_ION_HPP__
#define MMPDECODERVIDEO_V4L2_ION_HPP__

#include "MmpDecoderVideo.hpp"
#include "mmp_buffer_videoframe.hpp"
#include "v4l2_video_mfc_api.h"

class CMmpDecoderVideo_V4L2_ION : public CMmpDecoderVideo
{
friend class CMmpDecoderVideo;

private:
    enum{
     MAX_BUF_COUNT_SRC=1,
     MAX_BUF_COUNT_DST=6,
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
    
    
    int m_last_dst_dq_index;
    class mmp_buffer_videoframe* m_p_buf_vf_mfc[DECODED_BUF_MAX_COUNT];

protected:
    CMmpDecoderVideo_V4L2_ION(struct mmp_decoder_video_create_config *p_create_config);
    virtual ~CMmpDecoderVideo_V4L2_ION();

    virtual MMP_RESULT Open();
    virtual MMP_RESULT Close();

private:
    virtual MMP_RESULT DecodeDSI(class mmp_buffer_videostream* p_buf_videostream, MMP_BOOL *is_reconfig);

public:
    virtual MMP_BOOL is_support_this_fourcc_in(enum MMP_FOURCC fourcc);
    virtual MMP_BOOL is_support_this_fourcc_out(enum MMP_FOURCC fourcc);

    virtual MMP_RESULT DecodeAu(class mmp_buffer_videostream* p_buf_videostream, class mmp_buffer_videoframe** pp_buf_videoframe, MMP_BOOL *is_reconfig);
    
};

#endif

