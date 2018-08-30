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

#ifndef MMPENCODERVPUIF_HPP__
#define MMPENCODERVPUIF_HPP__

#include "MmpDecoder.hpp"
#include "TemplateList.hpp"
#include "vpuapi.h"
#include "vpurun.h"
#include "mmp_vpu_if.hpp"
#include "mmp_oal_mutex.hpp"

struct MmpEncoderProperty_AnapassVPU{

    //YUV_SRC_IMG           \\naslab\006.stream\images\08.ENC_TEST\ski_1280x720.yuv	// source YUV image file
    MMP_U32 FRAME_NUMBER_ENCODED;//  1000   			// number of frames to be encoded
    MMP_U32 PICTURE_WIDTH;//         1280   			// picture width
    MMP_U32 PICTURE_HEIGHT;//        720   			// picture height
    MMP_U32 FRAME_RATE;//            30   			// frame rate
    MMP_U32 CONSTRAINED_INTRA;//     0   			// constrained_intra_pred_flag
    //;-----------------------
    //; DEBLKING FILTER
    //;-----------------------
    MMP_U32 DISABLE_DEBLK;//         0  			// disable_deblk (0 : enable, 1 : disable, 2 : disable at slice boundary)
    MMP_U32 DEBLK_ALPHA;//           0   			// deblk_filter_offset_alpha (-6 ~ 6)
    MMP_U32 DEBLK_BETA;//            0   			// deblk_filter_offset_beta  (-6 ~ 6)
    MMP_U32 CHROMA_QP_OFFSET;//      2   			// chroma_qp_offset (-12 ~ 12)
    MMP_U32 PIC_QP_Y;//              12   			// pic_qp_y (0 ~ 51)
    MMP_U32 GOP_PIC_NUMBER;//        30   			// GOP picture number (0 : only first I, 1 : all I, 3 : I,P,P,I,)
    //;-----------------------
    //; SLICE STRUCTURE
    //;-----------------------
    MMP_U32 SLICE_MODE;//            0   			// slice mode (0 : one slice, 1 : multiple slice)
    MMP_U32 SLICE_SIZE_MODE;//       0   			// slice size mode (0 : slice bit number, 1 : slice mb number)
    MMP_U32 SLICE_SIZE_NUMBER;//     8192   			// slice size number (bit count or mb number)
    //;-----------------------
    //; RATE CONTROL
    //;-----------------------
    MMP_U32 RATE_CONTROL_ENABLE;//   1   			// rate control enable
    MMP_U32 BIT_RATE_KBPS;//         3200 	// bit rate in kbps (ignored if rate control disable)
    MMP_U32 DELAY_IN_MS;//           0   			// delay in ms (initial decoder buffer delay) (0 : ignore)
    MMP_U32 VBV_BUFFER_SIZE;//       0   			// reference decoder buffer size in bits (0 : ignore)
    //;-----------------------
    //; ERROR RESILIENCE
    //;-----------------------
    MMP_U32 INTRA_MB_REFRESH;//      0  			// Intra MB Refresh (0 - None, 1 ~ MbNum-1)
    //;-----------------------
    //; ADDITIONAL PARAMETER
    //;-----------------------
    MMP_U32 SEARCH_RANGE;//            1   // 3: 16x16, 2:32x16, 1:64x32, 0:128x64, H.263(Short Header : always 0)
    MMP_U32 ME_USE_ZERO_PMV;//         1   // 0: PMV_ENABLE, 1: PMV_DISABLE
    MMP_U32 WEIGHT_INTRA_COST;//       0   // Intra weight when compare Intra and Inter

};


#define ENC_SRC_BUF_NUM			2


class CMmpEncoderVpuIF 
{
private:
    enum {
        MAX_FRAMEBUFFER_COUNT = 16
    };

protected:
    MMP_FOURCC m_fourcc_in;
    MMP_FOURCC m_fourcc_out;
    MMP_S32 m_pic_width;
    MMP_S32 m_pic_height;

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
    //vpu_buffer_t m_vbStream;
    EncInitialInfo m_enc_init_info;
    MaverickCacheConfig m_encCacheConfig;
        
    MMP_U8 m_DSI[1024*100];
    MMP_S32 m_DSISize;
      
    class mmp_buffer* m_p_enc_buffer;
    vpu_buffer_t m_vpu_enc_buffer;

    class mmp_buffer* m_p_src_frame_buffer;
    vpu_buffer_t m_vpu_src_frame_buffer;
    FrameBuffer m_FrameBuffer_src;
    class mmp_buffer* m_p_frame_buffer[MAX_FRAMEBUFFER_COUNT];
    
    MMP_BOOL m_is_streamming_on;

    MMP_S32 m_is_cbcr_interleave;
    MMP_BOOL m_DEBUG_VPUEnc;
    MMP_S32 m_encoed_frame_count;

protected:
    CMmpEncoderVpuIF(MMP_FOURCC fourcc_in, MMP_FOURCC fourcc_out, MMP_S32 pic_width, MMP_S32 pic_height);
    virtual ~CMmpEncoderVpuIF();

    virtual MMP_RESULT Open(class mmp_vpu_if *p_vpu_if, class mmp_buffer_videoframe* p_buf_vf);
    virtual MMP_RESULT Close(class mmp_vpu_if *p_vpu_if);

    inline void set_pic_width(MMP_S32 w) { m_pic_width = w; }
    inline void set_pic_height(MMP_S32 h) { m_pic_height = h; }
    
    //void PostProcessing(AVFrame *pAVFrame_Decoded, AVCodecContext *pAVCodecContext);
private:
    void make_encOP_Common();
    void make_encOP_H263();
    void make_encOP_H264();
    void make_encOP_MPEG4();
        
protected:
    void print_reg();
    virtual MMP_RESULT EncodeDSI(class mmp_vpu_if *p_vpu_if, class mmp_buffer_videoframe* p_buf_vf);
    virtual MMP_RESULT EncodeAu(class mmp_vpu_if *p_vpu_if, class mmp_buffer_videoframe* p_buf_videoframe, class mmp_buffer_videostream* p_buf_videostream);
};

#endif

