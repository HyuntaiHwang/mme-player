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

#ifndef MMPMUXER_FFMPEGEX1_HPP__
#define MMPMUXER_FFMPEGEX1_HPP__

#include "MmpMuxer.hpp"


#if (FFMPEG_VERSION == FFMPEG_VERSION_n3_1_1)

#include "ffmpeg_c_api.h"

class CMmpMuxer_FfmpegEx1 : public CMmpMuxer
{
friend class CMmpMuxer;

private:
    int64_t m_iFileSize;

    AVFormatContext *m_oc;
    AVOutputFormat *m_fmt;
    
    enum AVCodecID m_AVCodecID_Video;
    AVStream *m_audio_st, *m_video_st;
    AVCodec *m_audio_codec, *m_video_codec;

    MMP_U8 *m_extra_data;
    MMP_S32 m_extra_data_size;
    MMP_BOOL m_is_dsi_init[MMP_MEDIATYPE_MAX];

protected:
    CMmpMuxer_FfmpegEx1(struct MmpMuxerCreateConfig* pCreateConfig);
    virtual ~CMmpMuxer_FfmpegEx1();

    virtual MMP_RESULT Open();
    virtual MMP_RESULT Close();

private:
    AVStream* add_stream(AVFormatContext *oc, AVCodec **codec, enum AVCodecID codec_id, enum MMP_MEDIATYPE mediatype);
    
public:
    virtual MMP_RESULT AddMediaConfig(MMP_U32 mediatype, MMP_U8* buffer, MMP_U32 buf_size);
    virtual MMP_RESULT AddMediaData(MMP_U32 mediatype, MMP_U8* buffer, MMP_U32 buf_size, MMP_U32 flag, MMP_S64 pts);
    
};


#endif //#ifndef _MMPDEMUXER_HPP__
#endif