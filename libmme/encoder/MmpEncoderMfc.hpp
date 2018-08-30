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

#ifndef MMPENCODERMFC_HPP__
#define MMPENCODERMFC_HPP__

#include "MmpEncoder.hpp"
#include "TemplateList.hpp"

#include "SsbSipMfcApi.h"

#define MMP_ENCODER_MFC_INPUT_BUF_COUNT 1
class CMmpEncoderMfc 
{
protected:
    struct MmpEncoderCreateConfig m_CreateConfig;

    void* m_hMFCHandle;
    SSBSIP_MFC_CODEC_TYPE m_MFCCodecType;
    SSBSIP_MFC_IMG_RESOLUTION m_MFCImgResol;
    SSBSIP_MFC_ENC_MPEG4_PARAM m_Mpeg4param;
    SSBSIP_MFC_ENC_H264_PARAM m_H264param;
    SSBSIP_MFC_ENC_H263_PARAM m_H263Param;
    SSBSIP_MFC_ENC_INPUT_INFO m_mfc_input_info[MMP_ENCODER_MFC_INPUT_BUF_COUNT];
    
protected:
    CMmpEncoderMfc(struct MmpEncoderCreateConfig *pCreateConfig);
    virtual ~CMmpEncoderMfc();

    virtual MMP_RESULT Open();
    virtual MMP_RESULT Close();

    void Set_Mpeg4Enc_Param(SSBSIP_MFC_ENC_MPEG4_PARAM *pMpeg4Param);
    void Set_H264Enc_Param(SSBSIP_MFC_ENC_H264_PARAM *pH264Arg);
    void Set_H263Enc_Param(SSBSIP_MFC_ENC_H263_PARAM *pH263Param);

public:
    virtual MMP_RESULT EncodeDSI(CMmpMediaSampleEncode* pMediaSample, CMmpMediaSampleEncodeResult* pEncResult);
    
    
};

#endif

