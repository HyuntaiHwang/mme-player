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

#include "MmpDemuxer_FfmpegEx1.hpp"
#include "MmpUtil.hpp"
#include "mmp_audio_tool.hpp"

#if (FFMPEG_VERSION == FFMPEG_VERSION_n3_1_1)

/////////////////////////////////////////////////////
// class

CMmpDemuxer_FfmpegEx1::CMmpDemuxer_FfmpegEx1(struct MmpDemuxerCreateConfig* pCreateConfig) : CMmpDemuxer(pCreateConfig)
,m_pAvformatCtx(NULL)
,m_iFileSize(0)
,m_nFileBufferIndex(0)
,m_nFileBufferSize(0)
,m_last_seek_pts(0)
{
    int i;

    av_register_all();

    for(i = 0; i < MMP_MEDIATYPE_MAX; i++) {
        m_nStreamIndex[i] = -1;
    }
}

CMmpDemuxer_FfmpegEx1::~CMmpDemuxer_FfmpegEx1()
{

}


static int open_codec_context(int *stream_idx,
                              AVFormatContext *fmt_ctx, enum AVMediaType type)
{
    int ret;
    //int stream_index;
    //AVStream *st;
    //AVCodecContext *dec_ctx = NULL;
    //AVCodec *dec = NULL;
    //AVDictionary *opts = NULL;

    ret = av_find_best_stream(fmt_ctx, type, -1, -1, NULL, 0);
    if (ret < 0) {
        //fprintf(stderr, "Could not find %s stream in input file '%s'\n",
        //        av_get_media_type_string(type), src_filename);
        return ret;
    } else {
#if 0
        stream_index = ret;
        st = fmt_ctx->streams[stream_index];

        /* find decoder for the stream */
        dec_ctx = st->codec;
        dec = avcodec_find_decoder(dec_ctx->codec_id);
        if (!dec) {
            fprintf(stderr, "Failed to find %s codec codec-id = 0x%x \n",
                    av_get_media_type_string(type), dec_ctx->codec_id);
            return AVERROR(EINVAL);
        }

        /* Init the decoders, with or without reference counting */
        int refcount = 0; //add by hthwang
        av_dict_set(&opts, "refcounted_frames", refcount ? "1" : "0", 0);
        if ((ret = avcodec_open2(dec_ctx, dec, &opts)) < 0) {
            fprintf(stderr, "Failed to open %s codec\n",
                    av_get_media_type_string(type));
            return ret;
        }
        *stream_idx = stream_index;
#else
        *stream_idx = ret;
#endif
    }

    return 0;
}

MMP_RESULT CMmpDemuxer_FfmpegEx1::Open()
{
    MMP_RESULT mmpResult = MMP_SUCCESS;

    MMP_S32 idx;
    AVInputFormat *fmt = NULL;
    

	MMPDEBUGMSG(1, (TEXT("[CMmpDemuxer_FfmpegEx1::Open] Init Demuxer ... Wait for about 1sec")));

    if (avformat_open_input(&m_pAvformatCtx, (const char*)this->m_create_config.filename, NULL, NULL) < 0) {
        //fprintf(stderr, "Could not open source file %s\n", src_filename);
        //exit(1);
        mmpResult = MMP_FAILURE;
    }
    
    /* retrieve stream information */
    if(mmpResult == MMP_SUCCESS) {
        if (avformat_find_stream_info(m_pAvformatCtx, NULL) < 0) {
            //fprintf(stderr, "Could not find stream information\n");
            //exit(1);
            mmpResult = MMP_FAILURE;
        }
    }

     if(mmpResult == MMP_SUCCESS) {   
        if (open_codec_context((int*)&idx, m_pAvformatCtx, AVMEDIA_TYPE_VIDEO) >= 0) {
            m_nStreamIndex[MMP_MEDIATYPE_VIDEO] = idx;    
        }
      }

    if(mmpResult == MMP_SUCCESS) {   
        if (open_codec_context((int*)&idx, m_pAvformatCtx, AVMEDIA_TYPE_AUDIO) >= 0) {
            m_nStreamIndex[MMP_MEDIATYPE_AUDIO] = idx;
        }
     }

    av_init_packet(&m_pkt);
    m_pkt.data = NULL;
    m_pkt.size = 0;

    return mmpResult;
}

MMP_RESULT CMmpDemuxer_FfmpegEx1::Close()
{
    
    if(m_pAvformatCtx != NULL)
    {
        MMP_S32 idx;
        static AVCodecContext *codec_ctx;

        MMPDEBUGMSG(1, (TEXT("[CMmpDemuxer_FfmpegEx1::Close] ln=%d "), __LINE__));
        idx = m_nStreamIndex[MMP_MEDIATYPE_VIDEO];
        if(idx >= 0) {
            if(m_pAvformatCtx->streams[idx] != NULL) {
                codec_ctx = m_pAvformatCtx->streams[idx]->codec;
                MMPDEBUGMSG(1, (TEXT("[CMmpDemuxer_FfmpegEx1::Close] ln=%d "), __LINE__));
                if(codec_ctx!=NULL) avcodec_close(codec_ctx);
            }
        }

        idx = m_nStreamIndex[MMP_MEDIATYPE_AUDIO];
        MMPDEBUGMSG(1, (TEXT("[CMmpDemuxer_FfmpegEx1::Close] ln=%d  idx=%d "), __LINE__,  idx));
        if(idx >= 0) {
            if(m_pAvformatCtx->streams[idx] != NULL) {
                codec_ctx = m_pAvformatCtx->streams[idx]->codec;
                MMPDEBUGMSG(1, (TEXT("[CMmpDemuxer_FfmpegEx1::Close] ln=%d "), __LINE__));
                if(codec_ctx!=NULL) avcodec_close(codec_ctx);
            }
        }

        avformat_close_input(&m_pAvformatCtx);
        m_pAvformatCtx = NULL;
    }
        
    
    return MMP_SUCCESS;
}

MMP_S64 CMmpDemuxer_FfmpegEx1::GetDuration() {

    int i;
    AVStream *s;
    AVCodecContext *cc;
    MMP_S32 stream_index;
    int64_t dur, keydur = 0;
    
    for(i = 0; i < MMP_MEDIATYPE_MAX; i++) {

        stream_index = m_nStreamIndex[i];
        if(stream_index >= 0) {

            s = m_pAvformatCtx->streams[stream_index];
            cc = s->codec;

            if(s->duration == AV_NOPTS_VALUE) {
                dur = m_pAvformatCtx->duration;
                keydur = dur;
            }
            else {
                dur = s->duration;
                keydur = (dur * 1000000) * s->time_base.num / s->time_base.den;
            }
            
            break;
        }
    }

    return keydur;
}

enum MMP_FOURCC CMmpDemuxer_FfmpegEx1::GetAudioFourcc() {

    AVStream *s;
    AVCodecContext *cc;
    MMP_S32 stream_index;
    enum MMP_FOURCC fourcc = MMP_FOURCC_AUDIO_UNKNOWN;

    stream_index = m_nStreamIndex[MMP_MEDIATYPE_AUDIO];
    if(stream_index >= 0) {
        s = m_pAvformatCtx->streams[stream_index];
        cc = s->codec;

        switch(cc->codec_id) {
        
            case AV_CODEC_ID_MP3:
                fourcc = MMP_FOURCC_AUDIO_MP3;
                break;

            case AV_CODEC_ID_MP2:
                fourcc = MMP_FOURCC_AUDIO_MP2;
                break;
            
            case AV_CODEC_ID_AAC:
                fourcc = MMP_FOURCC_AUDIO_AAC;
                break;

            case AV_CODEC_ID_WMAV1:
                fourcc = MMP_FOURCC_AUDIO_WMA1;
                break;

            case AV_CODEC_ID_WMAV2:
                fourcc = MMP_FOURCC_AUDIO_WMA2;
                break;

            case AV_CODEC_ID_WMALOSSLESS:
                fourcc = MMP_FOURCC_AUDIO_WMA_LOSSLESS;
                break;

            case AV_CODEC_ID_WMAPRO:
                fourcc = MMP_FOURCC_AUDIO_WMA_PRO;
                break;

            case AV_CODEC_ID_WMAVOICE:
                fourcc = MMP_FOURCC_AUDIO_WMA_VOICE;
                break;

            case AV_CODEC_ID_AC3:
                fourcc = MMP_FOURCC_AUDIO_AC3;
                break;

            case AV_CODEC_ID_AMR_NB:
                fourcc = MMP_FOURCC_AUDIO_FFMPEG;
                break;

            case AV_CODEC_ID_FLAC:
                fourcc = MMP_FOURCC_AUDIO_FLAC;
                break;

            case AV_CODEC_ID_PCM_S16LE:
                fourcc = MMP_FOURCC_AUDIO_CODEC_PCM_S16LE;
                break;

        }
    }

    return fourcc;
}

MMP_U32 CMmpDemuxer_FfmpegEx1::GetAudioChannel() {
    
    MMP_U32 retvalue = 0;
    AVStream *s;
    AVCodecContext *cc;
    MMP_S32 stream_index;
    MMP_U32 format = MMP_FOURCC_VIDEO_UNKNOWN;

    stream_index = m_nStreamIndex[MMP_MEDIATYPE_AUDIO];

    if(stream_index >= 0) {
        s = m_pAvformatCtx->streams[stream_index];
        cc = s->codec;

        retvalue = cc->channels;
        if(retvalue > 5) {
            retvalue = 2;
        }
    }

    return retvalue;
}
    
MMP_U32 CMmpDemuxer_FfmpegEx1::GetAudioSamplingRate() {

    MMP_U32 retvalue = 0;
    AVStream *s;
    AVCodecContext *cc;
    MMP_S32 stream_index;
    MMP_U32 format = MMP_FOURCC_VIDEO_UNKNOWN;

    stream_index = m_nStreamIndex[MMP_MEDIATYPE_AUDIO];

    if(stream_index >= 0) {
        s = m_pAvformatCtx->streams[stream_index];
        cc = s->codec;

        retvalue = cc->sample_rate;
    }

    return retvalue;
}

MMP_U32 CMmpDemuxer_FfmpegEx1::GetAudioBitsPerSample() {

    MMP_U32 retvalue = 0;
    AVStream *s;
    AVCodecContext *cc;
    MMP_S32 stream_index;
    MMP_U32 format = MMP_FOURCC_VIDEO_UNKNOWN;

    stream_index = m_nStreamIndex[MMP_MEDIATYPE_AUDIO];

    if(stream_index >= 0) {
        s = m_pAvformatCtx->streams[stream_index];
        cc = s->codec;

        retvalue = cc->bits_per_coded_sample;
        if(retvalue == 0) {
            retvalue = 16;    
        }
    }

    return retvalue;
}

MMP_S32 CMmpDemuxer_FfmpegEx1::GetAudioBitrate() {

    MMP_U32 retvalue = 0;
    AVStream *s;
    AVCodecContext *cc;
    MMP_S32 stream_index;
    MMP_U32 format = MMP_FOURCC_VIDEO_UNKNOWN;

    stream_index = m_nStreamIndex[MMP_MEDIATYPE_AUDIO];

    if(stream_index >= 0) {
        s = m_pAvformatCtx->streams[stream_index];
        cc = s->codec;

        retvalue = cc->bit_rate;
    }

    return retvalue;
}

struct mmp_audio_format CMmpDemuxer_FfmpegEx1::get_audio_format() {
    
    MMP_U32 retvalue = 0;
    AVStream *s;
    AVCodecContext *cc;
    MMP_S32 stream_index;
    MMP_U32 format = MMP_FOURCC_VIDEO_UNKNOWN;

    struct mmp_audio_format af;
    struct mmp_audio_format *p_af = &af;

    stream_index = m_nStreamIndex[MMP_MEDIATYPE_AUDIO];
    if( (m_pAvformatCtx != NULL) && (stream_index >= 0) ) {

        s = m_pAvformatCtx->streams[stream_index];
        cc = s->codec;

        p_af->key = MMP_AUDIO_FORMAT_KEY;
        p_af->fourcc = this->GetAudioFourcc();

        p_af->sample_rate = ff_get_codec_param_sample_rate(s);
        p_af->channels = ff_get_codec_param_channels(s);
        p_af->bits_per_sample = ff_get_codec_param_bits_per_raw_sample(s);
        if(p_af->bits_per_sample == 0) {
            p_af->bits_per_sample = ff_get_codec_param_bits_per_coded_sample(s);
        }
        
        p_af->bitrate = ff_get_codec_param_bit_rate(s);
        p_af->block_align = ff_get_codec_param_block_align(s);
        p_af->frame_size = ff_get_codec_param_frame_size(s);
        p_af->codec_tag = ff_get_codec_param_codec_tag(s);

        p_af->extra_data_size = cc->extradata_size;
        if(cc->extradata_size <= MMP_AUDIO_FORMAT_MAX_EXTRA_DATA_SIZE) {
            memcpy(p_af->extra_data, cc->extradata, cc->extradata_size);
        }
    }
    else {
        af.fourcc = MMP_FOURCC_AUDIO_UNKNOWN;
    }

    return af;
}

MMP_S32 CMmpDemuxer_FfmpegEx1::GetVideoBitrate() {

    MMP_U32 retvalue = 0;
    AVStream *s;
    AVCodecContext *cc;
    MMP_S32 stream_index;
    MMP_U32 format = MMP_FOURCC_VIDEO_UNKNOWN;

    stream_index = m_nStreamIndex[MMP_MEDIATYPE_VIDEO];

    if(stream_index >= 0) {
        s = m_pAvformatCtx->streams[stream_index];
        cc = s->codec;

        retvalue = cc->bit_rate;
    }

    return retvalue;
}

MMP_S32 CMmpDemuxer_FfmpegEx1::GetVideoFPS() {

    MMP_U32 retvalue = 0;
    AVStream *s;
    AVCodecContext *cc;
    MMP_S32 stream_index;
    MMP_U32 format = MMP_FOURCC_VIDEO_UNKNOWN;
    MMP_S32 fps = 0;

    stream_index = m_nStreamIndex[MMP_MEDIATYPE_VIDEO];

    if(stream_index >= 0) {
        s = m_pAvformatCtx->streams[stream_index];
        cc = s->codec;

        if(s->r_frame_rate.den != 0) {
            fps = s->r_frame_rate.num / s->r_frame_rate.den;
        }
    }

    return fps;
}
    
enum MMP_FOURCC CMmpDemuxer_FfmpegEx1::GetVideoFourcc() {

    AVStream *s;
    AVCodecContext *cc;
    MMP_S32 stream_index;
    enum MMP_FOURCC fourcc = MMP_FOURCC_VIDEO_UNKNOWN;

    stream_index = m_nStreamIndex[MMP_MEDIATYPE_VIDEO];

    if(stream_index >= 0) {
        s = m_pAvformatCtx->streams[stream_index];
        cc = s->codec;

        switch(cc->codec_id) {
        
            case AV_CODEC_ID_H263:  fourcc = MMP_FOURCC_VIDEO_H263; break;
            case AV_CODEC_ID_H264:  fourcc = MMP_FOURCC_VIDEO_H264; break;
            
            /* VPU를 위해서는 아래 4가본 MPEG4, Div5(divx5.0 or higher), xvid, divx  구분 필요*/
            case AV_CODEC_ID_MPEG4: fourcc = MMP_FOURCC_VIDEO_MPEG4;  break;

            case AV_CODEC_ID_MPEG2VIDEO : fourcc = MMP_FOURCC_VIDEO_MPEG2;  break;
            case AV_CODEC_ID_MJPEG : fourcc = MMP_FOURCC_VIDEO_MJPEG;  break;

            /* Microsoft Codec */
            case AV_CODEC_ID_WMV1: fourcc = MMP_FOURCC_VIDEO_WMV1; break;
            case AV_CODEC_ID_WMV2: fourcc = MMP_FOURCC_VIDEO_WMV2; break;
            case AV_CODEC_ID_WMV3: fourcc = MMP_FOURCC_VIDEO_WMV3; break; 
            case AV_CODEC_ID_VC1:  
                fourcc = MMP_FOURCC_VIDEO_WVC1; 
                //fourcc = MMP_FOURCC_VIDEO_WMV3; 
                break;

            case AV_CODEC_ID_MSMPEG4V2: fourcc = MMP_FOURCC_VIDEO_MSMPEG4V2; break;
            case AV_CODEC_ID_MSMPEG4V3: fourcc = MMP_FOURCC_VIDEO_MSMPEG4V3; break;
            case AV_CODEC_ID_MSS1: fourcc = MMP_FOURCC_VIDEO_MSS1;  break;
            case AV_CODEC_ID_MSS2: fourcc = MMP_FOURCC_VIDEO_MSS2;  break;

            /* RV */
            case AV_CODEC_ID_RV30: fourcc = MMP_FOURCC_VIDEO_RV30; break;
            case AV_CODEC_ID_RV40: fourcc = MMP_FOURCC_VIDEO_RV40; break;

            /* VP 6/7/8 */
            case AV_CODEC_ID_VP8:  fourcc = MMP_FOURCC_VIDEO_VP80; break;
            case AV_CODEC_ID_VP6:  fourcc = MMP_FOURCC_VIDEO_VP60; break;
            case AV_CODEC_ID_VP6F: fourcc = MMP_FOURCC_VIDEO_VP6F; break;
            case AV_CODEC_ID_VP6A: fourcc = MMP_FOURCC_VIDEO_VP6A; break;

            /* Etc */
            case AV_CODEC_ID_SVQ3: fourcc = MMP_FOURCC_VIDEO_SVQ3; break;
            case AV_CODEC_ID_THEORA: fourcc = MMP_FOURCC_VIDEO_THEORA; break;
            case AV_CODEC_ID_FLV1: fourcc = MMP_FOURCC_VIDEO_FLV1; break;
            case AV_CODEC_ID_INDEO2: fourcc = MMP_FOURCC_VIDEO_INDEO2; break;
            case AV_CODEC_ID_INDEO3: fourcc = MMP_FOURCC_VIDEO_INDEO3; break;
            case AV_CODEC_ID_INDEO4: fourcc = MMP_FOURCC_VIDEO_INDEO4; break;
            case AV_CODEC_ID_INDEO5: fourcc = MMP_FOURCC_VIDEO_INDEO5; break;
            case AV_CODEC_ID_TSCC:   fourcc = MMP_FOURCC_VIDEO_TSCC; break;
            
            default:
                fourcc = MMP_FOURCC_VIDEO_UNKNOWN;
        }
        

    }

    return fourcc;
}

MMP_U32 CMmpDemuxer_FfmpegEx1::GetVideoPicWidth() {

    AVStream *s;
    AVCodecContext *cc;
    MMP_S32 stream_index;
    MMP_U32 width = 0;

    stream_index = m_nStreamIndex[MMP_MEDIATYPE_VIDEO];

    if(stream_index >= 0) {
        s = m_pAvformatCtx->streams[stream_index];
        cc = s->codec;

        width = cc->width;

    }

    return width;
}

MMP_U32 CMmpDemuxer_FfmpegEx1::GetVideoPicHeight() {

    AVStream *s;
    AVCodecContext *cc;
    MMP_S32 stream_index;
    MMP_U32 height = 0;

    stream_index = m_nStreamIndex[MMP_MEDIATYPE_VIDEO];

    if(stream_index >= 0) {
        s = m_pAvformatCtx->streams[stream_index];
        cc = s->codec;

        height = cc->height;
    }

    return height;
}

MMP_RESULT CMmpDemuxer_FfmpegEx1::GetVideoExtraData(MMP_U8* buffer, MMP_U32 buf_max_size, MMP_U32* buf_size)  {

    return MMP_FAILURE;
}

MMP_RESULT CMmpDemuxer_FfmpegEx1::GetMediaExtraData(MMP_U32 mediatype, MMP_U8* buffer, MMP_U32 buf_max_size, MMP_U32* buf_size)  {

    return MMP_FAILURE;
}

MMP_RESULT CMmpDemuxer_FfmpegEx1::GetAudioExtraData(class mmp_buffer_audiostream* p_buf_audiostream) {

    MMP_RESULT mmpResult = MMP_FAILURE;
    AVStream *s;
    AVCodecContext *cc;
    MMP_S32 stream_index;
    MMP_U8* buffer;
    MMP_S32 frame_rate = 0;
    enum MMP_FOURCC fourcc;

    struct mmp_audio_format af;
    struct mmp_audio_format* p_af;
    android_adsp_codec_wma_t* p_wma;
    android_adsp_codec_flac_t* p_flac;

    uint8_t *extra_ffmpeg, *pdata;
    uint32_t key;
    struct mmp_audio_packet_header* p_audio_pkt_hdr;

    
    p_buf_audiostream->set_stream_size(0);
    buffer = (MMP_U8*)p_buf_audiostream->get_buf_vir_addr();
    stream_index = m_nStreamIndex[MMP_MEDIATYPE_AUDIO];

    fourcc = this->GetAudioFourcc();

    if(stream_index >= 0) {

        switch(fourcc) {
            case MMP_FOURCC_AUDIO_FFMPEG:
            
                s = m_pAvformatCtx->streams[stream_index];
                cc = s->codec;

                p_buf_audiostream->set_stream_size(sizeof(AVCodecContext) + cc->extradata_size + 1024);
                p_buf_audiostream->set_stream_offset(0);
                p_buf_audiostream->set_flag(mmp_buffer_media::FLAG_CONFIGDATA);

                        
                extra_ffmpeg = (uint8_t *)p_buf_audiostream->get_buf_vir_addr();
                p_audio_pkt_hdr = (struct mmp_audio_packet_header*)extra_ffmpeg;

                mmp_audio_tool::make_audio_packet_header(cc->extradata, cc->extradata_size,
                                                        cc, sizeof(AVCodecContext), 
                                                        (MMP_ADDR)extra_ffmpeg);

                break;
            
            case MMP_FOURCC_AUDIO_WMA2: 
                p_wma = (android_adsp_codec_wma_t*)buffer;
                s = m_pAvformatCtx->streams[stream_index];
                cc = s->codec;

                af = this->get_audio_format();

                p_wma->i_samp_freq = af.sample_rate;
                p_wma->i_num_chan = af.channels;
                p_wma->i_pcm_wd_sz = af.bits_per_sample;
                p_wma->i_w_fmt_tag = af.codec_tag;
                p_wma->i_blk_align = af.block_align;
                p_wma->i_avg_byte = af.bitrate/8;
                p_wma->extra_data_size = af.extra_data_size;
                if(p_wma->extra_data_size > 12) {
                   p_wma->extra_data_size = 0;
                }
                if(p_wma->extra_data_size > 0) {
                    memcpy(p_wma->extra_data, af.extra_data, p_wma->extra_data_size);
                }

                p_wma->i_encode_opt = 0;
                if(p_wma->extra_data_size >= 6) {
                    unsigned short* sp = (unsigned short*)p_wma->extra_data;
                    p_wma->i_encode_opt = sp[2];
                }
                
                p_buf_audiostream->set_stream_size(sizeof(android_adsp_codec_wma_t));
                p_buf_audiostream->set_stream_offset(0);
                p_buf_audiostream->set_flag(mmp_buffer_media::FLAG_CONFIGDATA);

                break;
            
            case MMP_FOURCC_AUDIO_FLAC: 
            
                p_flac = (android_adsp_codec_flac_t*)buffer;
                s = m_pAvformatCtx->streams[stream_index];
                cc = s->codec;

                af = this->get_audio_format();
                
                p_flac->extra_data_size = af.extra_data_size;
                if(p_flac->extra_data_size > FFMPEG_FLAC_STREAMINFO_SIZE) {
                   p_flac->extra_data_size = 0;
                }
                if(p_flac->extra_data_size > 0) {
                    memcpy(p_flac->extra_data, af.extra_data, p_flac->extra_data_size);
                }
                                
                p_buf_audiostream->set_stream_size(sizeof(android_adsp_codec_flac_t));
                p_buf_audiostream->set_stream_offset(0);
                p_buf_audiostream->set_flag(mmp_buffer_media::FLAG_CONFIGDATA);


                break;
            
            case MMP_FOURCC_AUDIO_MP3: 
            case MMP_FOURCC_AUDIO_WMA_PRO: 
            default: 
                p_af = (struct mmp_audio_format*)buffer;
                s = m_pAvformatCtx->streams[stream_index];
                cc = s->codec;

                (*p_af) = this->get_audio_format();
                                            
                p_buf_audiostream->set_stream_size(sizeof(struct mmp_audio_format));
                p_buf_audiostream->set_stream_offset(0);
                p_buf_audiostream->set_flag(mmp_buffer_media::FLAG_CONFIGDATA);

                break;
        }

        /*
        if(fourcc == MMP_FOURCC_AUDIO_FFMPEG) {
        
            uint8_t *extra_ffmpeg, *pdata;
            uint32_t key;
            struct mmp_audio_packet_header* p_audio_pkt_hdr;

            s = m_pAvformatCtx->streams[stream_index];
            cc = s->codec;

            p_buf_audiostream->set_stream_size(sizeof(AVCodecContext) + cc->extradata_size + 1024);
            p_buf_audiostream->set_stream_offset(0);
            p_buf_audiostream->set_flag(mmp_buffer_media::FLAG_CONFIGDATA);

                        
            extra_ffmpeg = (uint8_t *)p_buf_audiostream->get_buf_vir_addr();
            p_audio_pkt_hdr = (struct mmp_audio_packet_header*)extra_ffmpeg;

            mmp_audio_tool::make_audio_packet_header(cc->extradata, cc->extradata_size,
                                                    cc, sizeof(AVCodecContext), 
                                                    (MMP_ADDR)extra_ffmpeg);
            
        }
        else if(fourcc == MMP_FOURCC_AUDIO_WMA2) {

            p_extra_info = (struct mmp_audio_decoder_extra_info*)buffer;
            s = m_pAvformatCtx->streams[stream_index];
            cc = s->codec;

            p_extra_info->fourcc = fourcc;

            p_extra_info->channels = ff_get_codec_param_channels(s);
            p_extra_info->sample_rate = ff_get_codec_param_sample_rate(s);
            p_extra_info->block_align = ff_get_codec_param_block_align(s);
            p_extra_info->frame_size = ff_get_codec_param_frame_size(s);
            
            p_extra_info->extra_data_size = cc->extradata_size;
            memcpy(p_extra_info->extra_data, cc->extradata, cc->extradata_size);

            
            p_buf_audiostream->set_stream_size(sizeof(struct mmp_audio_decoder_extra_info));
            p_buf_audiostream->set_stream_offset(0);
            p_buf_audiostream->set_flag(mmp_buffer_media::FLAG_CONFIGDATA);
        }
        else {
            s = m_pAvformatCtx->streams[stream_index];
            cc = s->codec;

            memcpy(buffer, cc->extradata, cc->extradata_size);
            p_buf_audiostream->set_stream_size(cc->extradata_size);
            p_buf_audiostream->set_stream_offset(0);
            p_buf_audiostream->set_flag(mmp_buffer_media::FLAG_CONFIGDATA);

            // set cc 
            p_buf_audiostream->set_ffmpeg_codec_context(cc, sizeof(AVCodecContext) );
        
            // set frame rate 
            if(s->avg_frame_rate.den && s->avg_frame_rate.num)
                frame_rate = (MMP_S32)((double)s->avg_frame_rate.num/(double)s->avg_frame_rate.den);
        }
        */

        mmpResult = MMP_SUCCESS;
    }

    return mmpResult;
}

MMP_RESULT CMmpDemuxer_FfmpegEx1::GetVideoExtraData(class mmp_buffer_videostream* p_buf_vs) {

    MMP_RESULT mmpResult = MMP_FAILURE;
    AVStream *s;
    AVCodecContext *cc;
    MMP_S32 stream_index;
    MMP_U8* buffer;
    MMP_S32 frame_rate = 0;
    
    p_buf_vs->set_stream_size(0);
    buffer = (MMP_U8*)p_buf_vs->get_buf_vir_addr();
    stream_index = m_nStreamIndex[MMP_MEDIATYPE_VIDEO];

    if(stream_index >= 0) {

        s = m_pAvformatCtx->streams[stream_index];
        cc = s->codec;

        memcpy(buffer, cc->extradata, cc->extradata_size);
        p_buf_vs->set_stream_size(cc->extradata_size);
        p_buf_vs->set_stream_offset(0);
        p_buf_vs->set_flag(mmp_buffer_media::FLAG_CONFIGDATA);

        /* set cc */
        p_buf_vs->set_ffmpeg_codec_context(cc, sizeof(AVCodecContext) );

        /* set frame rate */
        if(s->avg_frame_rate.den && s->avg_frame_rate.num)
            frame_rate = (MMP_S32)((double)s->avg_frame_rate.num/(double)s->avg_frame_rate.den);

        if(!frame_rate && s->r_frame_rate.den && s->r_frame_rate.num)
            frame_rate = (MMP_S32)((double)s->r_frame_rate.num/(double)s->r_frame_rate.den);
        p_buf_vs->set_player_framerate(frame_rate);

        /* set width, height */
        p_buf_vs->set_pic_width(s->codec->width);
        p_buf_vs->set_pic_height(s->codec->height);

        /* set bit rate */
        p_buf_vs->set_player_bitrate(s->codec->bit_rate);


        mmpResult = MMP_SUCCESS;

#ifdef WIN32
        MMP_U32 codec_tag = ff_get_codec_param_codec_tag(s);
        MMPDEBUGMSG(1, (TEXT("[CMmpDemuxer_FfmpegEx1::GetVideoExtraData] codectag : %c%c%c%c "), MMPGETFOURCCARG(codec_tag) ));
#endif		
    }

    return mmpResult;
}


MMP_RESULT CMmpDemuxer_FfmpegEx1::GetNextMediaData(MMP_U32 mediatype, MMP_U8* buffer, MMP_U32 buf_max_size, MMP_U32* buf_size, MMP_S64* packt_pts, mmp_buffer_media::FLAG *flag) {

    MMP_RESULT mmpResult = MMP_FAILURE;
    MMP_BOOL bRun = MMP_TRUE;
    int64_t packet_timestamp;
    AVStream *s;
    char szMediaType[16];
    int req_stream_idx;


    if(buf_size) *buf_size = 0;
    if(packt_pts) *packt_pts = 0;

    while(bRun == MMP_TRUE) {

        if(av_read_frame(m_pAvformatCtx, &m_pkt ) != 0)  {
            break;
        }

        req_stream_idx = m_nStreamIndex[mediatype];

        if(m_pkt.stream_index == req_stream_idx) {

            s = m_pAvformatCtx->streams[m_pkt.stream_index];

            if(m_pkt.stream_index == m_nStreamIndex[MMP_MEDIATYPE_VIDEO]) {
                strcpy(szMediaType, "Video");
            }
            else if(m_pkt.stream_index == m_nStreamIndex[MMP_MEDIATYPE_AUDIO]) {
                strcpy(szMediaType, "Audio");
            }
            else {
                strcpy(szMediaType, "Unknown");
            }

            memcpy(buffer, m_pkt.data, m_pkt.size);
            if(buf_size) *buf_size = m_pkt.size;
                        
            if(flag) {
                if(m_pkt.flags == AV_PKT_FLAG_KEY) *flag = mmp_buffer_media::FLAG_VIDEO_KEYFRAME;
                else *flag = mmp_buffer_media::FLAG_NULL;
            }
            if(packt_pts) {
             
                packet_timestamp = m_pkt.pts;
                if(packet_timestamp == AV_NOPTS_VALUE) {
                    packet_timestamp = m_pkt.dts;
                }
                (*packt_pts) = (packet_timestamp - ((s->start_time!=AV_NOPTS_VALUE)?s->start_time:0) ) * 1000000 * s->time_base.num / s->time_base.den;
                //qpack.flags = 0;
                //if(pkt.flags & AV_PKT_FLAG_KEY) {
                //    qpack.flags |= mmp_buffer_media::FLAG_VIDEO_KEYFRAME;
               // }
                
                MMPDEBUGMSG(0, (TEXT("[CMmpDemuxer_FfmpegEx1::GetNextMediaData] MediaType=(%s)  pts=%lld packsz=%d  pack(pts=%d dts=%d start=%d num=%d den=%d ) \n\r"),
                                      szMediaType,
                                      (*packt_pts),
                                      m_pkt.size,
                                      //(unsigned int)(pkt.pts/1000), (unsigned int)(pkt.dts/1000),
                                      (unsigned int)(m_pkt.pts), (unsigned int)(m_pkt.dts),
                                      (unsigned int)(s->start_time/1000), s->time_base.num, s->time_base.den 
                                      ));
            }

            if(m_pkt.stream_index == req_stream_idx) {
                mmpResult = MMP_SUCCESS;
                bRun = MMP_FALSE;
            }
            
        }

        av_packet_unref(&m_pkt);
    }

    return mmpResult;
}


void CMmpDemuxer_FfmpegEx1::queue_buffering(void) {

    AVStream *s;
    int64_t packet_timestamp;
    MMP_S32 empty_stream_index;
    char szMediaType[32];
    class mmp_buffer_media* p_buf_media;
    MMP_TICKS pts;
    MMP_ADDR buf_vir_addr;
    
    while(1) {
        empty_stream_index = queue_get_empty_streamindex();
        if(empty_stream_index != -1) {
            /* Read a frame */
            if(av_read_frame(m_pAvformatCtx, &m_pkt ) )
            {
                //ALOGE ("av_read_frame error ... ");
                break;
            }
            else {
                
                s = m_pAvformatCtx->streams[m_pkt.stream_index];
                
                if(m_pkt.stream_index == m_nStreamIndex[MMP_MEDIATYPE_VIDEO]) {
                    p_buf_media = mmp_buffer_mgr::get_instance()->alloc_media_videostream((MMP_MEDIA_ID)this, mmp_buffer_videostream::DEFAULT_MAX_STREAM_SIZE, mmp_buffer::HEAP);
                    strcpy(szMediaType, "Video");
                }
                else if(m_pkt.stream_index == m_nStreamIndex[MMP_MEDIATYPE_AUDIO]) {
                    p_buf_media = mmp_buffer_mgr::get_instance()->alloc_media_audiostream((MMP_MEDIA_ID)this, mmp_buffer_audiostream::DEFAULT_MAX_STREAM_SIZE, mmp_buffer::HEAP);
                    strcpy(szMediaType, "Audio");
                }
                else {
                    p_buf_media = NULL;
                    strcpy(szMediaType, "Unknown");
                }

                packet_timestamp = m_pkt.pts;
                if(packet_timestamp == AV_NOPTS_VALUE) {
                    packet_timestamp = m_pkt.dts;
                }
                pts = (packet_timestamp - ((s->start_time!=AV_NOPTS_VALUE)?s->start_time:0) ) * 1000000 * s->time_base.num / s->time_base.den;
                if(p_buf_media != NULL) {
                    p_buf_media->set_pts(pts);
                    if(m_pkt.flags & AV_PKT_FLAG_KEY) {
                        p_buf_media->set_flag(mmp_buffer_media::FLAG_VIDEO_KEYFRAME);
                    }
                    else {
                        p_buf_media->set_flag(mmp_buffer_media::FLAG_NULL);
                    }
                
                    if(p_buf_media->is_audio() == MMP_TRUE) {
                         class mmp_buffer_audiostream* p_buf_audiostream = (class mmp_buffer_audiostream*)p_buf_media;
                         buf_vir_addr = p_buf_audiostream->get_buf_vir_addr();
                         p_buf_audiostream->set_stream_size(m_pkt.size);
                         memcpy((void*)buf_vir_addr, m_pkt.data, m_pkt.size);
                    }
                    else {
                         class mmp_buffer_videostream* p_buf_vs = (class mmp_buffer_videostream*)p_buf_media;
                         buf_vir_addr = p_buf_vs->get_buf_vir_addr();
                         p_buf_vs->set_stream_size(m_pkt.size);
                         memcpy((void*)buf_vir_addr, m_pkt.data, m_pkt.size);
                    }

                    int hour, min, sec, milesec;

                    hour = CMmpUtil::Time_GetHour((unsigned int)(pts/1000LL));
                    min = CMmpUtil::Time_GetMin((unsigned int)(pts/1000LL));
                    sec = CMmpUtil::Time_GetSec((unsigned int)(pts/1000LL)); 
                    milesec = CMmpUtil::Time_GetMileSec((unsigned int)(pts/1000LL));

                    MMPDEBUGMSG(1, (TEXT(MMP_CTAG" MediaType=(%d %s)  pts=<%lld %02d:%02d:%02d (%04dms)> packsz=%d  pack(pts=%lld dts=%lld start=%d num=%d den=%d ) \n\r"), MMP_CNAME, 
                                        p_buf_media->get_media_type(), szMediaType,
                                        pts, hour, min, sec, milesec,
                                        m_pkt.size,
                                        m_pkt.pts, m_pkt.dts,
                                        (unsigned int)(s->start_time/1000), s->time_base.num, s->time_base.den 
                                        ));

                
                    this->queue_add(p_buf_media);
                }
                
                av_packet_unref(&m_pkt);
            }

        }/* end of if(i != ANAFFMPEG_MAX_STREAM_COUNT) */
        else {
            break;
        } 
    } /* end of while(run == true) { */

}

#if 0
class mmp_buffer_media* CMmpDemuxer_FfmpegEx1::GetNextMediaBuffer() {

    AVStream *s;
    int64_t packet_timestamp;
    char szMediaType[32];
    class mmp_buffer_media* p_buf_media = NULL;
    MMP_TICKS pts;
    MMP_ADDR buf_vir_addr;
    
    /* Read a frame */
    if(av_read_frame(m_pAvformatCtx, &m_pkt ) )
    {
        //ALOGE ("av_read_frame error ... ");
        
    }
    else {
                
        s = m_pAvformatCtx->streams[m_pkt.stream_index];
                
        if(m_pkt.stream_index == m_nStreamIndex[MMP_MEDIATYPE_VIDEO]) {
            p_buf_media = mmp_buffer_mgr::get_instance()->alloc_media_videostream((MMP_MEDIA_ID)this, mmp_buffer_videostream::DEFAULT_MAX_STREAM_SIZE, mmp_buffer::HEAP);
            strcpy(szMediaType, "Video");
        }
        else if(m_pkt.stream_index == m_nStreamIndex[MMP_MEDIATYPE_AUDIO]) {
            p_buf_media = mmp_buffer_mgr::get_instance()->alloc_media_audiostream((MMP_MEDIA_ID)this, mmp_buffer_audiostream::DEFAULT_MAX_STREAM_SIZE, mmp_buffer::HEAP);
            strcpy(szMediaType, "Audio");
        }
        else {
            p_buf_media = NULL;
            strcpy(szMediaType, "Unknown");
        }

        packet_timestamp = m_pkt.pts;
        if(packet_timestamp == AV_NOPTS_VALUE) {
            packet_timestamp = m_pkt.dts;
        }
        if(packet_timestamp == AV_NOPTS_VALUE) {
            pts = m_last_seek_pts;
        }
        else {
            pts = (packet_timestamp - ((s->start_time!=AV_NOPTS_VALUE)?s->start_time:0) ) * 1000000 * s->time_base.num / s->time_base.den;
        }
        if(p_buf_media != NULL) {
            p_buf_media->set_pts(pts);
            if(m_pkt.flags & AV_PKT_FLAG_KEY) {
                p_buf_media->set_flag(mmp_buffer_media::FLAG_VIDEO_KEYFRAME);
            }
            else {
                p_buf_media->set_flag(mmp_buffer_media::FLAG_NULL);
            }
                
            if(p_buf_media->is_audio() == MMP_TRUE) {
                    class mmp_buffer_audiostream* p_buf_audiostream = (class mmp_buffer_audiostream*)p_buf_media;
                    buf_vir_addr = p_buf_audiostream->get_buf_vir_addr();
                    p_buf_audiostream->set_stream_size(m_pkt.size);
                    memcpy((void*)buf_vir_addr, m_pkt.data, m_pkt.size);
            }
            else {
                    class mmp_buffer_videostream* p_buf_vs = (class mmp_buffer_videostream*)p_buf_media;
                    buf_vir_addr = p_buf_vs->get_buf_vir_addr();
                    p_buf_vs->set_stream_size(m_pkt.size);
                    memcpy((void*)buf_vir_addr, m_pkt.data, m_pkt.size);
            }

            int hour, min, sec, milesec;

            hour = CMmpUtil::Time_GetHour((unsigned int)(pts/1000LL));
            min = CMmpUtil::Time_GetMin((unsigned int)(pts/1000LL));
            sec = CMmpUtil::Time_GetSec((unsigned int)(pts/1000LL)); 
            milesec = CMmpUtil::Time_GetMileSec((unsigned int)(pts/1000LL));

            MMPDEBUGMSG(1, (TEXT(MMP_CTAG" MediaType=(%d %s)  pts=<%lld %02d:%02d:%02d (%04dms)> packsz=%d  pack(pts=%lld dts=%lld start=%d num=%d den=%d ) \n\r"), MMP_CNAME, 
                                p_buf_media->get_media_type(), szMediaType,
                                pts, hour, min, sec, milesec,
                                m_pkt.size,
                                m_pkt.pts, m_pkt.dts,
                                (unsigned int)(s->start_time/1000), s->time_base.num, s->time_base.den 
                                ));

        }
                
        av_packet_unref(&m_pkt);
    }

    
    return p_buf_media;
}
#endif

MMP_RESULT CMmpDemuxer_FfmpegEx1::Seek(MMP_S64 pts) {


    this->queue_clear();

    int stream_index;
    AVStream* s;
    int64_t timestamp;
    int seek_res = -1;
    
    
    stream_index = m_nStreamIndex[MMP_MEDIATYPE_VIDEO];
    if(stream_index >= 0) {
        s = m_pAvformatCtx->streams[stream_index];
        timestamp = av_rescale(pts, s->time_base.den, AV_TIME_BASE * (int64_t) s->time_base.num);
        seek_res = av_seek_frame(m_pAvformatCtx, stream_index, timestamp, 0);
    }

    if(seek_res < 0) {

        stream_index = m_nStreamIndex[MMP_MEDIATYPE_AUDIO];
        if(stream_index >= 0) {
            s = m_pAvformatCtx->streams[stream_index];
            timestamp = av_rescale(pts, s->time_base.den, AV_TIME_BASE * (int64_t) s->time_base.num);
            seek_res = av_seek_frame(m_pAvformatCtx, stream_index, timestamp, 0);
        }
    }

    if(seek_res >= 0) {
        m_last_seek_pts = pts;
        MMPDEBUGMSG(1, (TEXT(MMP_CTAG" Seek %lld"), MMP_CNAME,  pts ));
    }

    return MMP_SUCCESS;
}


#endif