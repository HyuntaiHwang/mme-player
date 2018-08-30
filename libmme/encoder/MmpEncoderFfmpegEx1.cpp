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

#include "MmpEncoderFfmpegEx1.hpp"
#include "MmpUtil.hpp"

#if (FFMPEG_VERSION == FFMPEG_VERSION_n3_1_1)

/////////////////////////////////////////////////////////////
//CMmpEncoderFfmpegEx1 Member Functions

#define AV_CODEC_ID_FFMPEG    MKBETAG('F','F','M','P')

CMmpEncoderFfmpegEx1::CMmpEncoderFfmpegEx1() :

m_pAVCodec(NULL)
,m_pAVCodecContext(NULL)
,m_pAVFrame_Input(NULL)
,m_extra_data(NULL)

{
    avcodec_register_all();

#if 0
    switch(p_create_config->fourcc_out) {
    
#if 0
        /* Audio */
        case MMP_WAVE_FORMAT_MPEGLAYER3: m_AVCodecID = AV_CODEC_ID_MP3; break;
        case MMP_WAVE_FORMAT_MPEGLAYER2: m_AVCodecID = AV_CODEC_ID_MP2; break;
        case MMP_WAVE_FORMAT_MPEGLAYER1: m_AVCodecID = AV_CODEC_ID_MP1; break;
        case MMP_WAVE_FORMAT_WMA2: m_AVCodecID = AV_CODEC_ID_WMAV2; break;
        case MMP_WAVE_FORMAT_AC3: m_AVCodecID = AV_CODEC_ID_AC3; break;
        case MMP_WAVE_FORMAT_AAC: m_AVCodecID = AV_CODEC_ID_AAC; break;
        case MMP_WAVE_FORMAT_FLAC: m_AVCodecID = AV_CODEC_ID_FLAC; break;
        case MMP_WAVE_FORMAT_ADPCM_MS: m_AVCodecID = AV_CODEC_ID_ADPCM_MS; break;
        case MMP_WAVE_FORMAT_FFMPEG: m_AVCodecID = (AVCodecID)AV_CODEC_ID_FFMPEG; break;
#endif
        
        /* Video */
        case MMP_FOURCC_VIDEO_H263: m_AVCodecID=AV_CODEC_ID_H263; break;
        case MMP_FOURCC_VIDEO_H264: m_AVCodecID=AV_CODEC_ID_H264; break;
        case MMP_FOURCC_VIDEO_MPEG4: m_AVCodecID=AV_CODEC_ID_MPEG4; break;
        
        default:  m_AVCodecID = AV_CODEC_ID_NONE;
    }
#endif
}

CMmpEncoderFfmpegEx1::~CMmpEncoderFfmpegEx1()
{

}

MMP_RESULT CMmpEncoderFfmpegEx1::Open()
{
    
    return MMP_SUCCESS;
}


MMP_RESULT CMmpEncoderFfmpegEx1::Close()
{
    if(this->m_pAVCodecContext != NULL) {
        avcodec_close(this->m_pAVCodecContext);
        av_free(this->m_pAVCodecContext);
        this->m_pAVCodecContext = NULL;
    }
    
    if(m_pAVFrame_Input != NULL) {
        av_frame_free(&m_pAVFrame_Input);
        m_pAVFrame_Input = NULL;
    }

    if(m_extra_data != NULL) {
        delete [] m_extra_data;
        m_extra_data = NULL;
    }

    return MMP_SUCCESS;
}

MMP_RESULT CMmpEncoderFfmpegEx1::EncodeDSI(CMmpEncoderFfmpegEx1::config *p_encodr_config) 
{
    AVRational avr;
    
    AVCodec *codec;
    AVCodecContext *cc= NULL;
    AVCodecContext *cc1= NULL;
    MMP_U32 key=0;

    switch(p_encodr_config->fourcc_out) {
    
        /* Video */
        case MMP_FOURCC_VIDEO_H263: m_AVCodecID=AV_CODEC_ID_H263; break;
        case MMP_FOURCC_VIDEO_H264: m_AVCodecID=AV_CODEC_ID_H264; break;
        case MMP_FOURCC_VIDEO_MPEG4: m_AVCodecID=AV_CODEC_ID_MPEG4; break;
        
        default:  m_AVCodecID = AV_CODEC_ID_NONE;
    }
    
    codec = avcodec_find_encoder(m_AVCodecID);
    if(codec == NULL) {
        return MMP_FAILURE;
    }

    cc= avcodec_alloc_context3(codec);
    
    cc->bit_rate = p_encodr_config->br;    /* put sample parameters */
    cc->width = p_encodr_config->pic_width;   /* resolution must be a multiple of two */
    cc->height = p_encodr_config->pic_height;

    /* frames per second */
    avr.num = 1;
    avr.den = p_encodr_config->ip; /*IDR/Period*/
    cc->time_base= avr; //(AVRational){1, 25}; 

    cc->gop_size = p_encodr_config->fr; /*framerate */ /* emit one intra frame every ten frames */
    cc->max_b_frames=1;

    cc->pix_fmt = AV_PIX_FMT_YUV420P;

    
    cc->extradata = NULL;//pStream;
    cc->extradata_size = NULL;//nStreamSize;
    
    if(m_AVCodecID == AV_CODEC_ID_H263) {
        cc->max_b_frames = 0;
    }

    switch(m_AVCodecID) {
        case AV_CODEC_ID_H263:
            cc->max_b_frames = 0;
            break;

        case AV_CODEC_ID_H264:
            cc->max_b_frames = 0;
            cc->profile = FF_PROFILE_H264_BASELINE;
            cc->bit_rate = 100000;
            break;
    }

    /* open it */
    if(avcodec_open2(cc, codec, NULL) < 0) 
    {
        MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpEncoderFfmpegEx1::DecodeDSI] FAIL: could not open codec\n\r")));
        return MMP_FAILURE;
    }

    m_pAVCodec = codec;
    m_pAVCodecContext = cc;
    m_pAVFrame_Input = av_frame_alloc();
   
    
    return MMP_SUCCESS;
}


#endif
