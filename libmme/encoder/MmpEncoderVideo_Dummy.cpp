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

#include "MmpEncoderVideo_Dummy.hpp"
#include "MmpUtil.hpp"
#include "MmpImageTool.hpp"
#include "mmp_lock.hpp"

/////////////////////////////////////////////////////////////
//CMmpEncoderVideo_Dummy Member Functions


CMmpEncoderVideo_Dummy::CMmpEncoderVideo_Dummy(struct CMmpEncoderVideo::create_config *p_create_config) : 

CMmpEncoderVideo(p_create_config, "Dummy")

,m_nEncodedStreamCount(0)
,m_bEncodeOpen(MMP_FALSE)
,m_bEncodeDSI(MMP_FALSE)
{
    
}

CMmpEncoderVideo_Dummy::~CMmpEncoderVideo_Dummy()
{

}

MMP_RESULT CMmpEncoderVideo_Dummy::Open()
{
    MMP_RESULT mmpResult = MMP_SUCCESS;
    
    /* Enc initially check */
    if(mmpResult == MMP_SUCCESS) {
        mmpResult=CMmpEncoderVideo::Open();
        if(mmpResult!=MMP_SUCCESS)   {
            MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpEncoderVideo_Dummy::Open] FAIL : CMmpEncoderVideo::Open() ")));
        }
    }

    return mmpResult;
}


MMP_RESULT CMmpEncoderVideo_Dummy::Close()
{
    MMP_RESULT mmpResult;
        
    mmpResult=CMmpEncoderVideo::Close();
    if(mmpResult!=MMP_SUCCESS)   {
        MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpEncoderVideo_Dummy::Close] CMmpEncoderVideo::Close() \n\r")));
    }
    
    
    return mmpResult;
}

void CMmpEncoderVideo_Dummy::set_pic_width(MMP_S32 w) {

    CMmpEncoderVideo::set_pic_width(w);
}
    
void CMmpEncoderVideo_Dummy::set_pic_height(MMP_S32 h) {
    
    CMmpEncoderVideo::set_pic_height(h);
}

MMP_RESULT CMmpEncoderVideo_Dummy::EncodeAu(class mmp_buffer_videoframe* p_buf_vf, class mmp_buffer_videostream* p_buf_vs) {

    MMP_RESULT mmpResult = MMP_SUCCESS; 
    MMP_U32 preproc_start_tick, enc_start_tick, enc_end_tick;
    MMP_U8* pTemp;
    class mmp_buffer_videoframe* p_buf_vf_src;

    preproc_start_tick = CMmpUtil::GetTickCount();

    p_buf_vs->set_stream_size(0);
    p_buf_vs->set_stream_offset(0);
    p_buf_vs->set_flag(mmp_buffer_media::FLAG_NULL);
    p_buf_vs->set_pts(p_buf_vf->get_pts());
    p_buf_vs->set_dsi_size(0);
    
    

    enc_start_tick = CMmpUtil::GetTickCount();

    p_buf_vs->set_stream_size(1024);
    p_buf_vs->or_flag(mmp_buffer_media::FLAG_NULL);
    CMmpUtil::Sleep(10);

    enc_end_tick = CMmpUtil::GetTickCount();
    
    p_buf_vs->set_preproc_dur(enc_start_tick - preproc_start_tick);
    p_buf_vs->set_coding_dur(enc_end_tick - enc_start_tick);

    return mmpResult;
}