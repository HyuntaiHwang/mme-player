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

#ifndef MMPDECODERVIDEO_VPUT_HPP__
#define MMPDECODERVIDEO_VPUT_HPP__

#include "MmpDecoderVideo.hpp"
#include "MmpDecoderVpuIF.hpp"
#include "mscaler.h"
#include "mmp_oal_mutex.hpp"
#include "mmp_oal_cond.hpp"
#include "mmp_oal_task.hpp"


#define CLASS_DECODER_VPU CMmpDecoderVpuIF


class CMmpDecoderVideo_VpuT : public CMmpDecoderVideo, CLASS_DECODER_VPU
{
friend class CMmpDecoderVideo;

private:
    MMP_BOOL m_bDecodeDSI;
    MScalerHandle m_hScaler;
    int m_mscaler_test_fd;

    MMP_BOOL m_run_task_dec;
    MMP_BOOL m_is_task_main_wait;
    MMP_BOOL m_is_task_dec_wait;
    class mmp_oal_task* m_p_task_dec;
    class mmp_oal_mutex* m_p_mutex_dec_run;
    class mmp_oal_mutex* m_p_mutex_dec_done;
    class mmp_oal_cond* m_p_cond_dec_run;
    class mmp_oal_cond* m_p_cond_dec_done;
    
    class mmp_buffer_videostream* m_p_buf_vs_in;
    class mmp_buffer_videoframe* m_p_buf_vf_vpu_last;
    MMP_U32 m_dec_dur_last;
    MMP_TICKS m_dec_pts_last;
    
    
private:
    static void serviec_dec_stub(void* parm);
    void serviec_dec();

private:
    CMmpDecoderVideo_VpuT(struct CMmpDecoderVideo::create_config *p_create_config);
    virtual ~CMmpDecoderVideo_VpuT();

    virtual MMP_RESULT Open();
    virtual MMP_RESULT Close();

    virtual const MMP_CHAR* GetClassName() { return (const MMP_CHAR*)"VPU";}

private:
    static MMP_BOOL CheckSupportCodec(enum MMP_FOURCC fourcc_in);
    virtual MMP_RESULT Play_Function_Tool(MMP_PLAY_FORMAT playformat, MMP_S64 curpos, MMP_S64 totalpos);
        
    MMP_RESULT do_csc(class mmp_buffer_videoframe* p_buf_vf_vpu, class mmp_buffer_videoframe* p_buf_vf_decoded);
    MMP_RESULT do_csc_hw(class mmp_buffer_videoframe* p_buf_vf_vpu, class mmp_buffer_videoframe* p_buf_vf_decoded);
    MMP_RESULT do_csc_mscaler_test(class mmp_buffer_videoframe* p_buf_vf_vpu, class mmp_buffer_videoframe* p_buf_vf_decoded);
    MMP_RESULT do_csc_sw(class mmp_buffer_videoframe* p_buf_vf_vpu, class mmp_buffer_videoframe* p_buf_vf_decoded);
    
public:
    virtual MMP_RESULT DecodeAu(class mmp_buffer_videostream* p_buf_vs, class mmp_buffer_videoframe** pp_buf_vf, MMP_BOOL *is_reconfig);
    virtual MMP_RESULT DecodeAu(class mmp_buffer_videostream* p_buf_vs, class mmp_buffer_videoframe* p_buf_vf);
};

#endif

