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

#ifndef MMPDECODERVPUEX1_HPP__
#define MMPDECODERVPUEX1_HPP__

#include "MmpDecoder.hpp"
#include "TemplateList.hpp"
#include "mmp_buffer_mgr.hpp"
#include "mmp_vpu_if.hpp"
#include "mmp_lock.hpp"
#include "mmp_oal_mutex.hpp"

#define VPU_SUPPORT_THEORA  0
#define VPU_SUPOORT_DIVX   0
#define VPU_SUPOORT_DIV5   0

#if (VPU_SUPPORT_THEORA == 1)
#include "theora_parser.h"
#endif

//#define CMmpDecoderVpuEx1_DUMP_RAW_STREAM 
//#define CMmpDecoderVpuEx1_CAPTURE_FRAME

class CMmpDecoderVpuEx1 
{
private:
    enum {
        MAX_FRAMEBUFFER_COUNT = MMP_VIDEO_DECODED_BUF_ACTUAL_MAX_COUNT
    };

    //MPEG4 CLASS 0(MPEG4) / 1(DIVX 5.0 or higher) / 2(XVID) / 5(DIVX 4.0) / 8(DIVX/XVID Auto Detect)/ 256(Sorenson spark) :
    enum MPEG4_CLASS {
        MPEG4_CLASS_BASE = 0,    //Mpeg4 
        MPEG4_CLASS_DIVX50 = 1,  //(DIVX 5.0 or higher)
        MPEG4_CLASS_XVID =2,      //XVID  
        MPEG4_CLASS_DIVX40 = 5,  //DIVX4.0
        MPEG4_CLASS_AUTO_DETECT = 8,  //DIVX/XVID Auto Detect
        MPEG4_CLASS_SORENSON_SPARK = 256,  //256(Sorenson spark)
    };

protected:
    enum {
        SKIP_TYPE=(1<<0),
        CHUNK_REUSE=(1<<1),
        H264_SPS_PPS=(1<<2),
        SEQUENCE_CHANGE=(1<<3),
        VPU_TIMEOUT=(1<<4),
    };

private:
    enum MMP_FOURCC m_fourcc_in;
    enum MMP_FOURCC m_fourcc_out;
    MMP_S32 m_vpu_buf_width;
    MMP_S32 m_vpu_buf_height;

    MMP_S32 m_vpu_instance_index;

    Uint32 m_codec_idx;
    Uint32 m_version;
    Uint32 m_revision;
    Uint32 m_productId;
    Uint32 m_iframeSearchEnable;
    Uint32 m_skipframeMode;
    MMP_PLAY_FORMAT m_randomaccess;

    int m_mapType;
        
    int m_regFrameBufCount;
    int m_framebufSize;
    DecOpenParam m_decOP;
    DecHandle	m_DecHandle;
    
    DecInitialInfo m_dec_init_info;
    DecOutputInfo m_output_info;
	vpu_buffer_t m_vbFrame[MAX_FRAMEBUFFER_COUNT];
    
    class mmp_buffer* m_p_stream_buffer;
    vpu_buffer_t m_vpu_stream_buffer;

#if (VPU_SUPPORT_THEORA == 1)
	//thoParser
	tho_parser_t *m_thoParser;
#endif
    MMP_BOOL m_is_alloc_vf;
    class mmp_buffer_videoframe* m_p_buf_vf_arr[MAX_FRAMEBUFFER_COUNT];
    MMP_S32 m_buf_used_count[MAX_FRAMEBUFFER_COUNT];
    class mmp_buffer_videoframe* m_p_buf_vf_req_clear_before_dsi[MAX_FRAMEBUFFER_COUNT]; 
    MMP_BOOL m_is_complete_decode_dsi;

    MMP_S32 m_last_int_reason;
    MMP_S32 m_input_stream_count;

    MMP_S32 m_input_stream_size;
    MMP_S32 m_room1, m_room2;

    MMP_BOOL m_is_streamming_on;

    MMP_U8 *m_copyed_aubuf;
    MMP_S32 m_copyed_aubuf_max_size;
    
    /* h264, avc */
protected:
    MMP_S32 m_is_avc;
    MMP_S32 m_nal_length_size;
    MMP_S32 m_last_display_index;

    MMP_S32 m_DEBUG_VPUDec;

private:

#ifdef CMmpDecoderVpuEx1_DUMP_RAW_STREAM
    FILE* m_fp_dump_raw_stream;
    char m_filename_dump_raw_stream[128];
#endif

protected:
    CMmpDecoderVpuEx1(enum MMP_FOURCC fourcc_in);
    virtual ~CMmpDecoderVpuEx1();

    virtual MMP_RESULT Open(class mmp_vpu_if *p_vpu_if);
    virtual MMP_RESULT Close(class mmp_vpu_if *p_vpu_if);
        
private:
    void make_decOP_Common();
    void make_decOP_H263();
    void make_decOP_H264();
    void make_decOP_MPEG4(enum MPEG4_CLASS);
    void make_decOP_MPEG2();
    void make_decOP_VC1();
    void make_decOP_MSMpeg4V3();
    void make_decOP_RV30();
    void make_decOP_RV40();
    void make_decOP_VP80();
#if (VPU_SUPPORT_THEORA == 1)
    void make_decOP_Theora();
#endif

    MMP_RESULT make_seqheader_Common(class mmp_buffer_videostream* p_buf_vs);
    MMP_RESULT make_seqheader_H264(class mmp_buffer_videostream* p_buf_vs);
    MMP_RESULT make_seqheader_WMV3(class mmp_buffer_videostream* p_buf_vs);
    MMP_RESULT make_seqheader_DIV3(class mmp_buffer_videostream* p_buf_vs);
    MMP_RESULT make_seqheader_RV(class mmp_buffer_videostream* p_buf_vs);
    MMP_RESULT make_seqheader_VP8(class mmp_buffer_videostream* p_buf_vs);
#if (VPU_SUPPORT_THEORA == 1)
    MMP_RESULT make_seqheader_Theora(class mmp_buffer_videostream* p_buf_vs);
#endif

    MMP_RESULT make_frameheader_Common(class mmp_buffer_videostream* p_buf_vs);
    MMP_RESULT make_frameheader_H264(class mmp_buffer_videostream* p_buf_vs, MMP_S32* copyed_ausize);
    MMP_RESULT make_frameheader_WVC1(class mmp_buffer_videostream* p_buf_vs); /* VC1 Advanced Profile */
    MMP_RESULT make_frameheader_WMV3(class mmp_buffer_videostream* p_buf_vs);
    MMP_RESULT make_frameheader_DIV3(class mmp_buffer_videostream* p_buf_vs);
    MMP_RESULT make_frameheader_RV(class mmp_buffer_videostream* p_buf_vs);
    MMP_RESULT make_frameheader_VP8(class mmp_buffer_videostream* p_buf_vs);

#if (VPU_SUPPORT_THEORA == 1)
    MMP_RESULT make_frameheader_Theora(class mmp_buffer_videostream* p_buf_vs);
#endif

private:
    MMP_RESULT InitVpuFrameBuffer(class mmp_vpu_if *p_vpu_if);
    MMP_RESULT DeinitVpuFrameBuffer();

    inline void SetVpuRealFrameBufferCount(MMP_S32 bufcnt) { m_regFrameBufCount = bufcnt;}
        
protected:

    MMP_RESULT AllocVpuFrameBuffer(class mmp_vpu_if *p_vpu_if);
    MMP_RESULT SetVpuFrameBuffer(class mmp_vpu_if *p_vpu_if, class mmp_buffer_videoframe** p_buf_vf_arr, MMP_S32 bufcnt);

    MMP_RESULT ClearDisplayFlag(class mmp_vpu_if *p_vpu_if, class mmp_buffer_videoframe* p_buf_vf);

    MMP_S32 GetVpuRealFrameBufferCount() { return m_regFrameBufCount;}
    MMP_S32 GetVpuMinFrameBufferCount() { return m_dec_init_info.minFrameBufferCount; }
    
    enum MMP_FOURCC GetFourccIn() { return m_fourcc_in; }

    MMP_S32 GetVpuPicWidth() { return m_dec_init_info.picWidth; }
    MMP_S32 GetVpuPicHeight() { return m_dec_init_info.picHeight; }
    MMP_S32 GetVpuBufWidth() { return m_vpu_buf_width; }
    MMP_S32 GetVpuBufHeight() { return m_vpu_buf_height; }

    MMP_S32 GetVpuReconfigWidth() { return m_output_info.decPicWidth; }
    MMP_S32 GetVpuReconfigHeight() { return m_output_info.decPicHeight; }

    MMP_RESULT flush_buffer_in();
    MMP_RESULT flush_buffer_out(class mmp_vpu_if *p_vpu_if);
        
    virtual MMP_RESULT Play_Function_Tool(MMP_PLAY_FORMAT playformat, MMP_S64 curpos, MMP_S64 totalpos);	
    virtual MMP_RESULT DecodeDSI(class mmp_vpu_if *p_vpu_if, class mmp_buffer_videostream* p_buf_vs);
    
    MMP_RESULT DecodeAu_PicEnd_Run(class mmp_vpu_if *p_vpu_if, class mmp_buffer_videostream* p_buf_vs);
    MMP_RESULT DecodeAu_PicEnd_WaitDone(class mmp_vpu_if *p_vpu_if, class mmp_buffer_videostream* p_buf_vs, class mmp_buffer_videoframe** pp_buf_vf, MMP_U32 *vpu_result);
    
};

#endif

