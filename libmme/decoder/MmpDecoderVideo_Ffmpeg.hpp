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

#ifndef MMPDECODERVIDEO_FFMPEG_HPP__
#define MMPDECODERVIDEO_FFMPEG_HPP__

#include "MmpDecoderVideo.hpp"
#include "MmpDecoderFfmpeg.hpp"
#include "mmp_buffer_videoframe.hpp"

#if (FFMPEG_VERSION == FFMPEG_VERSION_2_6_1)

#define MMPDECODERVIDEO_H264_AVC_CHECK 0
#define MMPDECODERVIDEO_DIRECT_RENDER 0
#define MMPDECODERVIDEO_DUMMY_MODE_ENABLE 0


class CMmpDecoderVideo_Ffmpeg : public CMmpDecoderVideo, CMmpDecoderFfmpeg
{
friend class CMmpDecoderVideo;
private:
    enum {
        DECODED_BUF_ACTUAL_COUNT=4
    };

private:
    MMP_S32 m_decoded_picture_count;
    MMP_U8 m_stream_buf[1024*1024];
    MMP_BOOL m_enable_avcopy;

    TMinHeap<MMP_S64> m_minheap_ts;
        
    
protected:
    CMmpDecoderVideo_Ffmpeg(struct CMmpDecoderVideo::create_config *p_create_config);
    virtual ~CMmpDecoderVideo_Ffmpeg();

    virtual MMP_RESULT Open();
    virtual MMP_RESULT Close();

    virtual int ffmpeg_get_buffer_cb(struct AVCodecContext *p_context,  AVFrame *p_ff_pic);
    virtual void ffmpeg_release_buffer_cb(struct AVCodecContext *p_context,  AVFrame *p_ff_pic);

private:
    virtual MMP_RESULT DecodeDSI(class mmp_buffer_videostream* p_buf_videostream);

public:
    virtual MMP_RESULT flush_buffer_in();

    virtual MMP_RESULT DecodeAu(class mmp_buffer_videostream* p_buf_videostream, class mmp_buffer_videoframe** pp_buf_videoframe, enum ACTION *p_action);
    virtual MMP_RESULT DecodeAu(class mmp_buffer_videostream* p_buf_vs, class mmp_buffer_videoframe* p_buf_vf);
};

#endif
#endif

