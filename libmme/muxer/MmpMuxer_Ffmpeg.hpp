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

#ifndef MMPMUXER_FFMPEG_HPP__
#define MMPMUXER_FFMPEG_HPP__

#include "MmpMuxer.hpp"

#if (FFMPEG_VERSION == FFMPEG_VERSION_2_6_1)

#include "ffmpeg_c_api.h"

class CMmpMuxer_Ffmpeg : public CMmpMuxer
{
friend class CMmpMuxer;

private:
    FILE* m_fp;
    int64_t m_iFileSize;

    AVFormatContext *m_oc;
    AVOutputFormat *m_fmt;
    AVIOContext* m_pAVIOContext;
    MMP_U8* m_IoBuffer;
    URLContext* m_Url;
    
    enum AVCodecID m_AVCodecID_Video;
    AVStream *m_audio_st, *m_video_st;
    AVCodec *m_audio_codec, *m_video_codec;

    MMP_U8 *m_extra_data;
    MMP_S32 m_extra_data_size;
    MMP_BOOL m_is_dsi_init[MMP_MEDIATYPE_MAX];

protected:
    CMmpMuxer_Ffmpeg(struct MmpMuxerCreateConfig* pCreateConfig);
    virtual ~CMmpMuxer_Ffmpeg();

    virtual MMP_RESULT Open();
    virtual MMP_RESULT Close();

private:
    static int IORead_C_Stub(void *opaque, uint8_t *buf, int buf_size);
    static int IOWrite_C_Stub(void *opaque, uint8_t *buf, int buf_size);
    static int64_t IOSeek_C_Stub(void *opaque, int64_t offset, int whence);

    int IORead_C(void *opaque, uint8_t *buf, int buf_size);
    int IOWrite_C(void *opaque, uint8_t *buf, int buf_size);
    int64_t IOSeek_C(void *opaque, int64_t offset, int whence);

    AVStream* add_stream(AVFormatContext *oc, AVCodec **codec, enum AVCodecID codec_id, enum MMP_MEDIATYPE mediatype);
    
public:
    virtual MMP_RESULT AddMediaConfig(MMP_U32 mediatype, MMP_U8* buffer, MMP_U32 buf_size);
    virtual MMP_RESULT AddMediaData(MMP_U32 mediatype, MMP_U8* buffer, MMP_U32 buf_size, MMP_U32 flag, MMP_S64 pts);
    
};


#endif //#ifndef _MMPDEMUXER_HPP__
#endif