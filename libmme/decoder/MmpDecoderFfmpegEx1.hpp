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

#ifndef _MMPDECODERFFMPEGEX1_HPP__
#define _MMPDECODERFFMPEGEX1_HPP__

#include "MmpDecoder.hpp"
#include "TemplateList.hpp"

#if (FFMPEG_VERSION == FFMPEG_VERSION_n3_1_1)

#include "ffmpeg_c_api.h"

struct audio_decoded_packet {
    MMP_U8* p_buffer;
    MMP_U32 buf_size;
    MMP_U32 buf_index;
    MMP_U64 timestamp;
};

class CMmpDecoderFfmpegEx1 
{
protected:
    enum MMP_FOURCC m_fourcc_in;
    AVCodecID m_AVCodecID;
    AVCodec *m_pAVCodec;
    AVCodecContext *m_pAVCodecContext;
    AVFrame *m_pAVFrame_Decoded;

    MMP_U8* m_extra_data;
    
    /* h264, avc */
    MMP_S32 m_is_avc;
    MMP_S32 m_nal_length_size;
    
protected:
    CMmpDecoderFfmpegEx1(enum MMP_FOURCC fourcc_in);
    virtual ~CMmpDecoderFfmpegEx1();

    virtual MMP_RESULT Open();
    virtual MMP_RESULT Close();
    
    enum MMP_FOURCC GetFourccIn() { return m_fourcc_in; }

public:    
    static int ffmpeg_get_buffer_cb_stub( struct AVCodecContext *p_context,  AVFrame *p_ff_pic );
    static void ffmpeg_release_buffer_cb_stub( struct AVCodecContext *p_context,  AVFrame *p_ff_pic );

    virtual int ffmpeg_get_buffer_cb(struct AVCodecContext *p_context,  AVFrame *p_ff_pic);
    virtual void ffmpeg_release_buffer_cb(struct AVCodecContext *p_context,  AVFrame *p_ff_pic);

public:
    virtual MMP_RESULT DecodeDSI(MMP_U8* pStream, MMP_U32 nStreamSize);
    virtual MMP_RESULT DecodeDSI(class mmp_buffer_audiostream* p_buf_audiostream);
    virtual MMP_RESULT DecodeDSI(class mmp_buffer_videostream* p_buf_videostream, MMP_S32 pic_width, MMP_S32 pic_height);
    
};

#endif
#endif
