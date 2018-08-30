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

#ifndef _MMPDECODERAUDIO_HPP__
#define _MMPDECODERAUDIO_HPP__

#include "MmpDecoder.hpp"

#define MMP_DEFAULT_AUDIO_DECODED_BUFSIZE 8192
#define MMP_DEFAULT_AUDIO_OUT_CHANNEL    2
#define MMP_DEFAULT_AUDIO_OUT_SAMPLEBITS 16

class CMmpDecoderAudio : public CMmpDecoder
{
public:
    struct create_config {
        enum MMP_FOURCC fourcc_in;
        enum MMP_FOURCC fourcc_out;

        MMP_S32 sr; /* sample rate */
        MMP_S32 ch; /* channel count , mono/strero */
        MMP_S32 bps; /* bits/sample */
    
        public:
            create_config() :
                fourcc_out(MMP_FOURCC_AUDIO_PCM)
                ,sr(44100) /* 만약에 codec_config없이 Decoding을 시도하는 Player가 있을 수 있다.  최대한 Reconfig를 피하기 위해 44100으로 설정한다. */
                ,ch(2)
                ,bps(16)
            { 
                
            }

    };

public:
    static CMmpDecoderAudio* CreateObject(struct CMmpDecoderAudio::create_config *p_create_config);
    
private:
    enum {
        DECODED_BUF_MAX_COUNT = 32
    };

    /* Audio Format */
    struct mmp_audio_format m_af;

    /* WMA */
    enum MMP_AUDIO_WMAFORMATTYPE m_wma_fmt;
    enum MMP_AUDIO_WMAPROFILETYPE m_wma_profile;

    //MMP_S32 m_sr; /* m_sampling_freq; */
    //MMP_S32 m_ch; /* m_channel */
    //MMP_S32 m_bps; /* m_bits_per_sample */
    
protected:
    CMmpDecoderAudio(struct CMmpDecoderAudio::create_config *p_create_config, MMP_CHAR* class_name, 
                     const enum MMP_FOURCC *support_fourcc_in_list, MMP_S32 support_fourcc_in_count,
                     const enum MMP_FOURCC *support_fourcc_out_list, MMP_S32 support_fourcc_out_count
                );
    virtual ~CMmpDecoderAudio();

    virtual MMP_RESULT Open();
    virtual MMP_RESULT Close();

    
public:
    //void config(MMP_S32 sampling_freq, MMP_S32 channel);

public:
    //inline MMP_S32 get_sr() { return m_sr; }    /* sample rate  */
    //inline MMP_S32 get_ch() { return m_ch; }    /* chanel count */
    //inline MMP_S32 get_bps() { return m_bps; }  /* bits/sample  */
    
    //inline void set_sr(MMP_S32 sr) { m_sr=sr; }     /* sample rate  */
    //inline void set_ch(MMP_S32 ch) { m_ch=ch; }     /* chanel count */
    //inline void set_bps(MMP_S32 bps) { m_bps=bps; } /* bits/sample  */
   
    inline MMP_S32 get_sample_rate() { return m_af.sample_rate; }
    inline MMP_S32 get_channels() { return m_af.channels; }
    inline MMP_S32 get_bits_per_sample() { return m_af.bits_per_sample; }
    inline MMP_S32 get_bitrate() { return m_af.bitrate; }
    inline MMP_S32 get_block_align() { return m_af.block_align; }
    inline MMP_S32 get_frame_size() { return m_af.frame_size; }

    inline void set_sample_rate(MMP_S32 a) { m_af.sample_rate = a; }
    inline void set_channels(MMP_S32 a) { m_af.channels = a; }
    inline void set_bits_per_sample(MMP_S32 a) { m_af.bits_per_sample = a; }
    inline void set_bitrate(MMP_S32 a) { m_af.bitrate = a; }
    inline void set_block_align(MMP_S32 a) { m_af.block_align = a; }
    inline void set_frame_size(MMP_S32 a) { m_af.frame_size = a; }

    /* WMA */
    inline enum MMP_AUDIO_WMAFORMATTYPE get_wma_fmt() { return m_wma_fmt;}
    inline enum MMP_AUDIO_WMAPROFILETYPE get_wma_profile() { return m_wma_profile; }
    inline void get_wma_fmt(enum MMP_AUDIO_WMAFORMATTYPE a) { m_wma_fmt = a;}
    inline void get_wma_profile(enum MMP_AUDIO_WMAPROFILETYPE a) { m_wma_profile = a; }

    virtual MMP_RESULT flush_buffer() { return MMP_SUCCESS;}

public:
    virtual MMP_RESULT DecodeAu(class mmp_buffer_audiostream *p_buf_as, class mmp_buffer_audioframe *p_buf_af) = 0;
    void DecodeMonitor(class mmp_buffer_audioframe* p_buf_af);

};

#endif

