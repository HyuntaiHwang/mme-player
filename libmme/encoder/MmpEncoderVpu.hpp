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

#ifndef MMPENCODERVPU_HPP__
#define MMPENCODERVPU_HPP__

#include "MmpDecoder.hpp"
#include "TemplateList.hpp"
#include "vpuapi.h"
#include "vpurun.h"

#define ENC_SRC_BUF_NUM			2


class CMmpEncoderVpu 
{
private:
    enum {
        MAX_FRAMEBUFFER_COUNT = 16
    };


protected:
    struct MmpEncoderCreateConfig m_create_config;

    Uint32 m_codec_idx;
    Uint32 m_version;
    Uint32 m_revision;
    Uint32 m_productId;

    int m_mapType;
        
    int m_regFrameBufCount;
    int m_framebufSize;
    int m_framebufWidth;
    int m_framebufHeight;
    int m_framebufStride;

    EncOpenParam m_encOP;
    EncHandle	m_EncHandle;
    vpu_buffer_t m_vbStream;
    EncInitialInfo m_enc_init_info;
    MaverickCacheConfig m_encCacheConfig;
    
    FrameBuffer	m_fbSrc[ENC_SRC_BUF_NUM];

    MMP_U8 m_DSI[1024*100];
    MMP_S32 m_DSISize;
    int m_srcFrameIdx;
	        
protected:
    CMmpEncoderVpu(struct MmpEncoderCreateConfig *pCreateConfig);
    virtual ~CMmpEncoderVpu();

    virtual MMP_RESULT Open();
    virtual MMP_RESULT Close();

    //void PostProcessing(AVFrame *pAVFrame_Decoded, AVCodecContext *pAVCodecContext);
private:
    void make_encOP_Common();
    void make_encOP_H263();
    void make_encOP_H264();
    void make_encOP_MPEG4();

    void make_user_frame();
	

protected:
    virtual MMP_RESULT EncodeDSI();
    
    MMP_RESULT EncodeAuEx1(CMmpMediaSampleEncode* pMediaSample, CMmpMediaSampleEncodeResult* pEncResult);

    //virtual MMP_RESULT DecodeAu(CMmpMediaSample* pMediaSample, CMmpMediaSampleDecodeResult* pDecResult);
    
};

#endif

