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

#include "MmpDecoderVideo_Ffmpeg.hpp"
#include "MmpUtil.hpp"
#include "mmp_buffer_mgr.hpp"
#include "MmpH264Tool.hpp"
#include "MmpImageTool.hpp"

#if (FFMPEG_VERSION == FFMPEG_VERSION_2_6_1)

/////////////////////////////////////////////////////////////
//CMmpDecoderVideo_Ffmpeg Member Functions

static const enum MMP_FOURCC s_support_fourcc_in[]={

     MMP_FOURCC_VIDEO_H264,

     MMP_FOURCC_VIDEO_MPEG4,
     MMP_FOURCC_VIDEO_MPEG2,

     MMP_FOURCC_VIDEO_WMV2,
     MMP_FOURCC_VIDEO_WMV3,

     MMP_FOURCC_VIDEO_RV30,
     MMP_FOURCC_VIDEO_RV40,
     MMP_FOURCC_VIDEO_RV,

     MMP_FOURCC_VIDEO_VP80,

     MMP_FOURCC_VIDEO_H263,

     MMP_FOURCC_VIDEO_FLV1, /* sorenson spark */
     MMP_FOURCC_VIDEO_THEORA,

     MMP_FOURCC_VIDEO_MSMPEG4V3, /* only support msmpeg4v3 */

     /* VPU Support Only WMV3 */
     //MMP_FOURCC_VIDEO_VC1,
     //MMP_FOURCC_VIDEO_WMV1,



     MMP_FOURCC_VIDEO_FFMPEG,
};

static const enum MMP_FOURCC s_support_fourcc_out[]={
     MMP_FOURCC_IMAGE_YUV420,
     MMP_FOURCC_IMAGE_YUV420M,
     MMP_FOURCC_IMAGE_YVU420,
     MMP_FOURCC_IMAGE_YVU420M,
     MMP_FOURCC_IMAGE_YUV420M_META,
};

CMmpDecoderVideo_Ffmpeg::CMmpDecoderVideo_Ffmpeg(struct CMmpDecoderVideo::create_config *p_create_config) :

CMmpDecoderVideo(p_create_config, "Ffmpeg",
                 s_support_fourcc_in, sizeof(s_support_fourcc_in)/sizeof(s_support_fourcc_in[0]),
                 s_support_fourcc_out, sizeof(s_support_fourcc_out)/sizeof(s_support_fourcc_out[0]) )
,CMmpDecoderFfmpeg(p_create_config->fourcc_in)

,m_decoded_picture_count(0)
,m_enable_avcopy(MMP_TRUE)

{

}

CMmpDecoderVideo_Ffmpeg::~CMmpDecoderVideo_Ffmpeg()
{

}

MMP_RESULT CMmpDecoderVideo_Ffmpeg::Open()
{
    MMP_RESULT mmpResult = MMP_SUCCESS;

    if(mmpResult == MMP_SUCCESS) {
        mmpResult=CMmpDecoderVideo::Open();
    }

    if(mmpResult == MMP_SUCCESS) {
        mmpResult=CMmpDecoderFfmpeg::Open();
    }

    //if(mmpResult == MMP_SUCCESS) {
    //    MMPDEBUGMSG(MMPZONE_MONITOR, (TEXT("[CMmpDecoderVideo_Ffmpeg::Open] Success in=%s out=%s \n\r"),  this->get_fourcc_in_name(), this->get_fourcc_out_name() ));
    //}

    return mmpResult;
}


MMP_RESULT CMmpDecoderVideo_Ffmpeg::Close()
{
    MMP_RESULT mmpResult;

    mmpResult=CMmpDecoderFfmpeg::Close();
    if(mmpResult!=MMP_SUCCESS) {
        MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpDecoderVideo_Ffmpeg::Close] CMmpDecoderFfmpeg::Close() \n\r")));
        return mmpResult;
    }

    mmpResult=CMmpDecoderVideo::Close();
    if(mmpResult!=MMP_SUCCESS) {
        MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpDecoderVideo_Ffmpeg::Close] CMmpDecoderVideo::Close() \n\r")));
        return mmpResult;
    }


    return MMP_SUCCESS;
}

#if (MMPDECODERVIDEO_DIRECT_RENDER == 1)

#if 1
int CMmpDecoderVideo_Ffmpeg::ffmpeg_get_buffer_cb(struct AVCodecContext *p_context,  AVFrame *p_ff_pic) {

    class mmp_buffer_videoframe* p_buf_vf_decoded = NULL;
    p_buf_vf_decoded = this->get_next_vf();
    int iret = -1;

    if(p_buf_vf_decoded != NULL) {

        MMP_S32 pic_width, pic_height;
        enum MMP_FOURCC fourcc_decoded_buf;

        pic_width = p_buf_vf_decoded->get_pic_width();
        pic_height = p_buf_vf_decoded->get_pic_height();
        fourcc_decoded_buf = p_buf_vf_decoded->get_fourcc();

        p_ff_pic->opaque = NULL;
        p_ff_pic->type = FF_BUFFER_TYPE_USER;
        p_ff_pic->data[0] = (uint8_t*)p_buf_vf_decoded->get_buf_vir_addr();
        p_ff_pic->data[1] = (uint8_t*)p_ff_pic->data[0] + mmp_buffer_get_u_offset(fourcc_decoded_buf, pic_width, pic_height);
        p_ff_pic->data[2] = (uint8_t*)p_ff_pic->data[0] + mmp_buffer_get_v_offset(fourcc_decoded_buf, pic_width, pic_height);
        p_ff_pic->data[3] = NULL; /* alpha channel but I'm not sure */

        p_ff_pic->linesize[0] = mmp_buffer_get_y_stride(fourcc_decoded_buf, pic_width);
        p_ff_pic->linesize[1] = mmp_buffer_get_u_stride(fourcc_decoded_buf, pic_width);
        p_ff_pic->linesize[2] = mmp_buffer_get_v_stride(fourcc_decoded_buf, pic_width);
        p_ff_pic->linesize[3] = 0;

        iret = 0;
    }
    else {
        //iret = avcodec_default_get_buffer(p_context, p_ff_pic);

        MMP_S32 pic_width, pic_height;
        MMP_S32 ffmpeg_buf_width, ffmpeg_buf_height;
        MMP_S32 ffmpeg_buf_size;
        enum MMP_FOURCC fourcc_decoded_buf = MMP_FOURCC_IMAGE_YVU420;

        pic_width = p_ff_pic->width;
        ffmpeg_buf_width = mmp_buffer_get_y_stride(fourcc_decoded_buf, pic_width);

        pic_height = p_ff_pic->height;
        ffmpeg_buf_height = pic_height + 32;

        ffmpeg_buf_size = ffmpeg_buf_width*ffmpeg_buf_height;

        p_ff_pic->opaque = NULL;
        p_ff_pic->type = FF_BUFFER_TYPE_USER;
        p_ff_pic->data[0] = (uint8_t*)malloc(ffmpeg_buf_size);
        p_ff_pic->data[1] = (uint8_t*)malloc(ffmpeg_buf_size);
        p_ff_pic->data[2] = (uint8_t*)malloc(ffmpeg_buf_size);
        p_ff_pic->data[3] = NULL; /* alpha channel but I'm not sure */

        //MMPDEBUGMSG(1, (TEXT("Alloc 0x%08x 0x%08x 0x%08x "), p_ff_pic->data[0], p_ff_pic->data[1], p_ff_pic->data[2] ));

        p_ff_pic->linesize[0] = mmp_buffer_get_y_stride(fourcc_decoded_buf, pic_width);
        p_ff_pic->linesize[1] = mmp_buffer_get_u_stride(fourcc_decoded_buf, pic_width);
        p_ff_pic->linesize[2] = mmp_buffer_get_v_stride(fourcc_decoded_buf, pic_width);
        p_ff_pic->linesize[3] = 0;


        iret = 0;
    }

    return iret;
}

void CMmpDecoderVideo_Ffmpeg::ffmpeg_release_buffer_cb(struct AVCodecContext *p_context,  AVFrame *p_ff_pic) {

    if( p_ff_pic->type == FF_BUFFER_TYPE_INTERNAL ) {
        avcodec_default_release_buffer( p_context, p_ff_pic );
    }
}

#else
int CMmpDecoderVideo_Ffmpeg::ffmpeg_get_buffer_cb(struct AVCodecContext *p_context,  AVFrame *p_ff_pic) {

    MMP_S32 pic_width, pic_height;
    MMP_S32 ffmpeg_buf_width, ffmpeg_buf_height;
    MMP_S32 ffmpeg_buf_size;

    pic_width = p_ff_pic->width;
    ffmpeg_buf_width = pic_width + 32;

    pic_height = p_ff_pic->height;
    ffmpeg_buf_height = pic_height;

    ffmpeg_buf_size = ffmpeg_buf_width*ffmpeg_buf_height;

    p_ff_pic->opaque = NULL;
    p_ff_pic->type = FF_BUFFER_TYPE_USER;
    p_ff_pic->data[0] = (uint8_t*)malloc(ffmpeg_buf_size);
    p_ff_pic->data[1] = (uint8_t*)malloc(ffmpeg_buf_size);
    p_ff_pic->data[2] = (uint8_t*)malloc(ffmpeg_buf_size);
    p_ff_pic->data[3] = NULL; /* alpha channel but I'm not sure */

    //MMPDEBUGMSG(1, (TEXT("Alloc 0x%08x 0x%08x 0x%08x "), p_ff_pic->data[0], p_ff_pic->data[1], p_ff_pic->data[2] ));

    p_ff_pic->linesize[0] = ffmpeg_buf_width;
    p_ff_pic->linesize[1] = ffmpeg_buf_width/2;
    p_ff_pic->linesize[2] = ffmpeg_buf_width/2;
    p_ff_pic->linesize[3] = 0;

    return 0;
}

void CMmpDecoderVideo_Ffmpeg::ffmpeg_release_buffer_cb(struct AVCodecContext *p_context,  AVFrame *p_ff_pic) {

    avcodec_default_release_buffer( p_context, p_ff_pic );

#if 0
    if( p_ff_pic->type == FF_BUFFER_TYPE_INTERNAL )
        /* We can end up here without the AVFrame being allocated by
         * avcodec_default_get_buffer() if VA is used and the frame is
         * released when the decoder is closed
         */
        avcodec_default_release_buffer( p_context, p_ff_pic );

    else if( p_ff_pic->type == FF_BUFFER_TYPE_USER ) {
        //free(p_ff_pic->data[0]);
        //free(p_ff_pic->data[1]);
        //free(p_ff_pic->data[2]);
    }

    for( int i = 0; i < 4; i++ )
        p_ff_pic->data[i] = NULL;
#endif
}
#endif

#else
int CMmpDecoderVideo_Ffmpeg::ffmpeg_get_buffer_cb(struct AVCodecContext *p_context,  AVFrame *p_ff_pic) {
	av_frame_unref(p_ff_pic);

	return 0;
}

void CMmpDecoderVideo_Ffmpeg::ffmpeg_release_buffer_cb(struct AVCodecContext *p_context,  AVFrame *p_ff_pic) {
//    avcodec_default_release_buffer( p_context, p_ff_pic );
}
#endif

MMP_RESULT CMmpDecoderVideo_Ffmpeg::flush_buffer_in() {

    MMP_S64 pts;

    while(!m_minheap_ts.IsEmpty()) {
        m_minheap_ts.Delete(pts);
    }

    return MMP_SUCCESS;
}


MMP_RESULT CMmpDecoderVideo_Ffmpeg::DecodeDSI(class mmp_buffer_videostream* p_buf_vs) {

    MMP_RESULT mmpResult;
    enum MMP_FOURCC fourcc_in, fourcc_out;

    mmpResult = CMmpDecoderFfmpeg::DecodeDSI(p_buf_vs, this->get_decoder_pic_width(), this->get_decoder_pic_height() );
    if(mmpResult == MMP_SUCCESS) {

        fourcc_in = this->get_fourcc_in();
        fourcc_out = this->get_fourcc_out();

        MMPDEBUGMSG(MMPZONE_MONITOR, (TEXT("[VideoDec::Ffmpeg] DSI OK  in=%s out=%s"),  this->get_fourcc_in_name(), this->get_fourcc_out_name()  ));

    }

    return mmpResult;

}

MMP_RESULT CMmpDecoderVideo_Ffmpeg::DecodeAu(class mmp_buffer_videostream* p_buf_vs, class mmp_buffer_videoframe** pp_buf_vf, enum ACTION *next_action) {

    MMP_RESULT mmpResult = MMP_SUCCESS;
    MMP_S32 i;
    int32_t frameFinished = 192000 * 2;
    int32_t usebyte;
    AVPacket avpkt;

    MMP_U32 dec_start_tick, dec_end_tick;
    class mmp_buffer_videoframe* p_buf_vf_decoded = NULL;

    MMP_S32 vs_size = p_buf_vs->get_stream_real_size();
    MMP_S32 vs_size_sub;
    MMP_U8* p_stream_buf;
    MMP_S64 pts;

    dec_start_tick = CMmpUtil::GetTickCount();


    *next_action = ACTION_NONE;

    /* Init Parmeter */
    if(pp_buf_vf != NULL) {
        *pp_buf_vf = NULL;
    }

    /* Decode DSI */
    if(m_pAVCodec == NULL) {
        mmpResult = this->DecodeDSI(p_buf_vs);
        if(mmpResult != MMP_SUCCESS) {
            return mmpResult;
        }
    }

    /* Decode DSI */
    if((p_buf_vs->get_flag()&mmp_buffer_media::FLAG_CONFIGDATA) != 0) {
        *next_action = ACTION_RECONFIG;
        this->init_buf_vf_from_decoder(m_pAVCodecContext->width, m_pAVCodecContext->height, this->get_fourcc_out(), DECODED_BUF_ACTUAL_COUNT, DECODED_BUF_ACTUAL_COUNT);
        return MMP_SUCCESS;
    }


#if (MMPDECODERVIDEO_H264_AVC_CHECK == 1)
    /* src: queue buf */
    if(this->get_fourcc_in() == MMP_FOURCC_VIDEO_H264) {
        unsigned char* p_avc1 = (unsigned char*)p_buf_vs->get_stream_real_ptr();
        if( (p_avc1[0]==0) &&
            (p_avc1[1]==0) &&
            (p_avc1[2]==0) &&
            (p_avc1[3]==1) ) {

            vs_size_sub = vs_size;
            memcpy(m_stream_buf, p_avc1, vs_size_sub );
        }
        else {
            mmpResult  = CMmpH264Parser::ConvertAvc1ToH264(p_avc1, vs_size,  m_stream_buf, (MMP_S32*)&vs_size_sub, MMP_FALSE);
            if(mmpResult != MMP_SUCCESS) {
                p_buf_vs->inc_stream_offset(vs_size);
                return mmpResult;
            }
        }
    }
    else {
        vs_size_sub = vs_size;
        memcpy(m_stream_buf, p_buf_vs->get_stream_real_ptr(), vs_size_sub );
    }

    p_stream_buf = m_stream_buf;
#else

    p_stream_buf = (MMP_U8*)p_buf_vs->get_stream_real_ptr();
    vs_size_sub = vs_size;
#endif


    av_init_packet (&avpkt);
    avpkt.data = (uint8_t*)p_stream_buf;
    avpkt.size = vs_size_sub;
    avpkt.pts = p_buf_vs->get_pts();

#if (MMPDECODERVIDEO_DUMMY_MODE_ENABLE == 1)
    if(m_decoded_picture_count < 300) {
        usebyte = avcodec_decode_video2(m_pAVCodecContext, m_pAVFrame_Decoded, &frameFinished, &avpkt);
    }
    else {
        m_enable_avcopy = MMP_FALSE;
        usebyte = vs_size_sub;
        frameFinished = 1;
    }
#else
    usebyte = avcodec_decode_video2(m_pAVCodecContext, m_pAVFrame_Decoded, &frameFinished, &avpkt);
#endif
    if(usebyte > 0) {
        p_buf_vs->inc_stream_offset(usebyte);

        if(m_minheap_ts.IsFull()) {
            m_minheap_ts.Delete(pts);
        }
        m_minheap_ts.Add(avpkt.pts);

    }
    else {
        p_buf_vs->inc_stream_offset(vs_size_sub);
    }

    MMPDEBUGMSG(0, (TEXT("[CMmpDecoderVideo_Ffmpeg::DecodeAu] %d au(%02x %02x %02x %02x %02x %02x %02x %02x ) usedbyte=%d ausz=(%d/%d) pts=%d codec_width=%d codec_height=%d framefinished=%d "),
                       m_decoded_picture_count,
                       avpkt.data[0], avpkt.data[1], avpkt.data[2], avpkt.data[3],
                       avpkt.data[4], avpkt.data[5], avpkt.data[6], avpkt.data[7],
                       usebyte,
                       avpkt.size, vs_size,
                       (unsigned int)(avpkt.pts/1000LL),
                       m_pAVCodecContext->width, m_pAVCodecContext->height,
                       frameFinished
                       ));

    if(frameFinished != 0) {

        int vpu_width, vpu_height;
        int decoder_buf_width, decoder_buf_height;

        m_decoded_picture_count++;

        decoder_buf_width = this->get_decoder_buf_width();
        decoder_buf_height = this->get_decoder_buf_height();

        vpu_width = mmp_buffer_videoframe::s_get_stride(MMP_FOURCC_IMAGE_YUV420, m_pAVCodecContext->width, MMP_BUFTYPE_HW);
        vpu_height = mmp_buffer_videoframe::s_get_height(MMP_FOURCC_IMAGE_YUV420, m_pAVCodecContext->height, MMP_BUFTYPE_HW);

        if(  (decoder_buf_width != vpu_width)
            ||  (decoder_buf_height != vpu_height)
            ) {

            *next_action = ACTION_RECONFIG;
            this->init_buf_vf_from_decoder(m_pAVCodecContext->width, m_pAVCodecContext->height, this->get_fourcc_out(), this->get_buf_req_count_min(), this->get_buf_req_count_actual() );

        }

        p_buf_vf_decoded = this->get_next_vf();
        if(p_buf_vf_decoded != NULL)     {
            AVPicture pic;
            AVPicture *pFrameOut = &pic;
            MMP_S32 pic_width, pic_height;
            enum MMP_FOURCC fourcc_decoded_buf;

            memset(pFrameOut, 0x00, sizeof(AVPicture));

            pic_width = p_buf_vf_decoded->get_pic_width();
            pic_height = p_buf_vf_decoded->get_pic_height();
            fourcc_decoded_buf = p_buf_vf_decoded->get_fourcc();
            switch(fourcc_decoded_buf) {

                case MMP_FOURCC_IMAGE_YUV420:
                case MMP_FOURCC_IMAGE_YVU420:
                    pFrameOut->data[0] = (MMP_U8*)p_buf_vf_decoded->get_buf_vir_addr_y();
                    pFrameOut->data[1] = (MMP_U8*)p_buf_vf_decoded->get_buf_vir_addr_u();
                    pFrameOut->data[2] = (MMP_U8*)p_buf_vf_decoded->get_buf_vir_addr_v();

                    pFrameOut->linesize[0] = p_buf_vf_decoded->get_buf_stride(0);
                    pFrameOut->linesize[1] = p_buf_vf_decoded->get_buf_stride(1);
                    pFrameOut->linesize[2] = p_buf_vf_decoded->get_buf_stride(2);
                    break;

                case MMP_FOURCC_IMAGE_YUV420M:
                    for(i = 0; i < MMP_IMAGE_MAX_PLANE_COUNT; i++) {
                        pFrameOut->data[i] = (MMP_U8*)p_buf_vf_decoded->get_buf_vir_addr(i);
                        pFrameOut->linesize[i] = p_buf_vf_decoded->get_buf_stride(i);
                    }
                    break;

                case MMP_FOURCC_IMAGE_YUV420M_META:
                    {
                        struct mmp_image_meta* p_meta = (struct mmp_image_meta*)p_buf_vf_decoded->get_buf_vir_addr();
                        MMP_IMAGE_META_INIT(p_meta, MMP_FOURCC_IMAGE_YUV420M, 3);
                        p_meta->pic_width = p_buf_vf_decoded->get_pic_width();
                        p_meta->pic_height = p_buf_vf_decoded->get_pic_height();
                        for(i = 0; i < 3; i++) {
                            p_meta->buf_width[i] = m_pAVFrame_Decoded->linesize[i];
                            if(i == 0) p_meta->buf_height[i] = p_meta->pic_height;
                            else  p_meta->buf_height[i] = p_meta->pic_height/2;
                            p_meta->buf_viraddr[i] = (MMP_ADDR)m_pAVFrame_Decoded->data[i];
                        }

                        m_enable_avcopy = MMP_FALSE;
                    }
                    break;

                default:
                    p_buf_vf_decoded = NULL;
                    break;
            }


            if( (p_buf_vf_decoded != NULL) && (m_enable_avcopy == MMP_TRUE) ) {
                switch(m_pAVFrame_Decoded->format) {

                    case AV_PIX_FMT_YUV420P:
                    case AV_PIX_FMT_YUVJ420P:
                        av_picture_copy ((AVPicture *)pFrameOut, (AVPicture*)m_pAVFrame_Decoded, AV_PIX_FMT_YUV420P, m_pAVCodecContext->width, m_pAVCodecContext->height);
                        break;

                    case AV_PIX_FMT_YUVJ422P:
                        av_picture_copy ((AVPicture *)pFrameOut, (AVPicture*)m_pAVFrame_Decoded, AV_PIX_FMT_YUV420P, m_pAVCodecContext->width, m_pAVCodecContext->height);
                        break;

                    case AV_PIX_FMT_PAL8:

                        {
                            int stride = m_pAVFrame_Decoded->linesize[0];
                            unsigned char* rgb = (unsigned char*)m_pAVFrame_Decoded->data[0];
                            unsigned int* pallete = (unsigned int*)m_pAVFrame_Decoded->data[1];

                            CMmpUtil::ColorConvertRGB8Pallete32ToYUV420Planar(m_pAVCodecContext->width, m_pAVCodecContext->height, stride,
                                                                 rgb, pallete,
                                                                 pFrameOut->data[0], pFrameOut->data[1], pFrameOut->data[2],
                                                                 pFrameOut->linesize[0], pFrameOut->linesize[1]
                                                                );

                        }
                        break;

                    case AV_PIX_FMT_RGB24:

                        {
                            int stride = m_pAVFrame_Decoded->linesize[0];
                            unsigned char* rgb24 = (unsigned char*)m_pAVFrame_Decoded->data[0];

                            CMmpUtil::ColorConvertRGB24ToYUV420Planar(m_pAVCodecContext->width, m_pAVCodecContext->height, stride,
                                                             rgb24,
                                                             pFrameOut->data[0], pFrameOut->data[1], pFrameOut->data[2],
                                                             pFrameOut->linesize[0], pFrameOut->linesize[1]
                                                            );

                        }
                        break;
                } /* end of switch(m_pAVFrame_Decoded->format) { */
            } /* end of if( (p_buf_vf_decoded != NULL) && (m_enable_avcopy == MMP_TRUE) ) {*/

        }/* end of  if(p_buf_vf_decoded != NULL) { */
        else {
            MMPDEBUGMSG(1, (TEXT("[CMmpDecoderVideo_Ffmpeg::DecodeAu] FAIL to this->get_next_vf(); ")));
        }
    }

    dec_end_tick = CMmpUtil::GetTickCount();

    if(p_buf_vf_decoded != NULL) {

        if(m_minheap_ts.IsEmpty()) {
            p_buf_vf_decoded = NULL;
        }
        else {
            m_minheap_ts.Delete(pts);
            p_buf_vf_decoded->set_pts(pts);

            p_buf_vf_decoded->set_coding_dur(dec_end_tick - dec_start_tick);
            p_buf_vf_decoded->clear_own();
        }
    }

    if(pp_buf_vf != NULL) {
       *pp_buf_vf = p_buf_vf_decoded;
    }

	return mmpResult;
}

MMP_RESULT CMmpDecoderVideo_Ffmpeg::DecodeAu(class mmp_buffer_videostream* p_buf_vs, class mmp_buffer_videoframe* p_buf_vf) {

    MMP_RESULT mmpResult = MMP_SUCCESS;
    MMP_S32 i;
    int32_t frameFinished = 192000 * 2;
    int32_t usebyte;
    AVPacket avpkt;

    MMP_U32 dec_start_tick, dec_end_tick;

    MMP_S32 vs_size = p_buf_vs->get_stream_real_size();
    MMP_S32 vs_size_sub;
    MMP_U8* p_stream_buf;
    MMP_S64 pts;

    dec_start_tick = CMmpUtil::GetTickCount();
    if(p_buf_vf !=NULL) p_buf_vf->set_coding_result(mmp_buffer_media::FAIL);

    /* Decode DSI */
    if(m_pAVCodec == NULL) {
        mmpResult = this->DecodeDSI(p_buf_vs);
        if(mmpResult != MMP_SUCCESS) {
            return mmpResult;
        }
    }

    /* Decode DSI */
    if((p_buf_vs->get_flag()&mmp_buffer_media::FLAG_CONFIGDATA) != 0) {
        if(p_buf_vf !=NULL) p_buf_vf->set_coding_result(mmp_buffer_media::RECONFIG);
        this->init_buf_vf_from_decoder(m_pAVCodecContext->width, m_pAVCodecContext->height, this->get_fourcc_out(), this->get_buf_req_count_min(), this->get_buf_req_count_actual() );
        return MMP_SUCCESS;
    }

    p_stream_buf = (MMP_U8*)p_buf_vs->get_stream_real_ptr();
    vs_size_sub = vs_size;

    av_init_packet (&avpkt);
    avpkt.data = (uint8_t*)p_stream_buf;
    avpkt.size = vs_size_sub;
    avpkt.pts = p_buf_vs->get_pts();

    usebyte = avcodec_decode_video2(m_pAVCodecContext, m_pAVFrame_Decoded, &frameFinished, &avpkt);
    if(usebyte > 0) {
        p_buf_vs->inc_stream_offset(usebyte);

        if(m_minheap_ts.IsFull()) {
            m_minheap_ts.Delete(pts);
        }
        m_minheap_ts.Add(avpkt.pts);
    }
    else {
        p_buf_vs->inc_stream_offset(vs_size_sub);
    }

    MMPDEBUGMSG(0, (TEXT("[CMmpDecoderVideo_Ffmpeg::DecodeAu] %d au(%02x %02x %02x %02x %02x %02x %02x %02x ) usedbyte=%d ausz=(%d/%d) pts=%d codec_width=%d codec_height=%d framefinished=%d "),
                       m_decoded_picture_count,
                       avpkt.data[0], avpkt.data[1], avpkt.data[2], avpkt.data[3],
                       avpkt.data[4], avpkt.data[5], avpkt.data[6], avpkt.data[7],
                       usebyte,
                       avpkt.size, vs_size,
                       (unsigned int)(avpkt.pts/1000LL),
                       m_pAVCodecContext->width, m_pAVCodecContext->height,
                       frameFinished
                       ));

    if(frameFinished != 0) {

        m_decoded_picture_count++;

        if(  (this->get_decoder_pic_width() != m_pAVCodecContext->width)
            ||  (this->get_decoder_pic_height() != m_pAVCodecContext->height)
            ) {

            if(p_buf_vf !=NULL) p_buf_vf->set_coding_result(mmp_buffer_media::RECONFIG);
            this->init_buf_vf_from_decoder(m_pAVCodecContext->width, m_pAVCodecContext->height, this->get_fourcc_out(), this->get_buf_req_count_min(), this->get_buf_req_count_actual() );

        }

        if(p_buf_vf !=NULL) {

            AVPicture pic;
            AVPicture *pFrameOut = &pic;
            MMP_S32 pic_width, pic_height;
            enum MMP_FOURCC fourcc_decoded_buf;

            memset(pFrameOut, 0x00, sizeof(AVPicture));

            pic_width = p_buf_vf->get_pic_width();
            pic_height = p_buf_vf->get_pic_height();
            fourcc_decoded_buf = p_buf_vf->get_fourcc();
            switch(fourcc_decoded_buf) {

                case MMP_FOURCC_IMAGE_YUV420:
                case MMP_FOURCC_IMAGE_YVU420:
                    pFrameOut->data[0] = (MMP_U8*)p_buf_vf->get_buf_vir_addr_y();

                    if(this->is_android_buffer() == MMP_TRUE) {
                        //fourcc_decoded_buf = MMP_FOURCC_IMAGE_YVU420;
                        pFrameOut->data[2] = (MMP_U8*)p_buf_vf->get_buf_vir_addr_u();
                        pFrameOut->data[1] = (MMP_U8*)p_buf_vf->get_buf_vir_addr_v();
                    }
                    else {
                        pFrameOut->data[1] = (MMP_U8*)p_buf_vf->get_buf_vir_addr_u();
                        pFrameOut->data[2] = (MMP_U8*)p_buf_vf->get_buf_vir_addr_v();
                    }

                    pFrameOut->linesize[0] = p_buf_vf->get_buf_stride(0);
                    pFrameOut->linesize[1] = p_buf_vf->get_buf_stride(1);
                    pFrameOut->linesize[2] = p_buf_vf->get_buf_stride(2);
                    av_picture_copy ((AVPicture *)pFrameOut, (AVPicture*)m_pAVFrame_Decoded, AV_PIX_FMT_YUV420P, m_pAVCodecContext->width, m_pAVCodecContext->height);
                    break;

                case MMP_FOURCC_IMAGE_YUV420M:
                    for(i = 0; i < MMP_IMAGE_MAX_PLANE_COUNT; i++) {
                        pFrameOut->data[i] = (MMP_U8*)p_buf_vf->get_buf_vir_addr(i);
                        pFrameOut->linesize[i] = p_buf_vf->get_buf_stride(i);
                    }
                    av_picture_copy ((AVPicture *)pFrameOut, (AVPicture*)m_pAVFrame_Decoded, AV_PIX_FMT_YUV420P, m_pAVCodecContext->width, m_pAVCodecContext->height);
                    break;

                case MMP_FOURCC_IMAGE_ARGB8888:
                case MMP_FOURCC_IMAGE_ABGR8888:

                    CMmpImageTool::ConvertYUV420MtoRGB(m_pAVFrame_Decoded->data[0], m_pAVFrame_Decoded->data[1], m_pAVFrame_Decoded->data[2],
                                                       m_pAVFrame_Decoded->linesize[0], m_pAVFrame_Decoded->linesize[1], m_pAVFrame_Decoded->linesize[2],
                                                       pic_width, pic_height,
                                                       (MMP_U8*)p_buf_vf->get_buf_vir_addr(), fourcc_decoded_buf);

                    break;

            }
        }
    }

    dec_end_tick = CMmpUtil::GetTickCount();

    if(p_buf_vf !=NULL) {

        if(m_minheap_ts.IsEmpty()) {
            p_buf_vf->set_coding_result(mmp_buffer_media::FAIL);
        }
        else {
            m_minheap_ts.Delete(pts);
            p_buf_vf->set_pts(pts);

            p_buf_vf->set_coding_dur(dec_end_tick - dec_start_tick);
            p_buf_vf->set_coding_result(mmp_buffer_media::SUCCESS);
            p_buf_vf->clear_own();
        }
    }


	return mmpResult;
}

#endif
