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

#include "MmpDecoderAudio.hpp"
#include "MmpUtil.hpp"
#include "MmpDecoderAudio_Ffmpeg.hpp"
//#include "MmpDecoderAudio_FfmpegEx1.hpp"
//#include "MmpDecoderAudio_FfmpegEx2.hpp"
#include "MmpDecoderAudio_FfmpegEx3.hpp"

/////////////////////////////////////////////////////////////
//Create Object 

CMmpDecoderAudio* CMmpDecoderAudio::CreateObject(struct CMmpDecoderAudio::create_config *p_create_config) {

    CMmpDecoderAudio* pObj;
   	
#if (FFMPEG_VERSION == FFMPEG_VERSION_2_6_1)
    pObj=new CMmpDecoderAudio_Ffmpeg(p_create_config);
#elif (FFMPEG_VERSION == FFMPEG_VERSION_n3_1_1)
    pObj=new CMmpDecoderAudio_FfmpegEx3(p_create_config);
#else
    pObj = NULL;
#endif
	if(pObj != NULL) {
        
        if( pObj->Open()!=MMP_SUCCESS ) {
            pObj->Close();
            delete pObj;
            pObj = NULL;
        }
    }

    return pObj;
}


/////////////////////////////////////////////////////////////
//CMmpDecoderAudio Member Functions

CMmpDecoderAudio::CMmpDecoderAudio(struct CMmpDecoderAudio::create_config *p_create_config, MMP_CHAR* class_name, 
                     const enum MMP_FOURCC *support_fourcc_in_list, MMP_S32 support_fourcc_in_count,
                     const enum MMP_FOURCC *support_fourcc_out_list, MMP_S32 support_fourcc_out_count) : 
     
 CMmpDecoder(p_create_config->fourcc_in, p_create_config->fourcc_out, class_name, 
            support_fourcc_in_list, support_fourcc_in_count,
            support_fourcc_out_list, support_fourcc_out_count) 

,m_wma_fmt(MMP_AUDIO_WMAFormatUnused)
,m_wma_profile(MMP_AUDIO_WMAProfileUnused)

{
    memset(&m_af, 0x00, sizeof(m_af));

    m_af.fourcc = p_create_config->fourcc_in;
    m_af.sample_rate = p_create_config->sr;
    m_af.channels = p_create_config->ch;
    m_af.bits_per_sample = p_create_config->bps;
}

CMmpDecoderAudio::~CMmpDecoderAudio() {

}

MMP_RESULT CMmpDecoderAudio::Open() {
    MMP_RESULT mmpResult;

    mmpResult=CMmpDecoder::Open();
    if(mmpResult!=MMP_SUCCESS)
    {
        return mmpResult;
    }

    return MMP_SUCCESS;
}

MMP_RESULT CMmpDecoderAudio::Close() {
    MMP_RESULT mmpResult;

    mmpResult=CMmpDecoder::Close();
    if(mmpResult!=MMP_SUCCESS) {
        return mmpResult;
    }

    return MMP_SUCCESS;
}
   

void CMmpDecoderAudio::DecodeMonitor(class mmp_buffer_audioframe* p_buf_af) {

    static MMP_U32 before_tick = 0, data_sum = 0;
    MMP_U32 start_tick = m_nClassStartTick, cur_tick;
    MMP_U32 dur_avg = 0;

    if(p_buf_af != NULL) {

        //fps_sum ++;
        //dur_sum += p_buf_af->get_coding_dur();

        //m_nTotalDecDur += p_buf_videoframe->get_coding_dur();
        data_sum += p_buf_af->get_data_size();
 
        cur_tick = CMmpUtil::GetTickCount();
        if( (cur_tick - before_tick) > 1000 ) {
        
            
            MMPDEBUGMSG(1, (TEXT("[MME-AudioDec %s %s %d %d] %d. data=%d "), 
                this->get_class_name(),  this->get_fourcc_in_name(),  p_buf_af->get_samplerate(), p_buf_af->get_channel(),
                        (cur_tick-start_tick)/1000, data_sum ));

            
            before_tick = cur_tick;
            data_sum = 0;
            
        }

    }
}

