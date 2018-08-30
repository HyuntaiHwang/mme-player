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

#include "MmpDecoderAudio_FfmpegEx3.hpp"
#include "MmpUtil.hpp"

#if (FFMPEG_VERSION == FFMPEG_VERSION_n3_1_1)

#define CH_MONO   1
#define CH_STEREO 2
#define BYTE_PER_SAMPLE 2

/////////////////////////////////////////////////////////////
//CMmpDecoderAudio_FfmpegEx3 Member Functions

static const enum MMP_FOURCC s_support_fourcc_in[]={
     MMP_FOURCC_AUDIO_MP1,
     MMP_FOURCC_AUDIO_MP2,
     MMP_FOURCC_AUDIO_MP3,
     MMP_FOURCC_AUDIO_VORBIS,
     MMP_FOURCC_AUDIO_AAC,
     MMP_FOURCC_AUDIO_AC3,
     MMP_FOURCC_AUDIO_DTS,
     MMP_FOURCC_AUDIO_WMA1,
     MMP_FOURCC_AUDIO_WMA2,
     MMP_FOURCC_VIDEO_WMV1,
     MMP_FOURCC_VIDEO_WMV2,
     MMP_FOURCC_AUDIO_WMA_LOSSLESS,
     MMP_FOURCC_AUDIO_WMA_PRO,
     MMP_FOURCC_AUDIO_FLAC,
     MMP_FOURCC_AUDIO_AMR_NB,
     MMP_FOURCC_AUDIO_AMR_WB,
     MMP_FOURCC_AUDIO_CODEC_PCM_S16LE,
     MMP_FOURCC_AUDIO_FFMPEG,
};

static const enum MMP_FOURCC s_support_fourcc_out[]={
     MMP_FOURCC_AUDIO_PCM
};


CMmpDecoderAudio_FfmpegEx3::CMmpDecoderAudio_FfmpegEx3(struct CMmpDecoderAudio::create_config *p_create_config) : 

CMmpDecoderAudio(p_create_config, "Ffmpeg", 
                 s_support_fourcc_in, sizeof(s_support_fourcc_in)/sizeof(s_support_fourcc_in[0]),
                 s_support_fourcc_out, sizeof(s_support_fourcc_out)/sizeof(s_support_fourcc_out[0])
                 )

,CMmpDecoderFfmpegEx1(p_create_config->fourcc_in)

,m_reconfig_count(0)
,m_sample_count(0)
//,m_queue_decoded(60)
,m_last_pts(0)
,m_pAVAudioResampleContext(NULL)
{
    
}

CMmpDecoderAudio_FfmpegEx3::~CMmpDecoderAudio_FfmpegEx3()
{

}

MMP_RESULT CMmpDecoderAudio_FfmpegEx3::Open() {

    MMP_RESULT mmpResult;
    
    mmpResult=CMmpDecoderAudio::Open();
    if(mmpResult!=MMP_SUCCESS)
    {
        return mmpResult;
    }

    mmpResult=CMmpDecoderFfmpegEx1::Open();
    if(mmpResult!=MMP_SUCCESS)
    {
        return mmpResult;
    }

    return MMP_SUCCESS;
}


MMP_RESULT CMmpDecoderAudio_FfmpegEx3::Close()
{
    MMP_RESULT mmpResult;
    //struct audio_decoded_packet* p_decoded_packet;

    mmpResult=CMmpDecoderFfmpegEx1::Close();
    if(mmpResult!=MMP_SUCCESS)
    {
        MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpDecoderAudio_FfmpegEx3::Close] CMmpDecoderFfmpeg::Close() \n\r")));
        return mmpResult;
    }

    mmpResult=CMmpDecoderAudio::Close();
    if(mmpResult!=MMP_SUCCESS)
    {
        MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpDecoderAudio_FfmpegEx3::Close] CMmpDecoderAudio::Close() \n\r")));
        return mmpResult;
    }

    
    //while(!m_queue_decoded.IsEmpty()) {
     //   
     //   m_queue_decoded.Delete(p_decoded_packet);
     //   delete [] p_decoded_packet->p_buffer;
     //   delete p_decoded_packet;
    //}

    if(m_pAVAudioResampleContext != NULL) {
        avresample_close(m_pAVAudioResampleContext);
        free(m_pAVAudioResampleContext);
        m_pAVAudioResampleContext = NULL;
    }

        
    return MMP_SUCCESS;
}

MMP_RESULT CMmpDecoderAudio_FfmpegEx3::AudioConvert_Create(AVCodecContext *pAVCodecContext) {

    MMP_RESULT mmpResult = MMP_FAILURE;
    int ret;

    if(m_pAVAudioResampleContext != NULL) {
        avresample_close(m_pAVAudioResampleContext);
        free(m_pAVAudioResampleContext);
        m_pAVAudioResampleContext = NULL;
    }

    if( (pAVCodecContext->sample_fmt >= 0) && (pAVCodecContext->channels > 0) && (pAVCodecContext->sample_rate!=0) ) {

        m_pAVAudioResampleContext = (struct AVAudioResampleContext*)malloc(sizeof(struct AVAudioResampleContext));

        memset(m_pAVAudioResampleContext, 0x00, sizeof(struct AVAudioResampleContext)); 

        m_pAVAudioResampleContext->in_channel_layout = pAVCodecContext->channel_layout;
        if(m_pAVAudioResampleContext->in_channel_layout == 0) {
           int i, j;
           switch(pAVCodecContext->channels) {
                case 1 :  m_pAVAudioResampleContext->in_channel_layout = AV_CH_LAYOUT_MONO; break;
                case 2 :  m_pAVAudioResampleContext->in_channel_layout = AV_CH_LAYOUT_STEREO; break;
                case 3 :  m_pAVAudioResampleContext->in_channel_layout = AV_CH_LAYOUT_SURROUND; break;
                case 4 :  m_pAVAudioResampleContext->in_channel_layout = AV_CH_LAYOUT_4POINT0; break;
                case 5 :  m_pAVAudioResampleContext->in_channel_layout = AV_CH_LAYOUT_5POINT0; break;
                case 6 :  m_pAVAudioResampleContext->in_channel_layout = AV_CH_LAYOUT_5POINT1; break;
                default: 
                    for( i = 0, j=1 ; i < pAVCodecContext->channels; i++, j<<=1 ) {
                        m_pAVAudioResampleContext->in_channel_layout |= j;   
                    }
           }
        }
        
        if(pAVCodecContext->channels == 1) {
            m_pAVAudioResampleContext->out_channel_layout = AV_CH_LAYOUT_MONO;
            this->set_channels(CH_MONO);
        }
        else {
            m_pAVAudioResampleContext->out_channel_layout = AV_CH_LAYOUT_STEREO;
            this->set_channels(CH_STEREO);
        }
        m_pAVAudioResampleContext->in_sample_rate = pAVCodecContext->sample_rate;
        m_pAVAudioResampleContext->out_sample_rate = pAVCodecContext->sample_rate;
        m_pAVAudioResampleContext->internal_sample_fmt = AV_SAMPLE_FMT_NONE;
        m_pAVAudioResampleContext->in_sample_fmt = pAVCodecContext->sample_fmt;
        m_pAVAudioResampleContext->out_sample_fmt = AV_SAMPLE_FMT_S16;

        ret = avresample_open(m_pAVAudioResampleContext);
        if(ret == 0) {
            mmpResult = MMP_SUCCESS;
        }
        else {
            avresample_close(m_pAVAudioResampleContext);
            free(m_pAVAudioResampleContext);
            m_pAVAudioResampleContext = NULL;
        }
    }

    return mmpResult;
}

MMP_RESULT CMmpDecoderAudio_FfmpegEx3::flush_buffer() {

    enum MMP_FOURCC fourcc_in = this->get_fourcc_in();

    m_last_pts = 0;

    return MMP_SUCCESS;
}

MMP_RESULT CMmpDecoderAudio_FfmpegEx3::DecodeDSI(class mmp_buffer_audiostream *p_buf_as) {

    MMP_RESULT mmpResult;

    mmpResult = CMmpDecoderFfmpegEx1::DecodeDSI(p_buf_as);
    if(mmpResult == MMP_SUCCESS) {

        this->set_fourcc_in(CMmpDecoderFfmpegEx1::GetFourccIn());

        this->AudioConvert_Create(m_pAVCodecContext);
     }

    return mmpResult;
}

MMP_RESULT CMmpDecoderAudio_FfmpegEx3::DecodeAu(class mmp_buffer_audiostream *p_buf_as, class mmp_buffer_audioframe *p_buf_af) {

    MMP_RESULT mmpResult = MMP_SUCCESS;

    int32_t frameFinished = 0;
    int32_t usebyte;
    AVPacket avpkt;
    MMP_U32 dec_start_tick, dec_end_tick;
    enum MMP_FOURCC fourcc_in = this->get_fourcc_in();

    dec_start_tick = CMmpUtil::GetTickCount();
        
    
    if(m_pAVCodec == NULL) {
        mmpResult = this->DecodeDSI(p_buf_as);
        if(mmpResult != MMP_SUCCESS) {
            p_buf_as->inc_stream_offset(p_buf_as->get_stream_real_size());
            return mmpResult;
        }
    }

    if((p_buf_as->get_flag()&mmp_buffer_media::FLAG_CONFIGDATA) != 0) {
        usebyte = (int32_t)p_buf_as->get_stream_real_size();
        p_buf_as->inc_stream_offset(usebyte);
        return MMP_SUCCESS;
    }

    p_buf_af->set_coding_result(mmp_buffer_media::FAIL);

    av_init_packet (&avpkt);
    avpkt.data = (uint8_t*)p_buf_as->get_stream_real_addr();
    avpkt.size = (int)p_buf_as->get_stream_real_size();
    avpkt.pts = p_buf_as->get_pts();

    MMPDEBUGMSG(0, (TEXT("[CMmpDecoderAudio_FfmpegEx3::DecodeAu] sample_cnt=%d sz=%d (0x%02x 0x%02x 0x%02x 0x%02x ) "), m_sample_count++, avpkt.size,  avpkt.data[0], avpkt.data[1], avpkt.data[2], avpkt.data[3]  ));

 
    usebyte = avcodec_decode_audio4(m_pAVCodecContext, m_pAVFrame_Decoded, &frameFinished, &avpkt);
    if(usebyte > 0) {
        p_buf_as->inc_stream_offset(usebyte);
    }
    else {
        p_buf_as->inc_stream_offset(avpkt.size);
    }
        
    /* If we use frame reference counting, we own the data and need
     * to de-reference it when we don't use it anymore */
#if 1
    if(frameFinished != 0) {

        int ch1, ch2;

        ch1 = m_pAVCodecContext->channels;
        ch2 = this->get_channels();
        if(ch1>CH_STEREO) ch1 = CH_STEREO;
        if(ch2>CH_STEREO) ch2 = CH_STEREO;
    
        /* if a frame has been decoded, output it */
        if( (m_pAVCodecContext->sample_rate != this->get_sample_rate()) 
            || (ch1 != ch2 ) 
            ) {

            if( (m_pAVCodecContext->sample_rate != 0)
                && (m_pAVCodecContext->channels != 0) 
                )
            {
                MMPDEBUGMSG(1, (TEXT("[CMmpDecoderAudio_FfmpegEx3::DecodeAu] Reconfig Occur!!  (%d %d) => (%d %d) "),
                    this->get_sample_rate(), this->get_channels(),
                    m_pAVCodecContext->sample_rate, m_pAVCodecContext->channels ));

                       
                this->set_sample_rate(m_pAVCodecContext->sample_rate);
                this->set_channels(m_pAVCodecContext->channels);

                //this->config(m_pAVCodecContext->sample_rate, m_pAVCodecContext->channels);
                this->AudioConvert_Create(m_pAVCodecContext);
                p_buf_af->set_coding_result(mmp_buffer_media::RECONFIG);
            }
        }
        else 
        {
            if(m_pAVAudioResampleContext == NULL) {
                this->AudioConvert_Create(m_pAVCodecContext);
            }
            
            //this->AudioConvert_Processing(p_buf_as->get_pts(), m_pAVFrame_Decoded, m_pAVCodecContext);

            if(p_buf_af->get_coding_result() != mmp_buffer_media::RECONFIG) {
                this->AudioConvert_ProcessingEx1(p_buf_as->get_pts(), m_pAVFrame_Decoded, m_pAVCodecContext, p_buf_af);
            }
        }

    }
#endif


/*
    if(p_buf_af->get_coding_result() != mmp_buffer_media::RECONFIG) {

        if(!m_queue_decoded.IsEmpty()) {
            
            struct audio_decoded_packet* p_decoded_packet;
            int buf_remain_size, buf_copy_size;
            MMP_U8* pOutBuffer = (MMP_U8*)p_buf_af->get_buf_vir_addr();

            m_queue_decoded.Delete(p_decoded_packet);
            buf_remain_size = p_decoded_packet->buf_size - p_decoded_packet->buf_index;
            if((int)p_buf_af->get_buf_size() >= buf_remain_size) {
                buf_copy_size = buf_remain_size;
            
                memcpy(pOutBuffer, p_decoded_packet->p_buffer, buf_copy_size);
            
                p_buf_af->set_data_size(buf_copy_size);
                p_buf_af->set_pts(p_decoded_packet->timestamp);
                
                p_decoded_packet->buf_index += buf_copy_size;
                p_buf_af->set_coding_result(mmp_buffer_media::SUCCESS);
            }
            else {
                MMPDEBUGMSG(1, (TEXT("[CMmpDecoderAudio_FfmpegEx3::DecodeAu] decoded buffer too small (%d %d"), p_buf_af->get_buf_size(), buf_remain_size));
            }
            free(p_decoded_packet->p_buffer);
            delete p_decoded_packet;
        }

    }
 */

    dec_end_tick = CMmpUtil::GetTickCount();
    
    p_buf_af->set_coding_dur(dec_end_tick - dec_start_tick);
    p_buf_af->set_samplerate(m_pAVCodecContext->sample_rate);
    p_buf_af->set_channel(m_pAVCodecContext->channels);

    //CMmpDecoderAudio::DecodeMonitor(p_buf_af);

	return MMP_SUCCESS;
}


void CMmpDecoderAudio_FfmpegEx3::AudioConvert_Processing(MMP_TICKS uiTimeStamp, AVFrame *pAVFrame_Decoded, AVCodecContext *pAVCodecContext) {

#if 0
    struct audio_decoded_packet* p_decoded_packet;
    int converted_sample_count;
    
    p_decoded_packet = new struct audio_decoded_packet;
    if(p_decoded_packet != NULL) {

        p_decoded_packet->buf_size = pAVFrame_Decoded->nb_samples * m_pAVAudioResampleContext->out_channels /*channel*/ * BYTE_PER_SAMPLE; /* ch * sample_byte */
        p_decoded_packet->buf_index = 0;
        p_decoded_packet->p_buffer = (MMP_U8*)malloc(p_decoded_packet->buf_size);

        if(p_decoded_packet->p_buffer != NULL ) {

            p_decoded_packet->timestamp = uiTimeStamp;

            converted_sample_count = avresample_convert(m_pAVAudioResampleContext,
                               &p_decoded_packet->p_buffer, 0, pAVFrame_Decoded->nb_samples,
                               pAVFrame_Decoded->data, 0, pAVFrame_Decoded->nb_samples);
            if(converted_sample_count == pAVFrame_Decoded->nb_samples) {
                p_decoded_packet->buf_size = pAVFrame_Decoded->nb_samples * m_pAVAudioResampleContext->out_channels/*channel*/ * BYTE_PER_SAMPLE/*byte_per_sample*/; 
                this->m_queue_decoded.Add(p_decoded_packet);
            }
            else {
                free(p_decoded_packet->p_buffer);
                delete p_decoded_packet;
            }
        }
    }
#endif
}

void CMmpDecoderAudio_FfmpegEx3::AudioConvert_ProcessingEx1(MMP_TICKS uiTimeStamp, AVFrame *pAVFrame_Decoded, AVCodecContext *pAVCodecContext, class mmp_buffer_audioframe *p_buf_af) {

    int converted_sample_count;
    MMP_S32 out_pcm_byte_size;
    MMP_U8 *pcm_data;
    
    
    out_pcm_byte_size = pAVFrame_Decoded->nb_samples * m_pAVAudioResampleContext->out_channels /*channel*/ * BYTE_PER_SAMPLE; /* ch * sample_byte */
    if(p_buf_af->get_buf_size() >= out_pcm_byte_size) {

        pcm_data = (MMP_U8*)p_buf_af->get_buf_vir_addr();
        
        converted_sample_count = avresample_convert(m_pAVAudioResampleContext,
                                                    &pcm_data, 0, pAVFrame_Decoded->nb_samples,
                                                    pAVFrame_Decoded->data, 0, pAVFrame_Decoded->nb_samples);

        if(converted_sample_count == pAVFrame_Decoded->nb_samples) {

            p_buf_af->set_data_size(out_pcm_byte_size);
            p_buf_af->set_pts(uiTimeStamp);
            p_buf_af->set_coding_result(mmp_buffer_media::SUCCESS);
        }

    }
}

#endif
