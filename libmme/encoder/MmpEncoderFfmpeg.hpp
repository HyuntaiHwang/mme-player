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

#ifndef MMPENCODERFFMPEG_HPP__
#define MMPENCODERFFMPEG_HPP__

#include "MmpEncoderVideo.hpp"
#include "TemplateList.hpp"

#if (FFMPEG_VERSION == FFMPEG_VERSION_2_6_1)

extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavutil/mem.h"
#include "libavresample/audio_convert.h"
}

struct audio_decoded_packet {
    MMP_U8* p_buffer;
    MMP_U32 buf_size;
    MMP_U32 buf_index;
    MMP_U32 timestamp;
};

class CMmpEncoderFfmpeg 
{
protected:
    struct config {
        /* basic info */
        enum MMP_FOURCC fourcc_out;
                
        /* Inport info */
        MMP_S32 pic_width;
        MMP_S32 pic_height;
        MMP_S32 fr; /* nFrameRate; */
    
        /* Outport info */
        MMP_S32 ip; /*nIDRPeriod;*/
        MMP_S32 br; /*bitrate */
                
        union {
            MMP_VIDEO_PARAM_H264TYPE h264;
            MMP_VIDEO_PARAM_H263TYPE h263;
            MMP_VIDEO_PARAM_MPEG4TYPE mpeg4;
        }codec;
    };


protected:
    AVCodecID m_AVCodecID;
    AVCodec *m_pAVCodec;
    AVCodecContext *m_pAVCodecContext;
    AVFrame *m_pAVFrame_Input;

    MMP_U8* m_extra_data;

    
protected:
    CMmpEncoderFfmpeg();
    virtual ~CMmpEncoderFfmpeg();

    virtual MMP_RESULT Open();
    virtual MMP_RESULT Close();

    //void PostProcessing(AVFrame *pAVFrame_Decoded, AVCodecContext *pAVCodecContext);

public:
    virtual MMP_RESULT EncodeDSI(CMmpEncoderFfmpeg::config *p_encodr_config);
};

#endif 
#endif

