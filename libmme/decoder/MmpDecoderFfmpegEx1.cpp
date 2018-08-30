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

#include "MmpDecoderFfmpegEx1.hpp"
#include "MmpUtil.hpp"
#include "mmp_video_tool.hpp"
#include "mmp_audio_tool.hpp"
#include "MmpWMVTool.hpp"
#include "MmpH264Tool.hpp"

#if (FFMPEG_VERSION == FFMPEG_VERSION_n3_1_1)


#if (MMP_OS == MMP_OS_WIN32) /* ffmpeg win32 mingw parameter..*/
//extern "C" int __chkstk_ms=0;
//extern "C" int __divdi3 = 0;
#endif

/////////////////////////////////////////////////////////////
//CMmpDecoderFfmpegEx1 Member Functions

#define AV_CODEC_ID_FFMPEG    MKBETAG('F','F','M','P')

static AVCodecID get_ffmpeg_codecid_from_fourcc(enum MMP_FOURCC fourcc_in) {

    AVCodecID cid;

    switch(fourcc_in) {
    
        /* Audio */
        case MMP_FOURCC_AUDIO_MP3: cid = AV_CODEC_ID_MP3; break;
        case MMP_FOURCC_AUDIO_MP2: cid = AV_CODEC_ID_MP2; break;
        case MMP_FOURCC_AUDIO_MP1: cid = AV_CODEC_ID_MP1; break;
        case MMP_FOURCC_AUDIO_VORBIS: cid = AV_CODEC_ID_VORBIS; break;
        case MMP_FOURCC_AUDIO_WMA1: cid = AV_CODEC_ID_WMAV1; break;
        case MMP_FOURCC_AUDIO_WMA2: cid = AV_CODEC_ID_WMAV2; break;
        case MMP_FOURCC_AUDIO_WMA_PRO: cid = AV_CODEC_ID_WMAPRO; break;
        case MMP_FOURCC_AUDIO_WMA_LOSSLESS: cid = AV_CODEC_ID_WMALOSSLESS; break;
        case MMP_FOURCC_AUDIO_WMA_VOICE: cid = AV_CODEC_ID_WMAVOICE; break;
        case MMP_FOURCC_AUDIO_AC3: cid = AV_CODEC_ID_AC3; break;
        case MMP_FOURCC_AUDIO_DTS: cid = AV_CODEC_ID_DTS; break;
        case MMP_FOURCC_AUDIO_AAC: cid = AV_CODEC_ID_AAC; break;
        case MMP_FOURCC_AUDIO_FLAC: cid=AV_CODEC_ID_FLAC; break;
        case MMP_FOURCC_AUDIO_AMR_NB: cid=AV_CODEC_ID_AMR_NB; break;
        case MMP_FOURCC_AUDIO_AMR_WB: cid=AV_CODEC_ID_AMR_WB; break;
        case MMP_FOURCC_AUDIO_FFMPEG: cid = (AVCodecID)AV_CODEC_ID_FFMPEG; break;
        case MMP_FOURCC_AUDIO_CODEC_PCM_S16LE: cid = (AVCodecID)AV_CODEC_ID_PCM_S16LE; break;

        /* Video */
        case MMP_FOURCC_VIDEO_H263: cid=AV_CODEC_ID_H263; break;
        case MMP_FOURCC_VIDEO_H264: cid=AV_CODEC_ID_H264; break;
        case MMP_FOURCC_VIDEO_HEVC: cid=AV_CODEC_ID_HEVC; break;
        case MMP_FOURCC_VIDEO_MPEG4: cid=AV_CODEC_ID_MPEG4; break;
        case MMP_FOURCC_VIDEO_MPEG2: cid=AV_CODEC_ID_MPEG2VIDEO; break;
            
        /* VPU support Only WMV3 */
        case MMP_FOURCC_VIDEO_WMV1: cid=AV_CODEC_ID_WMV1; break;
        case MMP_FOURCC_VIDEO_WMV2: cid=AV_CODEC_ID_WMV2; break;
        case MMP_FOURCC_VIDEO_WMV3: cid=AV_CODEC_ID_WMV3; break;
        case MMP_FOURCC_VIDEO_WVC1: cid=AV_CODEC_ID_VC1; break; /* VC1 Advacned@L3 */

        case MMP_FOURCC_VIDEO_MSS1: cid=AV_CODEC_ID_MSS1; break;
        case MMP_FOURCC_VIDEO_MSS2: cid=AV_CODEC_ID_MSS2; break;
        
        case MMP_FOURCC_VIDEO_VP80: cid=AV_CODEC_ID_VP8; break;
        case MMP_FOURCC_VIDEO_VP60: cid=AV_CODEC_ID_VP6; break;
        case MMP_FOURCC_VIDEO_VP6F: cid=AV_CODEC_ID_VP6F; break;
        case MMP_FOURCC_VIDEO_VP6A: cid=AV_CODEC_ID_VP6A; break;

        case MMP_FOURCC_VIDEO_RV30: cid=AV_CODEC_ID_RV30; break;
        case MMP_FOURCC_VIDEO_RV40: cid=AV_CODEC_ID_RV40; break;
        case MMP_FOURCC_VIDEO_RV:  cid=AV_CODEC_ID_RV40;  break;
        
        case MMP_FOURCC_VIDEO_SVQ1: cid=AV_CODEC_ID_SVQ1; break;
        case MMP_FOURCC_VIDEO_SVQ3: cid=AV_CODEC_ID_SVQ3; break;
        
        case MMP_FOURCC_VIDEO_THEORA: cid=AV_CODEC_ID_THEORA; break;
        case MMP_FOURCC_VIDEO_MJPEG: cid=AV_CODEC_ID_MJPEG; break;
        case MMP_FOURCC_VIDEO_FLV1: cid=AV_CODEC_ID_FLV1; break;
        //case MMP_FOURCC_VIDEO_MSMPEG4V1: cid=AV_CODEC_ID_MSMPEG4V1; break;
        //case MMP_FOURCC_VIDEO_MSMPEG4V2: cid=AV_CODEC_ID_MSMPEG4V2; break;
        case MMP_FOURCC_VIDEO_MSMPEG4V3: cid=AV_CODEC_ID_MSMPEG4V3; break;
            
        
        case MMP_FOURCC_VIDEO_FFMPEG: cid = (AVCodecID)AV_CODEC_ID_FFMPEG; break;
        
        default:  
            cid = AV_CODEC_ID_NONE;
    }

    return cid;
}

CMmpDecoderFfmpegEx1::CMmpDecoderFfmpegEx1(enum MMP_FOURCC fourcc_in) :
m_fourcc_in(fourcc_in)
,m_pAVCodec(NULL)
,m_pAVCodecContext(NULL)
,m_pAVFrame_Decoded(NULL)
,m_extra_data(NULL)

/* h264, avc */
,m_is_avc(0)
,m_nal_length_size(0)
    
{
    avcodec_register_all();
    m_AVCodecID = get_ffmpeg_codecid_from_fourcc(fourcc_in);
    
}

CMmpDecoderFfmpegEx1::~CMmpDecoderFfmpegEx1()
{

}

MMP_RESULT CMmpDecoderFfmpegEx1::Open()
{
    MMP_RESULT mmpResult = MMP_SUCCESS;

    if(m_AVCodecID == AV_CODEC_ID_NONE) {
        mmpResult = MMP_FAILURE;
    }
    
    return mmpResult;
}


MMP_RESULT CMmpDecoderFfmpegEx1::Close()
{
    if(this->m_pAVCodecContext != NULL) {
        avcodec_close(this->m_pAVCodecContext);
        av_free(this->m_pAVCodecContext);
        this->m_pAVCodecContext = NULL;
    }
    
    if(m_pAVFrame_Decoded != NULL) {
        av_frame_free(&m_pAVFrame_Decoded);
        m_pAVFrame_Decoded = NULL;
    }

    if(m_extra_data != NULL) {
        delete [] m_extra_data;
        m_extra_data = NULL;
    }
    
    return MMP_SUCCESS;
}

MMP_RESULT CMmpDecoderFfmpegEx1::DecodeDSI(MMP_U8* pStream, MMP_U32 nStreamSize) {

    return MMP_FAILURE;
}

MMP_RESULT CMmpDecoderFfmpegEx1::DecodeDSI(class mmp_buffer_audiostream* p_buf_as) {
    
    AVCodec *codec;
    AVCodecContext *cc= NULL;
    MMP_BOOL is_audio_packet_hdr = MMP_FALSE;
    struct mmp_audio_packet_header *p_audio_pkt_hdr;
    MMP_ADDR pkt_addr, dsi_data_addr;
       

    is_audio_packet_hdr = mmp_audio_tool::is_valid_audio_packet_header(p_buf_as->get_buf_vir_addr(), p_buf_as->get_stream_size());
    if(is_audio_packet_hdr == MMP_TRUE) {
        
        pkt_addr = (MMP_ADDR)p_buf_as->get_buf_vir_addr();
        p_audio_pkt_hdr = (struct mmp_audio_packet_header *)pkt_addr;
        dsi_data_addr = pkt_addr + p_audio_pkt_hdr->dsi_pos;
     
        if(m_AVCodecID == (AVCodecID)AV_CODEC_ID_FFMPEG) {
            m_fourcc_in = p_audio_pkt_hdr->fourcc;
            m_AVCodecID = get_ffmpeg_codecid_from_fourcc(m_fourcc_in);
        }
    }
    
    codec = avcodec_find_decoder(m_AVCodecID);
    if(codec == NULL) {
        MMPDEBUGMSG(MMPZONE_ERROR, (TEXT(MMP_CTAG" FAIL: avcodec_find_decoder   m_AVCodecID=(%d 0x%08x) fourcc=%c%c%c%c "), MMP_CNAME, m_AVCodecID, m_AVCodecID, MMPGETFOURCCARG(m_fourcc_in) ));
        return MMP_FAILURE;
    }
    
    cc= avcodec_alloc_context3(codec);
    if(m_extra_data != NULL) {
        delete [] m_extra_data;
        m_extra_data = NULL;
    }
    
    if(is_audio_packet_hdr == MMP_TRUE) {
                
        cc->channels = p_audio_pkt_hdr->channels;
        cc->sample_rate = p_audio_pkt_hdr->sample_rate;
        cc->frame_size = p_audio_pkt_hdr->frame_size;
        cc->block_align = p_audio_pkt_hdr->block_align;
        cc->bit_rate = p_audio_pkt_hdr->bitrate;

        if(p_audio_pkt_hdr->dsi_size > 0) {
            m_extra_data = new MMP_U8[p_audio_pkt_hdr->dsi_size];
            memcpy(m_extra_data, (void*)dsi_data_addr, p_audio_pkt_hdr->dsi_size);
        }
        cc->extradata = m_extra_data;
        cc->extradata_size = p_audio_pkt_hdr->dsi_size;
    }
    else {
        switch(m_fourcc_in) {

            case MMP_FOURCC_AUDIO_WMA2:
                {
                    android_adsp_codec_wma_t *p_wma = (android_adsp_codec_wma_t*)p_buf_as->get_buf_vir_addr();

                    if(p_wma->extra_data_size <= MMP_AUDIO_FORMAT_MAX_EXTRA_DATA_SIZE) {
                        m_extra_data = new MMP_U8[p_wma->extra_data_size];
                        memcpy(m_extra_data, (void*)p_wma->extra_data, p_wma->extra_data_size);
                    }
                    cc->extradata = m_extra_data;
                    cc->extradata_size = p_wma->extra_data_size;
                    
                    cc->channels = p_wma->i_num_chan;
                    cc->sample_rate = p_wma->i_samp_freq;
                    cc->frame_size = 0;
                    cc->block_align = p_wma->i_blk_align;
                    cc->bit_rate = p_wma->i_avg_byte*8;

                }
                break;
            
            case MMP_FOURCC_AUDIO_FLAC:
                {
                    android_adsp_codec_flac_t *p_flac = (android_adsp_codec_flac_t*)p_buf_as->get_buf_vir_addr();

                    if(p_flac->extra_data_size <= FFMPEG_FLAC_STREAMINFO_SIZE) {
                        m_extra_data = new MMP_U8[p_flac->extra_data_size];
                        memcpy(m_extra_data, (void*)p_flac->extra_data, p_flac->extra_data_size);
                    }
                    cc->extradata = m_extra_data;
                    cc->extradata_size = p_flac->extra_data_size;
                    
                    cc->channels = 0;
                    cc->sample_rate = 0;
                    cc->frame_size = 0;
                    cc->block_align = 0;
                    cc->bit_rate = 0;
                    
                    MMPDEBUGMSG(MMPZONE_ERROR, (TEXT(MMP_CTAG" FLAC ext_sz=%d (%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x "), MMP_CNAME, 
                                              cc->extradata_size, 
                                              m_extra_data[0], m_extra_data[1], m_extra_data[2], m_extra_data[3], 
                                              m_extra_data[4], m_extra_data[5], m_extra_data[6], m_extra_data[7], 
                                              m_extra_data[8], m_extra_data[9], m_extra_data[10], m_extra_data[11], 
                                              m_extra_data[12], m_extra_data[13], m_extra_data[14], m_extra_data[15] 
                                              ));
                }
                break;
            
            case MMP_FOURCC_AUDIO_MP3:
            case MMP_FOURCC_AUDIO_WMA_PRO:
            default:
                {
                    struct mmp_audio_format* p_af = (struct mmp_audio_format*)p_buf_as->get_buf_vir_addr();
                    
                    if(p_af->key == MMP_AUDIO_FORMAT_KEY) {

                        if( (p_af->extra_data_size <= MMP_AUDIO_FORMAT_MAX_EXTRA_DATA_SIZE)
                            && (p_af->extra_data_size > 0)
                            ) {
                            m_extra_data = new MMP_U8[p_af->extra_data_size];
                            memcpy(m_extra_data, (void*)p_af->extra_data, p_af->extra_data_size);
                        }
                        else if(p_af->extra_data_size > 0) {
                            MMP_U8* p_tmp;
                            m_extra_data = new MMP_U8[p_af->extra_data_size];
                            p_tmp = (MMP_U8*)p_buf_as->get_buf_vir_addr();
                            memcpy(m_extra_data, &p_tmp[sizeof(struct mmp_audio_format)], p_af->extra_data_size);
                        }

                        cc->extradata = m_extra_data;
                        cc->extradata_size = p_af->extra_data_size;
                    
                        cc->channels = p_af->channels;
                        cc->sample_rate = p_af->sample_rate;
                        cc->frame_size = p_af->frame_size;
                        cc->block_align = p_af->block_align;
                        cc->bit_rate = p_af->bitrate;
                    }
                }
                break;
        }
    }
       
    /* open it */
    if(avcodec_open2(cc, codec, NULL) < 0) 
    {
        MMPDEBUGMSG(MMPZONE_ERROR, (TEXT(MMP_CTAG"FAIL: could not open codec %c%c%c%c "), MMP_CNAME, MMPGETFOURCCARG(m_fourcc_in) ));
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
    return CMmpDecoderFfmpegEx1::ffmpeg_get_buffer_cb_stub(p_context, p_ff_pic);
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
    CMmpDecoderFfmpegEx1::ffmpeg_release_buffer_cb_stub(p_context, p_ff_pic);
#endif
    //for( int i = 0; i < 4; i++ )
      //  p_ff_pic->data[i] = NULL;
}


int CMmpDecoderFfmpegEx1::ffmpeg_get_buffer_cb_stub( struct AVCodecContext *p_context,  AVFrame *p_ff_pic ) {
    CMmpDecoderFfmpegEx1* pFfmpeg = (CMmpDecoderFfmpegEx1*)p_context->opaque;
    return pFfmpeg->ffmpeg_get_buffer_cb(p_context, p_ff_pic);
}

void CMmpDecoderFfmpegEx1::ffmpeg_release_buffer_cb_stub( struct AVCodecContext *p_context,  AVFrame *p_ff_pic ) {
    CMmpDecoderFfmpegEx1* pFfmpeg = (CMmpDecoderFfmpegEx1*)p_context->opaque;
    pFfmpeg->ffmpeg_release_buffer_cb(p_context, p_ff_pic);
}

int CMmpDecoderFfmpegEx1::ffmpeg_get_buffer_cb(struct AVCodecContext *p_context,  AVFrame *p_ff_pic) {
    return 0;//avcodec_default_get_buffer(p_context, p_ff_pic);
}

void CMmpDecoderFfmpegEx1::ffmpeg_release_buffer_cb(struct AVCodecContext *p_context,  AVFrame *p_ff_pic) {
    //avcodec_default_release_buffer( p_context, p_ff_pic );
}

MMP_RESULT CMmpDecoderFfmpegEx1::DecodeDSI(class mmp_buffer_videostream* p_buf_vs, MMP_S32 pic_width, MMP_S32 pic_height ) {
        
    MMP_RESULT mmpResult = MMP_SUCCESS;
    AVCodec *codec;
    AVCodecContext *cc= NULL;
    AVCodecContext *cc1 = NULL;
    MMP_U8* extra_data;
    MMP_S32 extra_data_size;

    MMP_BOOL is_video_packet_hdr = MMP_FALSE;
    struct mmp_video_packet_header *p_video_pkt_hdr;
    MMP_ADDR pkt_addr, dsi_data_addr;
    
    is_video_packet_hdr = mmp_video_tool::is_valid_video_packet_header(p_buf_vs->get_buf_vir_addr(), p_buf_vs->get_stream_size());
    if(is_video_packet_hdr == MMP_TRUE) {
        pkt_addr = (MMP_ADDR)p_buf_vs->get_buf_vir_addr();
        p_video_pkt_hdr = (struct mmp_video_packet_header *)pkt_addr;
        dsi_data_addr = pkt_addr + p_video_pkt_hdr->dsi_pos;
                
        if(m_AVCodecID == (AVCodecID)AV_CODEC_ID_FFMPEG) {
            m_fourcc_in = p_video_pkt_hdr->fourcc;
            m_AVCodecID = get_ffmpeg_codecid_from_fourcc(m_fourcc_in);
        }
    }

    switch(m_fourcc_in) {
        case MMP_FOURCC_VIDEO_WMV3:
        case MMP_FOURCC_VIDEO_WVC1:

            extra_data = (MMP_U8*)p_buf_vs->get_stream_real_ptr();
            extra_data_size = p_buf_vs->get_stream_real_size();

            m_fourcc_in = CMmpWMVParser::get_fourcc_with_extradata(extra_data, extra_data_size);
            switch(m_fourcc_in) {
                case MMP_FOURCC_VIDEO_WMV3:  m_AVCodecID = AV_CODEC_ID_WMV3; break;
                case MMP_FOURCC_VIDEO_WVC1:  m_AVCodecID = AV_CODEC_ID_VC1; break;
                default :
                    m_AVCodecID = AV_CODEC_ID_NONE;
                    break;
            }
            
            break;
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
        
        cc->width = p_video_pkt_hdr->pic_width;
        cc->height = p_video_pkt_hdr->pic_height;

        if(p_video_pkt_hdr->dsi_size > 0) {
            m_extra_data = new MMP_U8[p_video_pkt_hdr->dsi_size + 4096 /* reserve data for example avc */];
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
        cc->width = pic_width;
        cc->height = pic_height;
        if(p_buf_vs->get_stream_size() > 0) {

            extra_data = (MMP_U8*)p_buf_vs->get_stream_real_ptr();
            extra_data_size = p_buf_vs->get_stream_real_size();

            m_extra_data = new MMP_U8[extra_data_size + 4096 /* reserve data for example avc*/];
            memcpy(m_extra_data, (void*)extra_data, extra_data_size);
            cc->extradata = m_extra_data;
            cc->extradata_size = extra_data_size;
        }
        else {
            cc->extradata = NULL;
            cc->extradata_size = 0;
        }

    }
    
    if( (m_fourcc_in == MMP_FOURCC_VIDEO_H264) 
      && (cc->extradata_size > 0) ) {
          
          unsigned char *tempbuf = new unsigned char[cc->extradata_size + 4096 /* reserve data for example avc*/];
          MMP_S32 tempbufsz;
          MMP_S32 dsi_size;

          tempbufsz = cc->extradata_size;
          memcpy(tempbuf, cc->extradata, tempbufsz);

          memset(cc->extradata, 0x00, cc->extradata_size);
          cc->extradata_size = 0;

          CMmpH264Parser::ConvertDSI_AVC1_To_H264(tempbuf, tempbufsz, cc->extradata, &dsi_size, &m_is_avc, &m_nal_length_size);
          if(m_is_avc == 1) {
               cc->extradata_size = (int)dsi_size;
          }
          else {
               cc->extradata_size = tempbufsz;
               memcpy(cc->extradata, tempbuf, cc->extradata_size); 
          }
    
          delete [] tempbuf;
    }

    
    /* open it */
    if(avcodec_open2(cc, codec, NULL) < 0)  {

        MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpDecoderFfmpegEx1::DecodeDSI] FAIL: could not open codec\n\r")));
        mmpResult = MMP_FAILURE;
    }
    else {
        m_pAVCodec = codec;
        m_pAVCodecContext = cc;
        m_pAVFrame_Decoded = av_frame_alloc();
    }

    if(mmpResult != MMP_SUCCESS) {
        MMP_U8* pdata = (MMP_U8*)p_buf_vs->get_buf_vir_addr();
        MMP_S32 dsisz = p_buf_vs->get_stream_size();
        MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpDecoderFfmpegEx1::DecodeDSI] dsisz : %d  dsi(0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x "), 
                  dsisz,                  
                  pdata[0], pdata[1], pdata[2], pdata[3],
                  pdata[4], pdata[5], pdata[6], pdata[7]
                  ));
        
    }

    return mmpResult;
}

#endif