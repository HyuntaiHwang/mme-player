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

#ifndef MMPDECODERVIDEO_VPUEX1_HPP__
#define MMPDECODERVIDEO_VPUEX1_HPP__

#include "MmpDecoderVideo.hpp"
#include "MmpDecoderVpuEx1.hpp"

#define CLASS_DECODER_VPU CMmpDecoderVpuEx1

class CMmpDecoderVideo_VpuEx1 : public CMmpDecoderVideo, CLASS_DECODER_VPU
{
friend class CMmpDecoderVideo;

private:
    class mmp_vpu_if *m_p_vpu_if;

    MMP_BOOL m_bOpenVPU;
    MMP_BOOL m_bDecodeDSI;
    TMinHeap<MMP_S64> m_minheap_ts;
    MMP_S64 m_last_pts;
    MMP_S32 m_decode_dsi_fail_count;
    
private:
    CMmpDecoderVideo_VpuEx1(struct CMmpDecoderVideo::create_config *p_create_config);
    virtual ~CMmpDecoderVideo_VpuEx1();

    virtual MMP_RESULT Open();
    virtual MMP_RESULT Close();

    virtual const MMP_CHAR* GetClassName() { return (const MMP_CHAR*)"VPU";}
    MMP_BOOL check_reconfig(MMP_BOOL is_msg, MMP_CHAR* msg_name, enum ACTION* next_actoin);

public:
    virtual MMP_RESULT flush_buffer_in();
    virtual MMP_RESULT flush_buffer_out();
    virtual MMP_RESULT clear_display_flag(class mmp_buffer_videoframe* p_buf_vf);
    virtual struct mmp_rect get_display_crop_rect();

    virtual MMP_RESULT DecodeAu(class mmp_buffer_videostream* p_buf_vs, class mmp_buffer_videoframe** pp_buf_vf, enum ACTION *next_action);
    virtual MMP_RESULT DecodeAu(class mmp_buffer_videostream* p_buf_vs, class mmp_buffer_videoframe* p_buf_vf);
};

#endif

