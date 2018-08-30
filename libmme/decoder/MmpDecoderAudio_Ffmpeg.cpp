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

#include "MmpDecoderAudio_Ffmpeg.hpp"
#include "MmpUtil.hpp"

#if (FFMPEG_VERSION == FFMPEG_VERSION_2_6_1)

/////////////////////////////////////////////////////////////
//CMmpDecoderAudio_Ffmpeg Member Functions

static const enum MMP_FOURCC s_support_fourcc_in[]={
     MMP_FOURCC_AUDIO_MP3,
     MMP_FOURCC_AUDIO_AAC,
     MMP_FOURCC_AUDIO_AC3,
     MMP_FOURCC_AUDIO_WMA1,
     MMP_FOURCC_AUDIO_WMA2,
     MMP_FOURCC_VIDEO_WMV1,
     MMP_FOURCC_VIDEO_WMV2,
     MMP_FOURCC_AUDIO_WMA_LOSSLESS,
     MMP_FOURCC_AUDIO_FFMPEG,
};

static const enum MMP_FOURCC s_support_fourcc_out[]={
     MMP_FOURCC_AUDIO_PCM
};


CMmpDecoderAudio_Ffmpeg::CMmpDecoderAudio_Ffmpeg(struct CMmpDecoderAudio::create_config *p_create_config) :

CMmpDecoderAudio(p_create_config, "Ffmpeg",
                 s_support_fourcc_in, sizeof(s_support_fourcc_in)/sizeof(s_support_fourcc_in[0]),
                 s_support_fourcc_out, sizeof(s_support_fourcc_out)/sizeof(s_support_fourcc_out[0])
                 )

,CMmpDecoderFfmpeg(p_create_config->fourcc_in)

,m_reconfig_count(0)
,m_pAudioConvert(NULL)
,m_queue_decoded(60)
{

}

CMmpDecoderAudio_Ffmpeg::~CMmpDecoderAudio_Ffmpeg()
{

}

MMP_RESULT CMmpDecoderAudio_Ffmpeg::Open() {

    MMP_RESULT mmpResult;

    mmpResult=CMmpDecoderAudio::Open();
    if(mmpResult!=MMP_SUCCESS)
    {
        return mmpResult;
    }

    mmpResult=CMmpDecoderFfmpeg::Open();
    if(mmpResult!=MMP_SUCCESS)
    {
        return mmpResult;
    }

    return MMP_SUCCESS;
}


MMP_RESULT CMmpDecoderAudio_Ffmpeg::Close()
{
    MMP_RESULT mmpResult;
    struct audio_decoded_packet* p_decoded_packet;

    mmpResult=CMmpDecoderFfmpeg::Close();
    if(mmpResult!=MMP_SUCCESS)
    {
        MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpDecoderAudio_Ffmpeg::Close] CMmpDecoderFfmpeg::Close() \n\r")));
        return mmpResult;
    }

    mmpResult=CMmpDecoderAudio::Close();
    if(mmpResult!=MMP_SUCCESS)
    {
        MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpDecoderAudio_Ffmpeg::Close] CMmpDecoderAudio::Close() \n\r")));
        return mmpResult;
    }


    while(!m_queue_decoded.IsEmpty()) {

        m_queue_decoded.Delete(p_decoded_packet);
        delete [] p_decoded_packet->p_buffer;
        delete p_decoded_packet;
    }

    if(m_pAudioConvert != NULL) {
        ff_audio_convert_free(&m_pAudioConvert);
        m_pAudioConvert = NULL;
    }

    return MMP_SUCCESS;
}

MMP_RESULT CMmpDecoderAudio_Ffmpeg::AudioConvert_Create(AVCodecContext *pAVCodecContext) {

    MMP_RESULT mmpResult = MMP_SUCCESS;

    memset(&m_AVAudioResampleContext, 0x00, sizeof(struct AVAudioResampleContext));
    m_AVAudioResampleContext.dither_method = AV_RESAMPLE_DITHER_NONE;
    m_AVAudioResampleContext.av_class = NULL;

    if(m_pAudioConvert != NULL) {
        ff_audio_convert_free(&m_pAudioConvert);
        m_pAudioConvert = NULL;
    }

    if( (pAVCodecContext->sample_fmt >= 0) && (pAVCodecContext->channels > 0) && (pAVCodecContext->sample_rate!=0) ) {
    
        m_pAudioConvert = ff_audio_convert_alloc(&m_AVAudioResampleContext, 
                                                AV_SAMPLE_FMT_S16, 
                                                pAVCodecContext->sample_fmt, 
                                                pAVCodecContext->channels, 
                                                pAVCodecContext->sample_rate, 0);
    }

    if(m_pAudioConvert == NULL ) {
        mmpResult = MMP_FAILURE;
    }

    return mmpResult;
}

MMP_RESULT CMmpDecoderAudio_Ffmpeg::DecodeDSI(class mmp_buffer_audiostream *p_buf_as) {

    MMP_RESULT mmpResult;

    mmpResult = CMmpDecoderFfmpeg::DecodeDSI(p_buf_as);
    if(mmpResult == MMP_SUCCESS) {
        this->AudioConvert_Create(m_pAVCodecContext);
     }

    return mmpResult;
}

MMP_RESULT CMmpDecoderAudio_Ffmpeg::DecodeAu(class mmp_buffer_audiostream *p_buf_as, class mmp_buffer_audioframe *p_buf_af) {

    MMP_RESULT mmpResult = MMP_SUCCESS;

    int32_t frameFinished = 192000 * 2;
    int32_t usebyte;
    AVPacket avpkt;
    MMP_U32 dec_start_tick, dec_end_tick;

    dec_start_tick = CMmpUtil::GetTickCount();
        
    
    if(m_pAVCodec == NULL) {
        mmpResult = this->DecodeDSI(p_buf_as);
        if(mmpResult != MMP_SUCCESS) {
            p_buf_as->inc_stream_offset(p_buf_as->get_stream_real_size());
            return mmpResult;
        }
    }

    if((p_buf_as->get_flag()&mmp_buffer_media::FLAG_CONFIGDATA) != 0) {
        return MMP_SUCCESS;
    }

    av_init_packet (&avpkt);
    avpkt.data = (uint8_t*)p_buf_as->get_stream_real_addr();
    avpkt.size = (int)p_buf_as->get_stream_real_size();
    avpkt.pts = p_buf_as->get_pts();

    avcodec_get_frame_defaults(m_pAVFrame_Decoded);
    usebyte = avcodec_decode_audio4(m_pAVCodecContext, m_pAVFrame_Decoded, &frameFinished, &avpkt);
    if(usebyte > 0) {
        p_buf_as->inc_stream_offset(usebyte);
    }
    else {
        p_buf_as->inc_stream_offset(avpkt.size);
        mmpResult = MMP_FAILURE;
    }

    if(frameFinished != 0) {

        int ch1, ch2;

        ch1 = m_pAVCodecContext->channels;
        ch2 = this->get_ch();
        if(ch1>2) ch1 = 2;
        if(ch2>2) ch2 = 2;
    
        /* if a frame has been decoded, output it */
        if( (m_pAVCodecContext->sample_rate != this->get_sr()) 
            || (ch1 != ch2 ) 
            ) {

            if( (m_pAVCodecContext->sample_rate != 0)
                && (m_pAVCodecContext->channels != 0) 
                )
            {
                MMPDEBUGMSG(1, (TEXT("[CMmpDecoderAudio_Ffmpeg::DecodeAu] Reconfig Occur!!  (%d %d) => (%d %d) "),
                    this->get_sr(), this->get_ch(),
                    m_pAVCodecContext->sample_rate, m_pAVCodecContext->channels ));

                       
                this->set_sr(m_pAVCodecContext->sample_rate);
                this->set_ch(m_pAVCodecContext->channels);

                //this->config(m_pAVCodecContext->sample_rate, m_pAVCodecContext->channels);
                this->AudioConvert_Create(m_pAVCodecContext);
                p_buf_af->set_coding_result(mmp_buffer_media::RECONFIG);
            }
        }
        else 
        {

            if(m_pAudioConvert == NULL) {
                this->AudioConvert_Create(m_pAVCodecContext);
            }
            this->AudioConvert_Processing(p_buf_as->get_pts(), m_pAVFrame_Decoded, m_pAVCodecContext);
        }

    }

#if 1
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
                MMPDEBUGMSG(1, (TEXT("[CMmpDecoderAudio_Ffmpeg::DecodeAu] decoded buffer too small (%d %d"), p_buf_af->get_buf_size(), buf_remain_size));
            }
            free(p_decoded_packet->p_buffer);
            delete p_decoded_packet;
        }

    }

    dec_end_tick = CMmpUtil::GetTickCount();
    
    p_buf_af->set_coding_dur(dec_end_tick - dec_start_tick);
    p_buf_af->set_samplerate(m_pAVCodecContext->sample_rate);
    p_buf_af->set_channel(m_pAVCodecContext->channels);
#endif
    //CMmpDecoderAudio::DecodeMonitor(p_buf_af);

	return MMP_SUCCESS;
}

#if 1

void CMmpDecoderAudio_Ffmpeg::AudioConvert_Processing(MMP_TICKS uiTimeStamp, AVFrame *pAVFrame_Decoded, AVCodecContext *pAVCodecContext) {

    AudioData out, in;
    struct audio_decoded_packet* p_decoded_packet;
    int iret;

    //uint8_t* in_buf = new uint8_t[1024*1024];
    //uint8_t* out_buf = new uint8_t[1024*1024];

    p_decoded_packet = new struct audio_decoded_packet;
    if(p_decoded_packet != NULL) {

        p_decoded_packet->buf_size = pAVFrame_Decoded->nb_samples * m_pAVCodecContext->channels * 2; /* ch * sample_byte */
        p_decoded_packet->buf_index = 0;
        p_decoded_packet->p_buffer = (MMP_U8*)malloc(p_decoded_packet->buf_size);

        if(p_decoded_packet->p_buffer != NULL ) {

            p_decoded_packet->timestamp = uiTimeStamp;
            ff_audio_data_init(&in, pAVFrame_Decoded->data, 0, m_pAVCodecContext->channels, pAVFrame_Decoded->nb_samples, pAVCodecContext->sample_fmt, 1 /*read_only*/, NULL);
            ff_audio_data_init(&out, &p_decoded_packet->p_buffer, 0, m_pAVCodecContext->channels, pAVFrame_Decoded->nb_samples, AV_SAMPLE_FMT_S16, 1 /*read_only*/, NULL);
            
            if(m_pAudioConvert != NULL) {
            
                iret = ff_audio_convert(m_pAudioConvert, &out, &in);
                
                if(iret == 0) {

                    if(this->get_ch() < m_pAVCodecContext->channels) {

                        int isam, left_index=0, right_index=1;

                        short* sa_src = (short*)p_decoded_packet->p_buffer;
                        short* sa_dest = (short*)p_decoded_packet->p_buffer;;

                        left_index = 0;
                        right_index = 1;
                        if(m_pAVCodecContext->codec_id == AV_CODEC_ID_AC3) {
                            //AC3  Front: L C R   Side: L R LFE 
                            //if(pAVFrame_Decoded->nb_samples == 6) {
                            //    left_index = 3;
                            //    right_index = 4;
                            //}
                            //else 
                            if(pAVFrame_Decoded->nb_samples >= 3) {
                                left_index = 0;
                                right_index = 2;
                            }
                        }
                        
                        for(isam=0; isam < pAVFrame_Decoded->nb_samples; isam++) {

                            sa_dest[0] = sa_src[left_index];
                            sa_dest[1] = sa_src[right_index];
                            
                            sa_dest += this->get_ch();
                            sa_src += m_pAVCodecContext->channels;
                        }
                        p_decoded_packet->buf_size = pAVFrame_Decoded->nb_samples * this->get_ch() * this->get_bps()/8; /* ch * sample_byte */
                    }

                    this->m_queue_decoded.Add(p_decoded_packet);
                }
                else {
                    free(p_decoded_packet->p_buffer);
                    delete p_decoded_packet;
                }
            }
            else {
                
                free(p_decoded_packet->p_buffer);
                delete p_decoded_packet;
            }

        }

    }
}

#else
void CMmpDecoderAudio_Ffmpeg::PostProcessing(AVFrame *pAVFrame_Decoded, AVCodecContext *pAVCodecContext) {

    AudioConvert *ac;
    struct AVAudioResampleContext avr;
    AudioData out, in;
    struct audio_decoded_packet* p_decoded_packet;
    int iret;

    //uint8_t* in_buf = new uint8_t[1024*1024];
    //uint8_t* out_buf = new uint8_t[1024*1024];

    p_decoded_packet = new struct audio_decoded_packet;
    p_decoded_packet->buf_size = pAVFrame_Decoded->nb_samples * 2 * 2;
    p_decoded_packet->buf_index = 0;
    p_decoded_packet->p_buffer = new MMP_U8[p_decoded_packet->buf_size];

    memset(&avr, 0x00, sizeof(struct AVAudioResampleContext));
    avr.dither_method = AV_RESAMPLE_DITHER_NONE;
    avr.av_class = NULL;

    //ff_audio_data_init(&in, &pAVFrame_Decoded->data[0], 0, m_pAVCodecContext->channels, pAVFrame_Decoded->nb_samples, AV_SAMPLE_FMT_FLTP, 1, NULL);
    ff_audio_data_init(&in, &pAVFrame_Decoded->data[0], 0, m_pAVCodecContext->channels, pAVFrame_Decoded->nb_samples, pAVCodecContext->sample_fmt, 1 /*read_only*/, NULL);
    ff_audio_data_init(&out, &p_decoded_packet->p_buffer, 0, m_pAVCodecContext->channels, pAVFrame_Decoded->nb_samples, AV_SAMPLE_FMT_S16, 0, NULL);

    ac = ff_audio_convert_alloc(&avr, AV_SAMPLE_FMT_S16, AV_SAMPLE_FMT_FLTP, m_pAVCodecContext->channels, m_pAVCodecContext->sample_rate, 0);
    if(ac != NULL) {
    
        iret = ff_audio_convert(ac, &out, &in);
        ff_audio_convert_free(&ac);

        if(iret == 0) {
            this->m_queue_decoded.Add(p_decoded_packet);
        }
        else {
            delete p_decoded_packet->p_buffer;
            delete p_decoded_packet;
        }
    }
}

#endif
#endif