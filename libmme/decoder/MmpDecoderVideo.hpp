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

#ifndef MMPDECODERVIDEO_HPP__
#define MMPDECODERVIDEO_HPP__

#include "MmpDecoder.hpp"
#include "mmp_oal_mutex.hpp"


class CMmpDecoderVideo : public CMmpDecoder
{
friend class CMmpDecoder;

public:
    struct create_config {
        enum MMP_FOURCC fourcc_in;
        enum MMP_FOURCC fourcc_out;
        int pic_width;
        int pic_height;
        int buf_req_count_min;
        int buf_req_count_actual;
        MMP_BOOL is_android_buffer;
    };

    enum ACTION {
        ACTION_NONE = 0,
        ACTION_RECONFIG = 0x100,
        ACTION_CHUNK_REUSE,
        ACTION_FAIL_DECODE_DSI,
        ACTION_FAIL_DECODE_TIMEOUT,
        ACTION_OUTPUT_CROP_CHANGED,
        ACTION_FAIL_DECODE_SetVpuFrameBuffer,
        ACTION_FAIL_DECODE_AllocVpuFrameBuffer,
    };

public:
    static CMmpDecoderVideo* CreateObject(struct CMmpDecoderVideo::create_config *p_create_config, MMP_BOOL bForceFfmpeg = MMP_FALSE);
    
public:
    enum {
        DECODED_BUF_MAX_COUNT = MMP_VIDEO_DECODED_BUF_ACTUAL_MAX_COUNT
    };

private:
    MMP_S32 m_nDecodingAvgFPS;
    
    MMP_U32 m_nTotalDecDur;
    MMP_U32 m_nTotalDecFrameCount;

    /* Demuxer가 container로부터 추출한 값 */
    MMP_S32 m_demuxer_pic_width;
    MMP_S32 m_demuxer_pic_height;

    /* Decoder가 coding stream으로부터 추출한 값 */
    MMP_S32 m_decoder_pic_width;
    MMP_S32 m_decoder_pic_height;
            
    /* Decoder가 display하기를 원하는 영역 */
    struct mmp_rect m_display_crop_rect;

    /* Decoder가 사용하는 buffer의 width/height */
    MMP_S32 m_decoder_buf_width;
    MMP_S32 m_decoder_buf_height;
    
    MMP_S32 m_buf_req_count_min;
    MMP_S32 m_buf_req_count_actual;

    MMP_S32 m_call_count_init_buf_vf_from_demuxer;

    class mmp_buffer_videoframe* m_p_buf_vf_decoded[DECODED_BUF_MAX_COUNT];
    MMP_S32 m_buf_vf_decoded_count;
    MMP_S32 m_buf_vf_index;
    
    MMP_BOOL m_is_android_buffer;

    MMP_U32 m_mon_before_tick;
    MMP_U32 m_mon_fps_sum;
    MMP_U32 m_mon_fps_fail_sum;
    MMP_U32 m_mon_coding_dur_sum;
    MMP_U32 m_mon_postproc_dur_sum;
    MMP_U32 m_mon_au_sum_size;
    MMP_U32 m_mon_au_sum_size_total;

protected:
    MMP_BOOL m_DEBUG_MMEDec;
    MMP_S32 m_decoder_reconfig_width;
    MMP_S32 m_decoder_reconfig_height;
    MMP_S32 m_decoder_reconfig_min_buf_count;
    MMP_S32 m_decoder_reconfig_actual_buf_count;
    
protected:
    CMmpDecoderVideo(struct CMmpDecoderVideo::create_config *p_create_config, MMP_CHAR* class_name, 
                     const enum MMP_FOURCC *support_fourcc_in_list, MMP_S32 support_fourcc_in_count,   
                     const enum MMP_FOURCC *support_fourcc_out_list, MMP_S32 support_fourcc_out_count);
    virtual ~CMmpDecoderVideo();

    virtual MMP_RESULT Open();
	virtual MMP_RESULT Open(MMP_U8* pStream, MMP_U32 nStreamSize) {return MMP_FAILURE;}
    virtual MMP_RESULT Close();
        
public:
    
    /* 이 함수는 Decoding 하기 전에 반드시 한번만 호출되어야 한다. 
       이후 Decoding하면서 config가 바뀌는 것은 Decoder에서 결정한다. 
    */
    MMP_RESULT init_buf_vf_from_demuxer(MMP_S32 demuxer_pic_width, MMP_S32 demuxer_pic_height, enum MMP_FOURCC fourcc_out, MMP_S32 buf_req_cnt_min, MMP_S32 buf_req_cnt_actual);
protected:
    MMP_RESULT init_buf_vf_from_decoder(MMP_S32 demuxer_pic_width, MMP_S32 demuxer_pic_height, enum MMP_FOURCC fourcc_out, MMP_S32 buf_req_cnt_min, MMP_S32 buf_req_cnt_actual);

public:    
    /* renderer로부터 사용하는 버퍼를 받는다. */
    MMP_RESULT use_buf_vf_from_renderer(class mmp_buffer_videoframe* p_buf_vf);
    MMP_RESULT free_buf_vf_from_renderer(class mmp_buffer_videoframe* p_buf_vf);
    
public:
    class mmp_buffer_videoframe* get_buf_vf(MMP_S32 id);
    MMP_BOOL is_next_vf();
    class mmp_buffer_videoframe* get_next_vf(MMP_BOOL auto_update_id = MMP_TRUE);
    inline MMP_S32 get_buf_vf_count() { return m_buf_vf_decoded_count; }
    MMP_S32 get_buf_vf_count_own(MMP_MEDIA_ID id);

protected:
    class mmp_buffer_videoframe** get_buf_vf_array() { return m_p_buf_vf_decoded; }
    
public:
    inline MMP_S32 get_demuxer_pic_width() { return m_demuxer_pic_width; }
    inline MMP_S32 get_demuxer_pic_height() { return m_demuxer_pic_height; }
    inline MMP_S32 get_decoder_pic_width() { return m_decoder_pic_width; }
    inline MMP_S32 get_decoder_pic_height() { return m_decoder_pic_height; }
    inline MMP_S32 get_decoder_reconfig_width() { return m_decoder_reconfig_width; }
    inline MMP_S32 get_decoder_reconfig_height() { return m_decoder_reconfig_height; }
    inline MMP_S32 get_decoder_reconfig_min_buf_count() { return m_decoder_reconfig_min_buf_count; }
    inline MMP_S32 get_decoder_reconfig_actual_buf_count() { return m_decoder_reconfig_actual_buf_count; }
    inline MMP_S32 get_decoder_buf_width() { return m_decoder_buf_width; }
    inline MMP_S32 get_decoder_buf_height() { return m_decoder_buf_height; }
    
    virtual struct mmp_rect get_display_crop_rect() { return m_display_crop_rect; }
    
    inline MMP_S32 get_buf_req_count_min() { return m_buf_req_count_min; }
    inline MMP_S32 get_buf_req_count_actual() { return m_buf_req_count_actual; }
    
    inline MMP_BOOL is_android_buffer() { return m_is_android_buffer; }
    inline void  set_android_buffer(MMP_BOOL bflag) { m_is_android_buffer = bflag; }
    
    virtual MMP_RESULT flush_buffer_in() { return MMP_SUCCESS;}
    virtual MMP_RESULT flush_buffer_out();
    virtual MMP_RESULT flush_buffer() { return MMP_SUCCESS;}

    virtual MMP_RESULT clear_display_flag(class mmp_buffer_videoframe* p_buf_vf) { return MMP_SUCCESS;}

protected:
    MMP_RESULT do_csc_sw(class mmp_buffer_videoframe* p_buf_vf_vpu, class mmp_buffer_videoframe* p_buf_vf_decoded);
    
public:

    MMP_S32 GetAvgFPS();
    MMP_S32 GetAvgDur();
    MMP_S32 GetTotalDecFrameCount() { return m_nTotalDecFrameCount; }

    void print_status(class mmp_buffer_videoframe* p_buf_videoframe, int au_size);

    virtual MMP_RESULT DecodeAu(class mmp_buffer_videostream* p_buf_videostream, class mmp_buffer_videoframe** pp_buf_videoframe, enum ACTION *p_action) = 0;
    virtual MMP_RESULT DecodeAu(class mmp_buffer_videostream* p_buf_vs, class mmp_buffer_videoframe* p_buf_vf) { return MMP_FAILURE;}
};

#endif


