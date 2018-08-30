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

#include "MmpDecoderFfmpeg.hpp"
#include "MmpUtil.hpp"
#include "mmp_video_tool.hpp"
#include "mmp_audio_tool.hpp"

#if (FFMPEG_VERSION == FFMPEG_VERSION_2_6_1)


#if (MMP_OS == MMP_OS_WIN32) /* ffmpeg win32 mingw parameter..*/
//extern "C" int __chkstk_ms=0;
//extern "C" int __divdi3 = 0;
#endif

/////////////////////////////////////////////////////////////
//CMmpDecoderFfmpeg Member Functions

#define AV_CODEC_ID_FFMPEG    MKBETAG('F','F','M','P')

CMmpDecoderFfmpeg::CMmpDecoderFfmpeg(enum MMP_FOURCC fourcc_in) :
m_pAVCodec(NULL)
,m_pAVCodecContext(NULL)
,m_pAVFrame_Decoded(NULL)
,m_extra_data(NULL)

{
    avcodec_register_all();

    switch(fourcc_in) {

        /* Audio */
        case MMP_FOURCC_AUDIO_MP3: m_AVCodecID = AV_CODEC_ID_MP3; break;
        case MMP_FOURCC_AUDIO_MP2: m_AVCodecID = AV_CODEC_ID_MP2; break;
        //case MMP_WAVE_FORMAT_MPEGLAYER1: m_AVCodecID = AV_CODEC_ID_MP1; break;
        case MMP_FOURCC_AUDIO_WMA1: m_AVCodecID = AV_CODEC_ID_WMAV1; break;
        case MMP_FOURCC_AUDIO_WMA2: m_AVCodecID = AV_CODEC_ID_WMAV2; break;
        case MMP_FOURCC_AUDIO_AC3: m_AVCodecID = AV_CODEC_ID_AC3; break;
        case MMP_FOURCC_AUDIO_AAC: m_AVCodecID = AV_CODEC_ID_AAC; break;
        //case MMP_WAVE_FORMAT_FLAC: m_AVCodecID = AV_CODEC_ID_FLAC; break;
        //case MMP_WAVE_FORMAT_ADPCM_MS: m_AVCodecID = AV_CODEC_ID_ADPCM_MS; break;
        case MMP_FOURCC_AUDIO_FFMPEG: m_AVCodecID = (AVCodecID)AV_CODEC_ID_FFMPEG; break;

        /* Video */
        case MMP_FOURCC_VIDEO_H263: m_AVCodecID=AV_CODEC_ID_H263; break;
        case MMP_FOURCC_VIDEO_H264: m_AVCodecID=AV_CODEC_ID_H264; break;
        case MMP_FOURCC_VIDEO_MPEG4: m_AVCodecID=AV_CODEC_ID_MPEG4; break;
        case MMP_FOURCC_VIDEO_MPEG2: m_AVCodecID=AV_CODEC_ID_MPEG2VIDEO; break;

        /* VPU support Only WMV3 */
        //case MMP_FOURCC_VIDEO_WMV1: m_AVCodecID=AV_CODEC_ID_WMV1; break;
        case MMP_FOURCC_VIDEO_WMV2: m_AVCodecID=AV_CODEC_ID_WMV2; break;
        case MMP_FOURCC_VIDEO_WMV3: m_AVCodecID=AV_CODEC_ID_WMV3; break;
        case MMP_FOURCC_VIDEO_WVC1: m_AVCodecID=AV_CODEC_ID_VC1; break; /* VC1 Advacned@L3 */

        case MMP_FOURCC_VIDEO_MSS1: m_AVCodecID=AV_CODEC_ID_MSS1; break;
        case MMP_FOURCC_VIDEO_MSS2: m_AVCodecID=AV_CODEC_ID_MSS2; break;

        case MMP_FOURCC_VIDEO_VP80: m_AVCodecID=AV_CODEC_ID_VP8; break;
        case MMP_FOURCC_VIDEO_VP60: m_AVCodecID=AV_CODEC_ID_VP6; break;
        case MMP_FOURCC_VIDEO_VP6F: m_AVCodecID=AV_CODEC_ID_VP6F; break;
        case MMP_FOURCC_VIDEO_VP6A: m_AVCodecID=AV_CODEC_ID_VP6A; break;

        case MMP_FOURCC_VIDEO_RV30: m_AVCodecID=AV_CODEC_ID_RV30; break;
        case MMP_FOURCC_VIDEO_RV40: m_AVCodecID=AV_CODEC_ID_RV40; break;
        case MMP_FOURCC_VIDEO_RV:  m_AVCodecID=AV_CODEC_ID_RV40;  break;

        case MMP_FOURCC_VIDEO_SVQ1: m_AVCodecID=AV_CODEC_ID_SVQ1; break;
        case MMP_FOURCC_VIDEO_SVQ3: m_AVCodecID=AV_CODEC_ID_SVQ3; break;

        case MMP_FOURCC_VIDEO_THEORA: m_AVCodecID=AV_CODEC_ID_THEORA; break;
        case MMP_FOURCC_VIDEO_MJPEG: m_AVCodecID=AV_CODEC_ID_MJPEG; break;
        case MMP_FOURCC_VIDEO_FLV1: m_AVCodecID=AV_CODEC_ID_FLV1; break;
        //case MMP_FOURCC_VIDEO_MSMPEG4V1: m_AVCodecID=AV_CODEC_ID_MSMPEG4V1; break;
        //case MMP_FOURCC_VIDEO_MSMPEG4V2: m_AVCodecID=AV_CODEC_ID_MSMPEG4V2; break;
        case MMP_FOURCC_VIDEO_MSMPEG4V3: m_AVCodecID=AV_CODEC_ID_MSMPEG4V3; break;


        case MMP_FOURCC_VIDEO_FFMPEG: m_AVCodecID = (AVCodecID)AV_CODEC_ID_FFMPEG; break;
        case MMP_FOURCC_AUDIO_CODEC_PCM_S16LE: m_AVCodecID = (AVCodecID)AV_CODEC_ID_PCM_S16LE; break;

        default:
            m_AVCodecID = AV_CODEC_ID_NONE;
    }
}

CMmpDecoderFfmpeg::~CMmpDecoderFfmpeg()
{

}

MMP_RESULT CMmpDecoderFfmpeg::Open()
{
    MMP_RESULT mmpResult = MMP_SUCCESS;

    if(m_AVCodecID == AV_CODEC_ID_NONE) {
        mmpResult = MMP_FAILURE;
    }

    return mmpResult;
}


MMP_RESULT CMmpDecoderFfmpeg::Close()
{
    if(this->m_pAVCodecContext != NULL) {
        avcodec_close(this->m_pAVCodecContext);
        av_free(this->m_pAVCodecContext);
        this->m_pAVCodecContext = NULL;
    }

    if(m_pAVFrame_Decoded != NULL) {
        avcodec_free_frame(&m_pAVFrame_Decoded);
        m_pAVFrame_Decoded = NULL;
    }

    if(m_extra_data != NULL) {
        delete [] m_extra_data;
        m_extra_data = NULL;
    }

    return MMP_SUCCESS;
}

MMP_RESULT CMmpDecoderFfmpeg::DecodeDSI(MMP_U8* pStream, MMP_U32 nStreamSize) {

    return MMP_FAILURE;
}

MMP_RESULT CMmpDecoderFfmpeg::DecodeDSI(class mmp_buffer_audiostream* p_buf_as) {

    AVCodec *codec;
    AVCodecContext *cc= NULL;
    MMP_BOOL is_audio_packet_hdr = MMP_FALSE;
    struct mmp_audio_packet_header *p_audio_pkt_hdr;
    MMP_ADDR pkt_addr, ffmpeg_hdr_addr, dsi_data_addr;
    void* ptr;


    is_audio_packet_hdr = mmp_audio_tool::is_valid_audio_packet_header(p_buf_as->get_buf_vir_addr(), p_buf_as->get_stream_size());
    if(is_audio_packet_hdr == MMP_TRUE) {

        pkt_addr = (MMP_ADDR)p_buf_as->get_buf_vir_addr();
        p_audio_pkt_hdr = (struct mmp_audio_packet_header *)pkt_addr;
        dsi_data_addr = pkt_addr + p_audio_pkt_hdr->dsi_pos;
        ffmpeg_hdr_addr = pkt_addr + p_audio_pkt_hdr->ffmpeg_hdr_pos;

        if(m_AVCodecID == (AVCodecID)AV_CODEC_ID_FFMPEG) {
            AVCodecContext *cc1 = (AVCodecContext *)ffmpeg_hdr_addr;
            m_AVCodecID = cc1->codec_id;
        }
    }

    codec = avcodec_find_decoder(m_AVCodecID);
    if(codec == NULL) {
        MMPDEBUGMSG(MMPZONE_ERROR, (TEXT(MMP_CTAG" FAIL: avcodec_find_decoder   m_AVCodecID=%d "), MMP_CNAME, m_AVCodecID ));
        return MMP_FAILURE;
    }

    cc= avcodec_alloc_context3(codec);

    if(m_extra_data != NULL) {
        delete [] m_extra_data;
        m_extra_data = NULL;
    }


    if(is_audio_packet_hdr == MMP_TRUE) {

        if(p_audio_pkt_hdr->ffmpeg_hdr_size == sizeof(AVCodecContext)) {
            memcpy(cc, (void*)ffmpeg_hdr_addr, sizeof(AVCodecContext));
        }

        m_extra_data = new MMP_U8[p_audio_pkt_hdr->dsi_size];
        memcpy(m_extra_data, (void*)dsi_data_addr, p_audio_pkt_hdr->dsi_size);
        cc->extradata = m_extra_data;
        cc->extradata_size = p_audio_pkt_hdr->dsi_size;
    }
    else {

        if(p_buf_as->get_ffmpeg_codec_context() != NULL) {
            switch(m_AVCodecID) {

                case AV_CODEC_ID_AC3:
                    break;

                case AV_CODEC_ID_WMAV2:
                default:
                    ptr = cc->priv_data;
                    memcpy(cc, p_buf_as->get_ffmpeg_codec_context(), sizeof(AVCodecContext));
                    cc->priv_data = ptr;
                    break;
            }
        }

        m_extra_data = new MMP_U8[p_buf_as->get_stream_size()];
        memcpy(m_extra_data, (void*)p_buf_as->get_buf_vir_addr(), p_buf_as->get_stream_size());
        cc->extradata = m_extra_data;
        cc->extradata_size = p_buf_as->get_stream_size();
    }


    /* open it */
    if(avcodec_open2(cc, codec, NULL) < 0)
    {
        MMPDEBUGMSG(MMPZONE_ERROR, (TEXT(MMP_CTAG"FAIL: could not open codec\n\r"), MMP_CNAME));
        return MMP_FAILURE;
    }

    m_pAVCodec = codec;
    m_pAVCodecContext = cc;
    m_pAVFrame_Decoded = av_frame_alloc();

    return MMP_SUCCESS;
}

extern "C" int my_get_buffer2(struct AVCodecContext *s, AVFrame *frame, int flags) {

    return 0;
}

extern "C" int ffmpeg_GetFrameBuf( struct AVCodecContext *p_context,  AVFrame *p_ff_pic )
{
#if 0
    p_ff_pic->opaque = NULL;
    p_ff_pic->type = FF_BUFFER_TYPE_USER;
    p_ff_pic->data[0] = new unsigned char[320*240*2];
    p_ff_pic->data[1] = new unsigned char[320*240*2];
    p_ff_pic->data[2] = new unsigned char[320*240*2];
    p_ff_pic->data[3] = NULL; /* alpha channel but I'm not sure */

    p_ff_pic->linesize[0] = 352;
    p_ff_pic->linesize[1] = 352/2;
    p_ff_pic->linesize[2] = 352/2;
    p_ff_pic->linesize[3] = 0;

    return 0;
#else
   //return avcodec_default_get_buffer(p_context, p_ff_pic);
    return CMmpDecoderFfmpeg::ffmpeg_get_buffer_cb_stub(p_context, p_ff_pic);
#endif
}

extern "C" void ffmpeg_ReleaseFrameBuf( struct AVCodecContext *p_context,  AVFrame *p_ff_pic )
{

#if 0
    if( p_ff_pic->type == FF_BUFFER_TYPE_INTERNAL )
        /* We can end up here without the AVFrame being allocated by
         * avcodec_default_get_buffer() if VA is used and the frame is
         * released when the decoder is closed
         */
        avcodec_default_release_buffer( p_context, p_ff_pic );
    else if( p_ff_pic->type == FF_BUFFER_TYPE_USER ) {
        delete [] p_ff_pic->data[0];
        delete [] p_ff_pic->data[1];
        delete [] p_ff_pic->data[2];
    }
#else
    //avcodec_default_release_buffer(p_context, p_ff_pic);
    CMmpDecoderFfmpeg::ffmpeg_release_buffer_cb_stub(p_context, p_ff_pic);
#endif
    //for( int i = 0; i < 4; i++ )
      //  p_ff_pic->data[i] = NULL;
}


int CMmpDecoderFfmpeg::ffmpeg_get_buffer_cb_stub( struct AVCodecContext *p_context,  AVFrame *p_ff_pic ) {
    CMmpDecoderFfmpeg* pFfmpeg = (CMmpDecoderFfmpeg*)p_context->opaque;
    return pFfmpeg->ffmpeg_get_buffer_cb(p_context, p_ff_pic);
}

void CMmpDecoderFfmpeg::ffmpeg_release_buffer_cb_stub( struct AVCodecContext *p_context,  AVFrame *p_ff_pic ) {
    CMmpDecoderFfmpeg* pFfmpeg = (CMmpDecoderFfmpeg*)p_context->opaque;
    pFfmpeg->ffmpeg_release_buffer_cb(p_context, p_ff_pic);
}

int CMmpDecoderFfmpeg::ffmpeg_get_buffer_cb(struct AVCodecContext *p_context,  AVFrame *p_ff_pic) {
    return avcodec_default_get_buffer(p_context, p_ff_pic);
}

void CMmpDecoderFfmpeg::ffmpeg_release_buffer_cb(struct AVCodecContext *p_context,  AVFrame *p_ff_pic) {
    avcodec_default_release_buffer( p_context, p_ff_pic );
}

MMP_RESULT CMmpDecoderFfmpeg::DecodeDSI(class mmp_buffer_videostream* p_buf_vs, MMP_S32 pic_width, MMP_S32 pic_height ) {

    MMP_RESULT mmpResult = MMP_SUCCESS;
    AVCodec *codec;
    AVCodecContext *cc= NULL;
    void* ptr;
    MMP_BOOL is_video_packet_hdr = MMP_FALSE;
    struct mmp_video_packet_header *p_video_pkt_hdr;
    MMP_ADDR pkt_addr, ffmpeg_hdr_addr, dsi_data_addr;

    is_video_packet_hdr = mmp_video_tool::is_valid_video_packet_header(p_buf_vs->get_buf_vir_addr(), p_buf_vs->get_stream_size());
    if(is_video_packet_hdr == MMP_TRUE) {

        pkt_addr = (MMP_ADDR)p_buf_vs->get_buf_vir_addr();
        p_video_pkt_hdr = (struct mmp_video_packet_header *)pkt_addr;
        dsi_data_addr = pkt_addr + p_video_pkt_hdr->dsi_pos;
        ffmpeg_hdr_addr = pkt_addr + p_video_pkt_hdr->ffmpeg_hdr_pos;

        if(m_AVCodecID == (AVCodecID)AV_CODEC_ID_FFMPEG) {
            AVCodecContext *cc1 = (AVCodecContext *)ffmpeg_hdr_addr;
            m_AVCodecID = cc1->codec_id;
        }
    }
    else if(m_AVCodecID == AV_CODEC_ID_RV40) {

        MMP_U32* extra_data_rv;
        MMP_U32  key;

        extra_data_rv = (MMP_U32*)p_buf_vs->get_stream_real_ptr();
        key = extra_data_rv[0];

        if(key == MMP_RV_EXTRA_DATA_KEY) {
            if(extra_data_rv[1] == 3) {
                m_AVCodecID = AV_CODEC_ID_RV30;
            }
        }

        MMPDEBUGMSG(1, (TEXT("extra_data_rv[0] = 0x%08x "), extra_data_rv[0] ));

    }

    codec = avcodec_find_decoder(m_AVCodecID);
    if(codec == NULL) {
        return MMP_FAILURE;
    }


    cc= avcodec_alloc_context3(codec);

    if(m_extra_data != NULL) {
        delete [] m_extra_data;
        m_extra_data = NULL;
    }

    if(is_video_packet_hdr  == MMP_TRUE) {

        /* copy ffmpeg hdr */
        if(p_video_pkt_hdr->ffmpeg_hdr_size == sizeof(AVCodecContext) ) {

            switch(m_AVCodecID) {
                case AV_CODEC_ID_H264:
                case AV_CODEC_ID_MPEG4:
                case AV_CODEC_ID_MPEG2VIDEO:
                    break;
                case AV_CODEC_ID_WMV3:
                default:
                    ptr = cc->priv_data;
                    memcpy(cc, (void*)ffmpeg_hdr_addr, sizeof(AVCodecContext));
                    cc->priv_data = ptr;
                    break;
            }
        }

        if(p_video_pkt_hdr->dsi_size > 0) {
            m_extra_data = new MMP_U8[p_video_pkt_hdr->dsi_size];
            memcpy(m_extra_data, (void*)dsi_data_addr, p_video_pkt_hdr->dsi_size);
            cc->extradata = m_extra_data;
            cc->extradata_size = p_video_pkt_hdr->dsi_size;
        }
        else {
            cc->extradata = NULL;
            cc->extradata_size = 0;
        }
    }
    else {

        if(p_buf_vs->get_ffmpeg_codec_context() != NULL) {
            switch(m_AVCodecID) {

                case AV_CODEC_ID_H264:
                case AV_CODEC_ID_MPEG4:
                case AV_CODEC_ID_MPEG2VIDEO:
                case AV_CODEC_ID_WMV3:
                case AV_CODEC_ID_VC1:
                case AV_CODEC_ID_RV30:
                case AV_CODEC_ID_RV40:
                    break;

                default:
                    memcpy(cc, p_buf_vs->get_ffmpeg_codec_context(), sizeof(AVCodecContext));
                    break;
            }
        }

        cc->width = pic_width;
        cc->height = pic_height;

        if(p_buf_vs->get_stream_size() > 0) {

            MMP_U8* extra_data = (MMP_U8*)p_buf_vs->get_stream_real_ptr();
            MMP_S32 extra_data_size = p_buf_vs->get_stream_real_size();

            switch(m_AVCodecID) {
                case AV_CODEC_ID_RV30:
                case AV_CODEC_ID_RV40:
                    extra_data += 8;
                    extra_data_size -= 8;
                    break;
            }

            m_extra_data = new MMP_U8[extra_data_size];
            memcpy(m_extra_data, (void*)extra_data, extra_data_size);
            cc->extradata = m_extra_data;
            cc->extradata_size = extra_data_size;
        }
        else {
            cc->extradata = NULL;
            cc->extradata_size = 0;
        }
    }


    /* open it */
    if(avcodec_open2(cc, codec, NULL) < 0)  {

        MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpDecoderFfmpeg::DecodeDSI] FAIL: could not open codec\n\r")));
        mmpResult = MMP_FAILURE;
    }
    else {
        m_pAVCodec = codec;
        m_pAVCodecContext = cc;
        m_pAVFrame_Decoded = avcodec_alloc_frame();

#if 0
#ifdef WIN32
        avcodec_open2_post_proc_for_win32(m_pAVCodecContext, this);
#else
        cc->opaque = this;
        cc->get_buffer = CMmpDecoderFfmpeg::ffmpeg_get_buffer_cb_stub;
        cc->reget_buffer = avcodec_default_reget_buffer;
        cc->release_buffer = CMmpDecoderFfmpeg::ffmpeg_release_buffer_cb_stub;
#endif
#endif
    }

    if(mmpResult != MMP_SUCCESS) {
        MMP_U8* pdata = (MMP_U8*)p_buf_vs->get_buf_vir_addr();
        MMP_S32 dsisz = p_buf_vs->get_stream_size();
        MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpDecoderFfmpeg::DecodeDSI] dsisz : %d  dsi(0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x "),
                  dsisz,
                  pdata[0], pdata[1], pdata[2], pdata[3],
                  pdata[4], pdata[5], pdata[6], pdata[7]
                  ));

    }

    return mmpResult;
}

#endif