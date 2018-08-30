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

#include "MmpDemuxer_Ffmpeg.hpp"
#include "MmpUtil.hpp"
#include "mmp_audio_tool.hpp"

#if (FFMPEG_VERSION == FFMPEG_VERSION_2_6_1)

/////////////////////////////////////////////////////
// class

CMmpDemuxer_Ffmpeg::CMmpDemuxer_Ffmpeg(struct MmpDemuxerCreateConfig* pCreateConfig) : CMmpDemuxer(pCreateConfig)
,m_fp(NULL)
,m_IoBuffer(NULL)
,m_Url(NULL)
,m_pAVIOContext(NULL)
,m_pAvformatCtx(NULL)
,m_iFileSize(0)
,m_nFileBufferIndex(0)
,m_nFileBufferSize(0)
{
    int i;

    av_register_all();

    for(i = 0; i < MMP_MEDIATYPE_MAX; i++) {
        m_nStreamIndex[i] = -1;
    }
}

CMmpDemuxer_Ffmpeg::~CMmpDemuxer_Ffmpeg()
{

}

MMP_RESULT CMmpDemuxer_Ffmpeg::Open()
{
    MMP_RESULT mmpResult = MMP_SUCCESS;

    int i;
    AVProbeData   pd;
    uint8_t *buffer = NULL;
    AVInputFormat *fmt = NULL;
    const int io_buffer_size = 32768;  /* FIXME */

	MMPDEBUGMSG(1, (TEXT("[CMmpDemuxer_Ffmpeg::Open] Init Demuxer ... Wait for about 1sec")));

    m_fp = fopen((const char*)this->m_create_config.filename, "rb");
    if(m_fp == NULL) {
        mmpResult = MMP_FAILURE;
    }
    else {
        fseek(m_fp, 0, SEEK_END);
        m_iFileSize = ftell(m_fp);
        fseek(m_fp, 0, SEEK_SET);
    
    }

    /* alloc first 1MB data */
    if(mmpResult == MMP_SUCCESS) {

        pd.buf_size = 1024 * 1024 * 1;
        buffer = (uint8_t *)malloc(pd.buf_size);
        if(buffer == NULL) {
            mmpResult = MMP_FAILURE;
        }
        pd.filename = NULL;
        pd.buf = buffer;
    }

    /* read first 1MB data  and Guess format */
    if(mmpResult == MMP_SUCCESS) {
        
        int32_t size = fread(buffer, 1, pd.buf_size, m_fp);
        if(size <= 0)
        {
            mmpResult = MMP_FAILURE;
        }
        else {

            fseek(m_fp, 0, SEEK_SET);

            pd.buf_size = size;
            fmt = av_probe_input_format(&pd, 1);
            if(fmt == NULL)
            {
               mmpResult = MMP_FAILURE;     
            }
        }
    }

    /* Create I/O wrapper and URL */
    if(mmpResult == MMP_SUCCESS) {

        m_IoBuffer = (MMP_U8*)malloc(io_buffer_size);
        if(m_IoBuffer == NULL)  {
            mmpResult = MMP_FAILURE;     
        }
        else {
            memset(m_IoBuffer, 0x00, io_buffer_size);
        }
            	
        m_Url = (URLContext*)malloc( sizeof(URLContext) );
        if(m_Url == NULL) {
            mmpResult = MMP_FAILURE;     
        }
        else {
            memset(m_Url, 0x00, sizeof(URLContext));
            m_Url->priv_data = (void*)this;
        }
    }

    /* open io context */
    if(mmpResult == MMP_SUCCESS) {
    
        m_pAVIOContext = avio_alloc_context(m_IoBuffer, io_buffer_size, 0, m_Url, CMmpDemuxer_Ffmpeg::IORead_C_Stub, NULL, CMmpDemuxer_Ffmpeg::IOSeek_C_Stub);
        if(m_pAVIOContext == NULL) {
            mmpResult = MMP_FAILURE;     
        }
    }

    /* open format context */
    if(mmpResult == MMP_SUCCESS) {
        m_pAvformatCtx = avformat_alloc_context();
        m_pAvformatCtx->pb = m_pAVIOContext;
        if(avformat_open_input( &m_pAvformatCtx, "", fmt, NULL ) != 0)
        {
            mmpResult = MMP_FAILURE;  
        }
        else {

            //if (av_find_stream_info(m_pAvformatCtx) < 0) 
            if(avformat_find_stream_info(m_pAvformatCtx, NULL) < 0)     
            {
                mmpResult = MMP_FAILURE;  
            }
        }
    }

    if(mmpResult == MMP_SUCCESS) {

        for(i = 0; i < (int)(m_pAvformatCtx->nb_streams); i++ )
        {
            AVStream *s = m_pAvformatCtx->streams[i];
            AVCodecContext *cc = s->codec;

            //ALOGI ("AVCodecContext codecID = 0x%x", cc->codec_id);
            switch((MMP_U32)cc->codec_type ) {
            
                case AVMEDIA_TYPE_AUDIO:
                    m_nStreamIndex[MMP_MEDIATYPE_AUDIO] = i;
                    break;

                case AVMEDIA_TYPE_VIDEO:
                    m_nStreamIndex[MMP_MEDIATYPE_VIDEO] = i;
                    break;
            }
        }
    
    }

    if(buffer != NULL) {
        free(buffer);
    }

    return mmpResult;
}

MMP_RESULT CMmpDemuxer_Ffmpeg::Close()
{
 
    if(m_pAvformatCtx != NULL)
    {
        avformat_close_input(&m_pAvformatCtx);
        m_pAvformatCtx = NULL;
    }

    if(m_pAVIOContext != NULL) {
#if (MMP_OS == MMP_OS_WIN32)
        avio_close(m_pAVIOContext);
#endif   
        m_pAVIOContext = NULL;

        m_Url = NULL;
        m_IoBuffer = NULL;
    }

    if(m_Url != NULL) {
        free(m_Url);
        m_Url = NULL;
    }

    if(m_IoBuffer != NULL) {
        free(m_IoBuffer);
        m_IoBuffer = NULL;
    }

    if(m_fp != NULL) {
        fclose(m_fp);
        m_fp = NULL;
    }

    return MMP_SUCCESS;
}

MMP_S64 CMmpDemuxer_Ffmpeg::GetDuration() {

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

enum MMP_FOURCC CMmpDemuxer_Ffmpeg::GetAudioFourcc() {

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

        }
    }

    return fourcc;
}

MMP_U32 CMmpDemuxer_Ffmpeg::GetAudioChannel() {
    
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
    
MMP_U32 CMmpDemuxer_Ffmpeg::GetAudioSamplingRate() {

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

MMP_U32 CMmpDemuxer_Ffmpeg::GetAudioBitsPerSample() {

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

MMP_S32 CMmpDemuxer_Ffmpeg::GetAudioBitrate() {

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

MMP_S32 CMmpDemuxer_Ffmpeg::GetVideoBitrate() {

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

MMP_S32 CMmpDemuxer_Ffmpeg::GetVideoFPS() {

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
    
enum MMP_FOURCC CMmpDemuxer_Ffmpeg::GetVideoFourcc() {

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
            case AV_CODEC_ID_VC1:  fourcc = MMP_FOURCC_VIDEO_WVC1; break;

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

MMP_U32 CMmpDemuxer_Ffmpeg::GetVideoPicWidth() {

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

MMP_U32 CMmpDemuxer_Ffmpeg::GetVideoPicHeight() {

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

MMP_RESULT CMmpDemuxer_Ffmpeg::GetVideoExtraData(MMP_U8* buffer, MMP_U32 buf_max_size, MMP_U32* buf_size)  {

#if 0
    MMP_RESULT mmpResult = MMP_FAILURE;
    AVStream *s;
    AVCodecContext *cc;
    MMP_S32 stream_index;
    struct mmp_ffmpeg_packet_header ffmpeg_packet_header;
    MMP_U8* pdata;
    
    if(buf_size) *buf_size = 0;
    stream_index = m_nStreamIndex[MMP_MEDIATYPE_VIDEO];

    if(stream_index >= 0) {
        s = m_pAvformatCtx->streams[stream_index];
        cc = s->codec;

        switch((MMP_U32)cc->codec_id) {

            case AV_CODEC_ID_H264:
            case AV_CODEC_ID_MPEG4:
            case AV_CODEC_ID_WMV3:
                if(cc->extradata_size > 0) {
                    memcpy(buffer, cc->extradata, cc->extradata_size);
                    if(buf_size) *buf_size = cc->extradata_size;
                    mmpResult = MMP_SUCCESS;
                }
                break;
            
            case AV_CODEC_ID_RV30:
            case AV_CODEC_ID_RV40:
            case AV_CODEC_ID_VP8:
                pdata = buffer;
                memcpy(pdata, cc, sizeof(AVCodecContext)); pdata+=sizeof(AVCodecContext);
                memcpy(pdata, s, sizeof(AVStream)); pdata+=sizeof(AVStream);
                if(cc->extradata_size > 0) {
                    memcpy(pdata, cc->extradata, cc->extradata_size); pdata+=cc->extradata_size;
                }
                if(buf_size) *buf_size = (unsigned int)pdata-(unsigned int)buffer;
                break;
            
            case AV_CODEC_ID_MSMPEG4V3:
                pdata = buffer;
                memcpy(pdata, cc, sizeof(AVCodecContext)); pdata+=sizeof(AVCodecContext);
                memcpy(pdata, s, sizeof(AVStream)); pdata+=sizeof(AVStream);
                //memcpy(pdata, cc->extradata, cc->extradata_size); pdata+=cc->extradata_size;
                if(buf_size) *buf_size = (unsigned int)pdata-(unsigned int)buffer;
                break;

            default:
                ffmpeg_packet_header.key = MMP_FFMPEG_PACKET_HEADER_KEY;
                ffmpeg_packet_header.payload_type = MMP_FFMPEG_PACKET_TYPE_AVCodecContext;
                ffmpeg_packet_header.hdr_size = sizeof(struct mmp_ffmpeg_packet_header);
                ffmpeg_packet_header.payload_size = sizeof(AVCodecContext);
                ffmpeg_packet_header.extra_data_size = cc->extradata_size;
                ffmpeg_packet_header.packet_size = ffmpeg_packet_header.hdr_size+ffmpeg_packet_header.payload_size+ffmpeg_packet_header.extra_data_size;

                pdata = buffer;
                memcpy(pdata, &ffmpeg_packet_header, sizeof(struct mmp_ffmpeg_packet_header)); pdata+=sizeof(struct mmp_ffmpeg_packet_header);
                memcpy(pdata, cc, sizeof(AVCodecContext)); pdata+=sizeof(AVCodecContext);
                memcpy(pdata, cc->extradata, cc->extradata_size); pdata+=cc->extradata_size;
                
                mmpResult = MMP_SUCCESS;
                if(buf_size) *buf_size = ffmpeg_packet_header.packet_size;
        }

    }

    return mmpResult;

#else

    return MMP_FAILURE;
#endif
}

MMP_RESULT CMmpDemuxer_Ffmpeg::GetMediaExtraData(MMP_U32 mediatype, MMP_U8* buffer, MMP_U32 buf_max_size, MMP_U32* buf_size)  {

#if 0
    MMP_RESULT mmpResult = MMP_FAILURE;
    AVStream *s;
    AVCodecContext *cc;
    MMP_S32 stream_index;
    struct mmp_ffmpeg_packet_header ffmpeg_packet_header;
    MMP_U8* pdata;
    
    if(buf_size) *buf_size = 0;
    stream_index = m_nStreamIndex[mediatype];

    if(stream_index >= 0) {
        s = m_pAvformatCtx->streams[stream_index];
        cc = s->codec;

        switch((MMP_U32)cc->codec_id) {

            case AV_CODEC_ID_H264:
            case AV_CODEC_ID_MPEG4:
                if(cc->extradata_size > 0) {
                    memcpy(buffer, cc->extradata, cc->extradata_size);
                    if(buf_size) *buf_size = cc->extradata_size;
                    mmpResult = MMP_SUCCESS;
                }
                break;
            
            case AV_CODEC_ID_RV30:
            case AV_CODEC_ID_RV40:
            case AV_CODEC_ID_WMV3:
            case AV_CODEC_ID_VP8:
                pdata = buffer;
                memcpy(pdata, cc, sizeof(AVCodecContext)); pdata+=sizeof(AVCodecContext);
                memcpy(pdata, s, sizeof(AVStream)); pdata+=sizeof(AVStream);
                if(cc->extradata_size > 0) {
                    memcpy(pdata, cc->extradata, cc->extradata_size); pdata+=cc->extradata_size;
                }
                if(buf_size) *buf_size = (unsigned int)pdata-(unsigned int)buffer;
                mmpResult = MMP_SUCCESS;
                break;
            
            default:
                ffmpeg_packet_header.key = MMP_FFMPEG_PACKET_HEADER_KEY;
                ffmpeg_packet_header.payload_type = MMP_FFMPEG_PACKET_TYPE_AVCodecContext;
                ffmpeg_packet_header.hdr_size = sizeof(struct mmp_ffmpeg_packet_header);
                ffmpeg_packet_header.payload_size = sizeof(AVCodecContext);
                ffmpeg_packet_header.extra_data_size = cc->extradata_size;
                ffmpeg_packet_header.packet_size = ffmpeg_packet_header.hdr_size+ffmpeg_packet_header.payload_size+ffmpeg_packet_header.extra_data_size;

                pdata = buffer;
                memcpy(pdata, &ffmpeg_packet_header, sizeof(struct mmp_ffmpeg_packet_header)); pdata+=sizeof(struct mmp_ffmpeg_packet_header);
                memcpy(pdata, cc, sizeof(AVCodecContext)); pdata+=sizeof(AVCodecContext);
                memcpy(pdata, cc->extradata, cc->extradata_size); pdata+=cc->extradata_size;
                
                mmpResult = MMP_SUCCESS;
                if(buf_size) *buf_size = ffmpeg_packet_header.packet_size;
        }

    }

    return mmpResult;

#else
    return MMP_FAILURE;
#endif
}

MMP_RESULT CMmpDemuxer_Ffmpeg::GetAudioExtraData(class mmp_buffer_audiostream* p_buf_audiostream) {

    MMP_RESULT mmpResult = MMP_FAILURE;
    AVStream *s;
    AVCodecContext *cc;
    MMP_S32 stream_index;
    MMP_U8* buffer;
    MMP_S32 frame_rate = 0;
    enum MMP_FOURCC fourcc;
    
    p_buf_audiostream->set_stream_size(0);
    buffer = (MMP_U8*)p_buf_audiostream->get_buf_vir_addr();
    stream_index = m_nStreamIndex[MMP_MEDIATYPE_AUDIO];

    fourcc = this->GetAudioFourcc();

    if(stream_index >= 0) {

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
        else {
            s = m_pAvformatCtx->streams[stream_index];
            cc = s->codec;

            memcpy(buffer, cc->extradata, cc->extradata_size);
            p_buf_audiostream->set_stream_size(cc->extradata_size);
            p_buf_audiostream->set_stream_offset(0);
            p_buf_audiostream->set_flag(mmp_buffer_media::FLAG_CONFIGDATA);

            /* set cc */
            p_buf_audiostream->set_ffmpeg_codec_context(cc, sizeof(AVCodecContext) );
        
            /* set frame rate */
            if(s->avg_frame_rate.den && s->avg_frame_rate.num)
                frame_rate = (MMP_S32)((double)s->avg_frame_rate.num/(double)s->avg_frame_rate.den);
        }

        mmpResult = MMP_SUCCESS;
    }

    return mmpResult;
}

MMP_RESULT CMmpDemuxer_Ffmpeg::GetVideoExtraData(class mmp_buffer_videostream* p_buf_vs) {

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
    }

    return mmpResult;
}

#if 0
MMP_RESULT CMmpDemuxer_Ffmpeg::GetNextVideoData(MMP_U8* buffer, MMP_U32 buf_max_size, MMP_U32* buf_size, MMP_S64* packt_pts) {

    MMP_RESULT mmpResult = MMP_FAILURE;
    AVPacket  pkt;
    MMP_BOOL bRun = MMP_TRUE;

    if(buf_size) *buf_size = 0;

    while(bRun == MMP_TRUE) {

        if(av_read_frame(m_pAvformatCtx, &pkt ) != 0)
        {
            break;
        }

        if(pkt.stream_index == m_nStreamIndex[MMP_MEDIATYPE_VIDEO]) {
            memcpy(buffer, pkt.data, pkt.size);
            if(buf_size) *buf_size = pkt.size;
            if(packt_pts) *packt_pts = pkt.pts;
            mmpResult = MMP_SUCCESS;
            bRun = MMP_FALSE;
        }

        av_free_packet(&pkt);
    }

    return mmpResult;
}  
#endif

MMP_RESULT CMmpDemuxer_Ffmpeg::GetNextMediaData(MMP_U32 mediatype, MMP_U8* buffer, MMP_U32 buf_max_size, MMP_U32* buf_size, MMP_S64* packt_pts, mmp_buffer_media::FLAG *flag) {

#if 1
    MMP_RESULT mmpResult = MMP_FAILURE;
    AVPacket  pkt;
    MMP_BOOL bRun = MMP_TRUE;
    int64_t packet_timestamp;
    AVStream *s;
    char szMediaType[16];

    if(buf_size) *buf_size = 0;
    if(packt_pts) *packt_pts = 0;

    while(bRun == MMP_TRUE) {

        if(av_read_frame(m_pAvformatCtx, &pkt ) != 0)
        {
            break;
        }

        if(pkt.stream_index == m_nStreamIndex[mediatype]) {

            s = m_pAvformatCtx->streams[pkt.stream_index];

            if(pkt.stream_index == m_nStreamIndex[MMP_MEDIATYPE_VIDEO]) {
                strcpy(szMediaType, "Video");
            }
            else if(pkt.stream_index == m_nStreamIndex[MMP_MEDIATYPE_AUDIO]) {
                strcpy(szMediaType, "Audio");
            }
            else {
                strcpy(szMediaType, "Unknown");
            }

            memcpy(buffer, pkt.data, pkt.size);
            if(buf_size) *buf_size = pkt.size;
            if(flag) {
                if(pkt.flags == AV_PKT_FLAG_KEY) *flag = mmp_buffer_media::FLAG_VIDEO_KEYFRAME;
                else *flag = mmp_buffer_media::FLAG_NULL;
            }
            if(packt_pts) {
             
                packet_timestamp = pkt.pts;
                if(packet_timestamp == AV_NOPTS_VALUE) {
                    packet_timestamp = pkt.dts;
                }
                (*packt_pts) = (packet_timestamp - ((s->start_time!=AV_NOPTS_VALUE)?s->start_time:0) ) * 1000000 * s->time_base.num / s->time_base.den;
                //qpack.flags = 0;
                //if(pkt.flags & AV_PKT_FLAG_KEY) {
                //    qpack.flags |= mmp_buffer_media::FLAG_VIDEO_KEYFRAME;
               // }

                MMPDEBUGMSG(0, (TEXT("[CMmpDemuxer_Ffmpeg::GetNextMediaData] MediaType=(%s)  pts=%d packsz=%d  pack(pts=%d dts=%d start=%d num=%d den=%d ) \n\r"),
                                      szMediaType,
                                      (unsigned int)((*packt_pts)/1000),
                                      pkt.size,
                                      //(unsigned int)(pkt.pts/1000), (unsigned int)(pkt.dts/1000),
                                      (unsigned int)(pkt.pts), (unsigned int)(pkt.dts),
                                      (unsigned int)(s->start_time/1000), s->time_base.num, s->time_base.den 
                                      ));
            }

            mmpResult = MMP_SUCCESS;
            bRun = MMP_FALSE;

            
        }

        av_free_packet(&pkt);
    }

    return mmpResult;

#else

    //mMMP_U8* buffer, MMP_U32 buf_max_size, MMP_U32* buf_size, MMP_S64* packt_pts

     *buf_size = 1024*8;
     *packt_pts = 100;

    return MMP_SUCCESS;
#endif 
}
    
int CMmpDemuxer_Ffmpeg::IORead_C_Stub(void *opaque, uint8_t *buf, int buf_size) {

    URLContext *p_url = (URLContext*)opaque;
    CMmpDemuxer_Ffmpeg *pObj = (CMmpDemuxer_Ffmpeg *)p_url->priv_data;
    return pObj->IORead_C(opaque, buf, buf_size);
}

int64_t CMmpDemuxer_Ffmpeg::IOSeek_C_Stub(void *opaque, int64_t offset, int whence) {

    URLContext *p_url = (URLContext*)opaque;
    CMmpDemuxer_Ffmpeg *pObj = (CMmpDemuxer_Ffmpeg *)p_url->priv_data;
    return pObj->IOSeek_C(opaque, offset, whence);
}

#if 0

int CMmpDemuxer_Ffmpeg::IORead_C(void *opaque, uint8_t *buf, int buf_size) {

    int i_ret ;
    int buffer_remain_byte;
    uint8_t * buf1;
    int buf_size1;
		
    if( buf_size < 0 ) return -1;
    if( buf_size == 0 ) return 0;
   

    buffer_remain_byte = m_nFileBufferSize - m_nFileBufferIndex;
    if(buffer_remain_byte == 0) {
       
        m_nFileBufferSize = fread(m_FileBuffer, 1, MMP_DEMUXER_FILE_BUFFER_MAX_SIZE, m_fp);
        m_nFileBufferIndex = 0;

        MMPDEBUGMSG(1, (TEXT("[CMmpDemuxer_Ffmpeg::IORead_C]  Burst Read 1 ")));
    }

    buffer_remain_byte = m_nFileBufferSize - m_nFileBufferIndex;

    if(buffer_remain_byte == 0) {
        i_ret = -1;
    }
    else if(buffer_remain_byte >= buf_size) {
        
        memcpy(buf, &m_FileBuffer[m_nFileBufferIndex], buf_size);
        m_nFileBufferIndex+=buf_size;

        i_ret = buf_size;
    }
    else {

        memcpy(buf, &m_FileBuffer[m_nFileBufferIndex], buffer_remain_byte);
        i_ret = buffer_remain_byte;
        
        buf1 = buf + buffer_remain_byte;
        buf_size1 = buf_size - buffer_remain_byte;
        while(buf_size1 > 0) {
            
            MMPDEBUGMSG(1, (TEXT("[CMmpDemuxer_Ffmpeg::IORead_C]  Burst Read 2 ")));
            m_nFileBufferSize = fread(m_FileBuffer, 1, MMP_DEMUXER_FILE_BUFFER_MAX_SIZE, m_fp);
            m_nFileBufferIndex = 0;

            buffer_remain_byte = m_nFileBufferSize - m_nFileBufferIndex;
            if(buffer_remain_byte == 0) {
                  break;
            }
            else if(buffer_remain_byte >= buf_size1) {
    
                memcpy(buf1, &m_FileBuffer[m_nFileBufferIndex], buf_size1);
                m_nFileBufferIndex+=buf_size1;

                i_ret += buf_size1;
                break;
            }
            else {
                //memcpy(buf1, &m_FileBuffer[m_nFileBufferIndex], buffer_remain_byte);
                //i_ret += buffer_remain_byte;
                //buf_size1 -= buffer_remain_byte;
                break;
            }

        } /* end of while */

    }
    

    return i_ret;
    //i_ret = fread(buf, 1, buf_size, m_fp);

    //return (i_ret >= 0) ? i_ret : -1;

}


#else
int CMmpDemuxer_Ffmpeg::IORead_C(void *opaque, uint8_t *buf, int buf_size) {

    int i_ret;
		
    if( buf_size < 0 ) return -1;

    i_ret = fread(buf, 1, buf_size, m_fp);

    return (i_ret >= 0) ? i_ret : -1;

}
#endif

int64_t CMmpDemuxer_Ffmpeg::IOSeek_C(void *opaque, int64_t offset, int whence) {


#if 0
    switch(whence)
    {
#ifdef AVSEEK_SIZE
        case AVSEEK_SIZE:
            return m_iFileSize;
#endif
        case SEEK_SET:
            //i_absolute = (int64_t)offset;
            fseek(m_fp, (long)offset, SEEK_SET);
            break;
        case SEEK_CUR:
            //i_absolute = pss->offset + (int64_t)offset;
            fseek(m_fp, (long)offset, SEEK_CUR);
            break;
        case SEEK_END:
            //i_absolute = i_size + (int64_t)offset;
            fseek(m_fp, (long)offset, SEEK_END);
            break;
        default:
            return -1;

    }

#else


    switch(whence)
    {
#ifdef AVSEEK_SIZE
        case AVSEEK_SIZE:
            return m_iFileSize;
#endif
        case SEEK_SET:
        case SEEK_CUR:
        case SEEK_END:
            fseek(m_fp, (long)offset, whence);
            m_nFileBufferIndex = 0;
            m_nFileBufferSize = 0;
            break;
            
        default:
            return -1;

    }

#endif

    return (int64_t)ftell(m_fp);
}


void CMmpDemuxer_Ffmpeg::queue_buffering(void) {

    AVPacket  pkt;
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
            if(av_read_frame(m_pAvformatCtx, &pkt ) )
            {
                //ALOGE ("av_read_frame error ... ");
                break;
            }
            else {
                
                s = m_pAvformatCtx->streams[pkt.stream_index];
                
                if(pkt.stream_index == m_nStreamIndex[MMP_MEDIATYPE_VIDEO]) {
                    p_buf_media = mmp_buffer_mgr::get_instance()->alloc_media_videostream((MMP_MEDIA_ID)this, mmp_buffer_videostream::DEFAULT_MAX_STREAM_SIZE, mmp_buffer::HEAP);
                    strcpy(szMediaType, "Video");
                }
                else if(pkt.stream_index == m_nStreamIndex[MMP_MEDIATYPE_AUDIO]) {
                    p_buf_media = mmp_buffer_mgr::get_instance()->alloc_media_audiostream((MMP_MEDIA_ID)this, mmp_buffer_audiostream::DEFAULT_MAX_STREAM_SIZE, mmp_buffer::HEAP);
                    strcpy(szMediaType, "Audio");
                }
                else {
                    p_buf_media = NULL;
                    strcpy(szMediaType, "Unknown");
                }

                packet_timestamp = pkt.pts;
                if(packet_timestamp == AV_NOPTS_VALUE) {
                    packet_timestamp = pkt.dts;
                }
                pts = (packet_timestamp - ((s->start_time!=AV_NOPTS_VALUE)?s->start_time:0) ) * 1000000 * s->time_base.num / s->time_base.den;
                if(p_buf_media != NULL) {
                    p_buf_media->set_pts(pts);
                    if(pkt.flags & AV_PKT_FLAG_KEY) {
                        p_buf_media->set_flag(mmp_buffer_media::FLAG_VIDEO_KEYFRAME);
                    }
                    else {
                        p_buf_media->set_flag(mmp_buffer_media::FLAG_NULL);
                    }
                
                    if(p_buf_media->is_audio() == MMP_TRUE) {
                         class mmp_buffer_audiostream* p_buf_audiostream = (class mmp_buffer_audiostream*)p_buf_media;
                         buf_vir_addr = p_buf_audiostream->get_buf_vir_addr();
                         p_buf_audiostream->set_stream_size(pkt.size);
                         memcpy((void*)buf_vir_addr, pkt.data, pkt.size);
                    }
                    else {
                         class mmp_buffer_videostream* p_buf_vs = (class mmp_buffer_videostream*)p_buf_media;
                         buf_vir_addr = p_buf_vs->get_buf_vir_addr();
                         p_buf_vs->set_stream_size(pkt.size);
                         memcpy((void*)buf_vir_addr, pkt.data, pkt.size);
                    }

                    MMPDEBUGMSG(0, (TEXT(MMP_CTAG" MediaType=(%d %s)  pts=%d packsz=%d  pack(pts=%d dts=%d start=%d num=%d den=%d ) \n\r"), MMP_CNAME, 
                                      p_buf_media->get_media_type(), szMediaType,
                                      (unsigned int)(pts/1000),
                                      pkt.size,
                                      (unsigned int)(pkt.pts/1000), (unsigned int)(pkt.dts/1000),
                                      (unsigned int)(s->start_time/1000), s->time_base.num, s->time_base.den 
                                      ));

                
                    this->queue_add(p_buf_media);
                }
                
                av_free_packet(&pkt);
            }

        }/* end of if(i != ANAFFMPEG_MAX_STREAM_COUNT) */
        else {
            break;
        } 
    } /* end of while(run == true) { */

}

class mmp_buffer_media* CMmpDemuxer_Ffmpeg::GetNextMediaBuffer() {

    AVPacket  pkt;
    AVStream *s;
    int64_t packet_timestamp;
    char szMediaType[32];
    class mmp_buffer_media* p_buf_media = NULL;
    MMP_TICKS pts;
    MMP_ADDR buf_vir_addr;
    
    /* Read a frame */
    if(av_read_frame(m_pAvformatCtx, &pkt ) )
    {
        //ALOGE ("av_read_frame error ... ");
        
    }
    else {
                
        s = m_pAvformatCtx->streams[pkt.stream_index];
                
        if(pkt.stream_index == m_nStreamIndex[MMP_MEDIATYPE_VIDEO]) {
            p_buf_media = mmp_buffer_mgr::get_instance()->alloc_media_videostream((MMP_MEDIA_ID)this, mmp_buffer_videostream::DEFAULT_MAX_STREAM_SIZE, mmp_buffer::HEAP);
            strcpy(szMediaType, "Video");
        }
        else if(pkt.stream_index == m_nStreamIndex[MMP_MEDIATYPE_AUDIO]) {
            p_buf_media = mmp_buffer_mgr::get_instance()->alloc_media_audiostream((MMP_MEDIA_ID)this, mmp_buffer_audiostream::DEFAULT_MAX_STREAM_SIZE, mmp_buffer::HEAP);
            strcpy(szMediaType, "Audio");
        }
        else {
            p_buf_media = NULL;
            strcpy(szMediaType, "Unknown");
        }

        packet_timestamp = pkt.pts;
        if(packet_timestamp == AV_NOPTS_VALUE) {
            packet_timestamp = pkt.dts;
        }
        pts = (packet_timestamp - ((s->start_time!=AV_NOPTS_VALUE)?s->start_time:0) ) * 1000000 * s->time_base.num / s->time_base.den;
        if(p_buf_media != NULL) {
            p_buf_media->set_pts(pts);
            if(pkt.flags & AV_PKT_FLAG_KEY) {
                p_buf_media->set_flag(mmp_buffer_media::FLAG_VIDEO_KEYFRAME);
            }
            else {
                p_buf_media->set_flag(mmp_buffer_media::FLAG_NULL);
            }
                
            if(p_buf_media->is_audio() == MMP_TRUE) {
                    class mmp_buffer_audiostream* p_buf_audiostream = (class mmp_buffer_audiostream*)p_buf_media;
                    buf_vir_addr = p_buf_audiostream->get_buf_vir_addr();
                    p_buf_audiostream->set_stream_size(pkt.size);
                    memcpy((void*)buf_vir_addr, pkt.data, pkt.size);
            }
            else {
                    class mmp_buffer_videostream* p_buf_vs = (class mmp_buffer_videostream*)p_buf_media;
                    buf_vir_addr = p_buf_vs->get_buf_vir_addr();
                    p_buf_vs->set_stream_size(pkt.size);
                    memcpy((void*)buf_vir_addr, pkt.data, pkt.size);
            }

            MMPDEBUGMSG(0, (TEXT(MMP_CTAG" MediaType=(%d %s)  pts=%d packsz=%d  pack(pts=%d dts=%d start=%d num=%d den=%d ) \n\r"), MMP_CNAME, 
                                p_buf_media->get_media_type(), szMediaType,
                                (unsigned int)(pts/1000),
                                pkt.size,
                                (unsigned int)(pkt.pts/1000), (unsigned int)(pkt.dts/1000),
                                (unsigned int)(s->start_time/1000), s->time_base.num, s->time_base.den 
                                ));

        }
                
        av_free_packet(&pkt);
    }

    
    return p_buf_media;
}

MMP_RESULT CMmpDemuxer_Ffmpeg::Seek(MMP_S64 pts) {

    this->queue_clear();

    if(av_seek_frame(m_pAvformatCtx, -1, pts, 0)  < 0)
    {
            
    }
    else {
    
    
    }

    return MMP_SUCCESS;
}

#endif
