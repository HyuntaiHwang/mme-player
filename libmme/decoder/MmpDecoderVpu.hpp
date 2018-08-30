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

#ifndef MMPDECODERVPU_HPP__
#define MMPDECODERVPU_HPP__

#include "MmpDecoder.hpp"
#include "TemplateList.hpp"
#include "vpuapi.h"
#include "vpurun.h"
#include "mmp_buffer_mgr.hpp"

class CMmpDecoderVpu 
{
private:
    enum {
        MAX_FRAMEBUFFER_COUNT = 16
    };

protected:
    struct MmpDecoderCreateConfig m_create_config;

    Uint32 m_codec_idx;
    Uint32 m_version;
    Uint32 m_revision;
    Uint32 m_productId;

    int m_mapType;
    
    MMP_U8* m_p_dsi_stream;
    MMP_S32 m_dsi_stream_size;
    
    int m_regFrameBufCount;
    int m_framebufSize;

    DecOpenParam m_decOP;
    DecHandle	m_DecHandle;
    vpu_buffer_t m_vbStream;
    DecInitialInfo m_dec_init_info;
    DecOutputInfo m_output_info;
	vpu_buffer_t m_vbFrame[MAX_FRAMEBUFFER_COUNT];
    
    class mmp_buffer* m_p_decoded_buffer[MAX_FRAMEBUFFER_COUNT];
        
protected:
    CMmpDecoderVpu(struct MmpDecoderCreateConfig *pCreateConfig);
    virtual ~CMmpDecoderVpu();

    virtual MMP_RESULT Open();
    virtual MMP_RESULT Close();

    //void PostProcessing(AVFrame *pAVFrame_Decoded, AVCodecContext *pAVCodecContext);
private:
    void make_decOP_Common();
    void make_decOP_H263();
    void make_decOP_H264();
    void make_decOP_MPEG4();
    void make_decOP_MPEG2();
    void make_decOP_VC1();
    void make_decOP_MSMpeg4V3();
    void make_decOP_RV30();
    void make_decOP_RV40();
    void make_decOP_VP80();

    void make_user_frame();

	static void vdi_memcpy_stub(void* param, void* dest_vaddr, void* src_paddr, int size);

private:
    MMP_RESULT DecodeDSI_CheckStream_Mpeg4(MMP_U8* pStream, MMP_U32 nStreamSize);

    MMP_RESULT DecodeAu_StreamRemake_AVC1(CMmpMediaSample* pMediaSample);

protected:
    virtual MMP_RESULT DecodeDSI(MMP_U8* pStream, MMP_U32 nStreamSize);
    
    MMP_RESULT DecodeAu_PinEnd(CMmpMediaSample* pMediaSample, CMmpMediaSampleDecodeResult* pDecResult);

    //virtual MMP_RESULT DecodeAu(CMmpMediaSample* pMediaSample, CMmpMediaSampleDecodeResult* pDecResult);
    
};

#endif

