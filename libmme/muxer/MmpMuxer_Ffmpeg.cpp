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

#include "MmpMuxer_Ffmpeg.hpp"
#include "MmpUtil.hpp"

#if (FFMPEG_VERSION == FFMPEG_VERSION_2_6_1)

/////////////////////////////////////////////////////
// class

#define STREAM_DURATION 10.0
#define STREAM_FRAME_RATE 25 /* 25 images/s */
#define STREAM_PIX_FMT AV_PIX_FMT_YUV420P /* default pix_fmt */


CMmpMuxer_Ffmpeg::CMmpMuxer_Ffmpeg(struct MmpMuxerCreateConfig* pCreateConfig) : CMmpMuxer(pCreateConfig)
,m_fp(NULL)
,m_iFileSize(0)

,m_oc(NULL)
,m_fmt(NULL)
,m_pAVIOContext(NULL)
,m_IoBuffer(NULL)
,m_Url(NULL)

,m_audio_st(NULL)
,m_video_st(NULL)
,m_audio_codec(NULL)
,m_video_codec(NULL)

,m_extra_data(NULL)
,m_extra_data_size(0)
{
    MMP_S32 i;

    av_register_all();

    switch(pCreateConfig->bih.biCompression) {
        /* Video */
        case MMP_FOURCC_VIDEO_H263: m_AVCodecID_Video = AV_CODEC_ID_H263; break;
        case MMP_FOURCC_VIDEO_H264: m_AVCodecID_Video = AV_CODEC_ID_H264; break;
        case MMP_FOURCC_VIDEO_MPEG4: m_AVCodecID_Video = AV_CODEC_ID_MPEG4; break;
        case MMP_FOURCC_VIDEO_MPEG2: m_AVCodecID_Video = AV_CODEC_ID_MPEG2VIDEO; break;
        
        case MMP_FOURCC_VIDEO_WMV1: m_AVCodecID_Video = AV_CODEC_ID_WMV1; break;
        case MMP_FOURCC_VIDEO_WMV2: m_AVCodecID_Video = AV_CODEC_ID_WMV2; break;
        case MMP_FOURCC_VIDEO_WMV3: m_AVCodecID_Video = AV_CODEC_ID_WMV3; break;

        case MMP_FOURCC_VIDEO_MSS1: m_AVCodecID_Video = AV_CODEC_ID_MSS1; break;
        case MMP_FOURCC_VIDEO_MSS2: m_AVCodecID_Video = AV_CODEC_ID_MSS2; break;
        
        case MMP_FOURCC_VIDEO_VP80: m_AVCodecID_Video = AV_CODEC_ID_VP8; break;
        case MMP_FOURCC_VIDEO_VP60: m_AVCodecID_Video = AV_CODEC_ID_VP6; break;
        case MMP_FOURCC_VIDEO_VP6F: m_AVCodecID_Video = AV_CODEC_ID_VP6F; break;
        case MMP_FOURCC_VIDEO_VP6A: m_AVCodecID_Video = AV_CODEC_ID_VP6A; break;

        case MMP_FOURCC_VIDEO_RV30: m_AVCodecID_Video = AV_CODEC_ID_RV30; break;
        case MMP_FOURCC_VIDEO_RV40: m_AVCodecID_Video = AV_CODEC_ID_RV40; break;
        
        case MMP_FOURCC_VIDEO_SVQ1: m_AVCodecID_Video = AV_CODEC_ID_SVQ1; break;
        case MMP_FOURCC_VIDEO_SVQ3: m_AVCodecID_Video = AV_CODEC_ID_SVQ3; break;
        
        case MMP_FOURCC_VIDEO_THEORA: m_AVCodecID_Video = AV_CODEC_ID_THEORA; break;
        case MMP_FOURCC_VIDEO_MJPEG: m_AVCodecID_Video = AV_CODEC_ID_MJPEG; break;
        case MMP_FOURCC_VIDEO_FLV1: m_AVCodecID_Video = AV_CODEC_ID_FLV1; break;
        //case MMP_FOURCC_VIDEO_MSMPEG4V1: m_AVCodecID_Video = AV_CODEC_ID_MSMPEG4V1; break;
        //case MMP_FOURCC_VIDEO_MSMPEG4V2: m_AVCodecID_Video = AV_CODEC_ID_MSMPEG4V2; break;
        case MMP_FOURCC_VIDEO_MSMPEG4V3: m_AVCodecID_Video = AV_CODEC_ID_MSMPEG4V3; break;
        
        default:  
            m_AVCodecID_Video = AV_CODEC_ID_NONE;
    }

    for(i = 0; i < MMP_MEDIATYPE_MAX; i++) {
         m_is_dsi_init[i] = MMP_FALSE;
    }
}

CMmpMuxer_Ffmpeg::~CMmpMuxer_Ffmpeg()
{

}

MMP_RESULT CMmpMuxer_Ffmpeg::Open()
{
    MMP_RESULT mmpResult = MMP_SUCCESS;
    //int ret;
    //AVStream *audio_st, *video_st;
    const int io_buffer_size = 32768;  /* FIXME */

    /* Muxer File Open */
    if(mmpResult == MMP_SUCCESS) {
        m_fp = fopen((const char*)m_create_config.filename, "wb");
        if(m_fp == NULL) {
            mmpResult = MMP_FAILURE;
            MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpMuxer_Ffmpeg::Open] FAIL: fopen %s "), m_create_config.filename));
        }
    }
   
    /* Create I/O wrapper and URL */
    if(mmpResult == MMP_SUCCESS) {

        m_IoBuffer = (MMP_U8*)MMP_MALLOC(io_buffer_size);
        if(m_IoBuffer == NULL)  {
            mmpResult = MMP_FAILURE;     
        }
        else {
            memset(m_IoBuffer, 0x00, io_buffer_size);
        }
            	
        m_Url = (URLContext*)MMP_MALLOC( sizeof(URLContext) );
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
    
        m_pAVIOContext = avio_alloc_context(m_IoBuffer, io_buffer_size, 1, m_Url, CMmpMuxer_Ffmpeg::IORead_C_Stub, CMmpMuxer_Ffmpeg::IOWrite_C_Stub, CMmpMuxer_Ffmpeg::IOSeek_C_Stub);
        if(m_pAVIOContext == NULL) {
            mmpResult = MMP_FAILURE;     
        }
    }

    /* allocate the output media context */
    if(mmpResult == MMP_SUCCESS) {
        avformat_alloc_output_context2(&m_oc, NULL, NULL, (const char*)m_create_config.filename);
        if(m_oc == NULL) {
            MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpMuxer_Ffmpeg::Open] FAIL: avformat_alloc_output_context2 ")));
            mmpResult = MMP_FAILURE;
            m_oc = NULL;
        }
        else {
            m_fmt = m_oc->oformat;
            m_fmt->video_codec = m_AVCodecID_Video;
            m_oc->pb = m_pAVIOContext; 
        }
    }

    /* add vide stream */
    if( (mmpResult == MMP_SUCCESS) 
      && (this->m_create_config.bMedia[MMP_MEDIATYPE_VIDEO] == MMP_TRUE) ) 
    {
        if(m_fmt->video_codec == AV_CODEC_ID_NONE) {
            MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpMuxer_Ffmpeg::Open] FAIL: not support video codec (%c%c%c%c)"),
                          MMPGETFOURCC(m_create_config.bih.biCompression, 0), MMPGETFOURCC(m_create_config.bih.biCompression, 1),
                          MMPGETFOURCC(m_create_config.bih.biCompression, 2), MMPGETFOURCC(m_create_config.bih.biCompression, 2)
                      ));
            mmpResult = MMP_FAILURE;
        }
        else {
            m_video_st = this->add_stream(m_oc, &m_video_codec, m_fmt->video_codec, MMP_MEDIATYPE_VIDEO);
            if(m_video_st == NULL) {
                MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpMuxer_Ffmpeg::Open] FAIL: add video stream")));
                mmpResult = MMP_FAILURE;
            }
        }
    }
    

#if 0
    frame->pts = 0;
    for (;;) {
        /* Compute current audio and video time. */
        audio_time = audio_st ? audio_st->pts.val * av_q2d(audio_st->time_base) : 0.0;
        video_time = video_st ? video_st->pts.val * av_q2d(video_st->time_base) : 0.0;
        if ((!audio_st || audio_time >= STREAM_DURATION) &&
        (!video_st || video_time >= STREAM_DURATION))
            break;
    
        /* write interleaved audio and video frames */
        if (!video_st || (video_st && audio_st && audio_time < video_time)) {
            write_audio_frame(oc, audio_st);
        } 
        else {
            write_video_frame(oc, video_st);
            frame->pts += av_rescale_q(1, video_st->codec->time_base, video_st->time_base);
        }
    }
#endif

    return mmpResult;
}

MMP_RESULT CMmpMuxer_Ffmpeg::Close()
{
    
    /* Write the trailer, if any. The trailer must be written before you
    * close the CodecContexts open when you wrote the header; otherwise
    * av_write_trailer() may try to use memory that was freed on
    * av_codec_close(). */

    if(m_oc != NULL) {

        if(m_oc->pb != NULL) {
            av_write_trailer(m_oc);

            /* Close each codec. */
            //if (video_st)
              //  close_video(oc, video_st);
            //if (audio_st)
              //  close_audio(oc, audio_st);

            //if (!(m_fmt->flags & AVFMT_NOFILE))
                /* Close the output file. */
             //   avio_close(m_oc->pb);
        }

        //static void close_video(AVFormatContext *oc, AVStream *st)
        if(m_video_st != NULL) {
            avcodec_close(m_video_st->codec);
            //av_free(src_picture.data[0]);
            //av_free(dst_picture.data[0]);
            //av_frame_free(&frame);
        }



        /* free the stream */
        avformat_free_context(m_oc);
        m_oc = NULL;
    }

    if(m_pAVIOContext != NULL) {
#if (MMP_OS == MMP_OS_WIN32)
        avio_close(m_pAVIOContext);
#endif   
        m_pAVIOContext = NULL;

        m_Url = NULL;
        m_IoBuffer = NULL;
    }


    if(m_IoBuffer != NULL) {
        MMP_FREE(m_IoBuffer);
        m_IoBuffer = NULL;
    }

    if(m_Url != NULL) {
        MMP_FREE(m_Url);
        m_Url = NULL;
    }

    if(m_fp != NULL) {
        fclose(m_fp);
        m_fp = NULL;
    }

#if 0
    /* need not free  here because Ffmpeg do free in it's lib */
    if(m_extra_data != NULL) {
        MMP_FREE(m_extra_data);
        m_extra_data = NULL;
    }
#endif

    return MMP_SUCCESS;
}

MMP_RESULT CMmpMuxer_Ffmpeg::AddMediaConfig(MMP_U32 mediatype, MMP_U8* buffer, MMP_U32 buf_size) {

    MMP_RESULT mmpResult = MMP_SUCCESS;
    AVStream* st;
    AVCodecContext *c;
    int ret;

    if(mediatype == MMP_MEDIATYPE_VIDEO) {
        st = this->m_video_st;
    }
    else {
        mmpResult = MMP_FAILURE;
    }

    /* alloc extra data */
    if(mmpResult == MMP_SUCCESS) {

        if(m_extra_data != NULL) {
            MMP_FREE(m_extra_data);
            m_extra_data = NULL;
            m_extra_data_size = 0;
        }
    
        if(buf_size > 0) {
            m_extra_data = (MMP_U8*)MMP_MALLOC(buf_size);
            if(m_extra_data == NULL) {
                mmpResult = MMP_FAILURE;
            }
            else {
                m_extra_data_size = buf_size;
                memcpy(m_extra_data, buffer, m_extra_data_size);
            }
        }
    }

    if(mmpResult == MMP_SUCCESS) {
        
        c = st->codec;
        c->extradata = m_extra_data;
        c->extradata_size = m_extra_data_size;

        ret = avformat_write_header(m_oc, NULL);
        if (ret < 0) {
            MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpMuxer_Ffmpeg::AddMediaConfig] FAIL: avformat_write_header")));
            mmpResult = MMP_FAILURE;
        }
        else {
            m_is_dsi_init[mediatype] = MMP_TRUE;
        }
    }

    return mmpResult;
}

MMP_RESULT CMmpMuxer_Ffmpeg::AddMediaData(MMP_U32 mediatype, MMP_U8* buffer, MMP_U32 buf_size, MMP_U32 flag, MMP_S64 pts) {

    MMP_RESULT mmpResult = MMP_SUCCESS;
    AVPacket avpkt;
    AVCodecContext *c;
    AVStream* st;
    int ret;
    MMP_S64 dts;

    if(mediatype == MMP_MEDIATYPE_VIDEO) {
        st = this->m_video_st;
    }
    else {
        mmpResult = MMP_FAILURE;
    }

    if(mmpResult == MMP_SUCCESS) {

        if(m_is_dsi_init[mediatype] == MMP_FALSE) {
            this->AddMediaConfig(mediatype, NULL, 0);
        }

        c = st->codec;

        av_init_packet (&avpkt);
        avpkt.data = buffer;
        avpkt.size = buf_size;
        if(flag&mmp_buffer_media::FLAG_VIDEO_KEYFRAME) {
            avpkt.flags = AV_PKT_FLAG_KEY;
        }
        else {
            avpkt.flags = 0;
        }
        
        this->set_last_input_pts(pts);

        //avpkt.pts = pts;
        //avpkt.dts = pts;
        //avpkt.duration = 0;
        if(pts > 30000L) {
            dts = pts-30000L;
        }
        else {
            dts = 0;
        }

        //(*packt_pts) = packet_timestamp  * 1000000 * s->time_base.num / s->time_base.den;
        avpkt.pts = pts *  st->time_base.den / (st->time_base.num * 1000000L);
        avpkt.dts = dts *  st->time_base.den / (st->time_base.num * 1000000L);

        //avpkt.pts = av_rescale_q_rnd(avpkt.pts, st->time_base, c->time_base, (enum AVRounding)(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));

        //avpkt.pts = av_rescale_q_rnd(avpkt.pts, st->time_base, c->time_base, (enum AVRounding)(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
        //avpkt.dts = av_rescale_q_rnd(avpkt.pts, st->time_base, c->time_base, (enum AVRounding)(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
        //avpkt.pts = pts;//av_rescale_q_rnd(pts, st->time_base, c->time_base, (enum AVRounding)(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
        //avpkt.dts = dts;//av_rescale_q_rnd(dts, st->time_base, c->time_base, (enum AVRounding)(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
        //avpkt.duration = (int)av_rescale_q(avpkt.duration, st->time_base, c->time_base);
        avpkt.stream_index = st->index;

        /* Write the compressed frame to the media file. */
        //log_packet(fmt_ctx, pkt);
        ret = av_interleaved_write_frame(m_oc, &avpkt);
        if(ret < 0) {
            mmpResult = MMP_FAILURE;
        }
        
    }

    return mmpResult;
}

int CMmpMuxer_Ffmpeg::IORead_C_Stub(void *opaque, uint8_t *buf, int buf_size) {

    URLContext *p_url = (URLContext*)opaque;
    CMmpMuxer_Ffmpeg *pObj = (CMmpMuxer_Ffmpeg *)p_url->priv_data;
    return pObj->IORead_C(opaque, buf, buf_size);
}

int CMmpMuxer_Ffmpeg::IOWrite_C_Stub(void *opaque, uint8_t *buf, int buf_size) {

    URLContext *p_url = (URLContext*)opaque;
    CMmpMuxer_Ffmpeg *pObj = (CMmpMuxer_Ffmpeg *)p_url->priv_data;
    return pObj->IOWrite_C(opaque, buf, buf_size);
}

int64_t CMmpMuxer_Ffmpeg::IOSeek_C_Stub(void *opaque, int64_t offset, int whence) {

    URLContext *p_url = (URLContext*)opaque;
    CMmpMuxer_Ffmpeg *pObj = (CMmpMuxer_Ffmpeg *)p_url->priv_data;
    return pObj->IOSeek_C(opaque, offset, whence);
}

int CMmpMuxer_Ffmpeg::IORead_C(void *opaque, uint8_t *buf, int buf_size) {

    int i_ret;
		
    if( buf_size < 0 ) return -1;

    i_ret = fread(buf, 1, buf_size, m_fp);

    return (i_ret >= 0) ? i_ret : -1;

}

int CMmpMuxer_Ffmpeg::IOWrite_C(void *opaque, uint8_t *buf, int buf_size) {

    int i_ret;
		
    if( buf_size < 0 ) return -1;

    i_ret = fwrite(buf, buf_size, 1, m_fp);

    return (i_ret >= 0) ? i_ret : -1;

}

int64_t CMmpMuxer_Ffmpeg::IOSeek_C(void *opaque, int64_t offset, int whence) {


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
            //m_nFileBufferIndex = 0;
            //m_nFileBufferSize = 0;
            break;
            
        default:
            return -1;

    }

    return (int64_t)ftell(m_fp);
}


/* Add an output stream. */
AVStream* CMmpMuxer_Ffmpeg::add_stream(AVFormatContext *oc, AVCodec **codec, enum AVCodecID codec_id, enum MMP_MEDIATYPE mediatype)
{
    AVCodecContext *c = NULL;
    AVStream *st = NULL;
    MMP_S32 err_cnt = 0;

    /* find the encoder */
    if(err_cnt == 0) {
        *codec = avcodec_find_encoder(codec_id);
        if(!(*codec)) {
            //fprintf(stderr, "Could not find encoder for '%s'\n",
            //avcodec_get_name(codec_id));
            //exit(1);
            err_cnt++;
        }
    }

    /* create new stream */
    if(err_cnt == 0) {
        st = avformat_new_stream(oc, *codec);
        if (!st) {
            //fprintf(stderr, "Could not allocate stream\n");
            //exit(1);
            err_cnt++;
        }
    }

    /* setting codec */
    if(err_cnt == 0) {

        st->id = oc->nb_streams-1;
        c = st->codec;
        switch (mediatype) {
            case MMP_MEDIATYPE_AUDIO:
                c->sample_fmt = (*codec)->sample_fmts ?
                (*codec)->sample_fmts[0] : AV_SAMPLE_FMT_FLTP;
                c->bit_rate = 64000;
                c->sample_rate = 44100;
                c->channels = 2;
                break;

            case MMP_MEDIATYPE_VIDEO:
                c->codec_id = codec_id;
                c->bit_rate = 400000;
                /* Resolution must be a multiple of two. */
                c->width = this->m_create_config.bih.biWidth;
                c->height = this->m_create_config.bih.biHeight;
                /* timebase: This is the fundamental unit of time (in seconds) in terms
                * of which frame timestamps are represented. For fixed-fps content,
                * timebase should be 1/framerate and timestamp increments should be
                * identical to 1. */
                c->time_base.den = this->m_create_config.video_fps;//30;//STREAM_FRAME_RATE;
                c->time_base.num = 1;
                c->gop_size = this->m_create_config.video_idr_period;//12; /* emit one intra frame every twelve frames at most */
                c->pix_fmt = STREAM_PIX_FMT;
                if (c->codec_id == AV_CODEC_ID_MPEG2VIDEO) {
                    /* just for testing, we also add B frames */
                    c->max_b_frames = 2;
                }

                if (c->codec_id == AV_CODEC_ID_MPEG1VIDEO) {
                    /* Needed to avoid using macroblocks in which some coeffs overflow.
                    * This does not happen with normal video, it just happens here as
                    * the motion of the chroma plane does not match the luma plane. */
                    c->mb_decision = 2;
                }
                break;
        
            default:
                break;
        }
        /* Some formats want stream headers to be separate. */
        if (oc->oformat->flags & AVFMT_GLOBALHEADER) {
                c->flags |= CODEC_FLAG_GLOBAL_HEADER;
        }

    }

    return st;
}

#endif
