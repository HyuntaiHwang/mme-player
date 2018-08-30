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

#include "MmpEncoderVideo_Ffmpeg.hpp"
#include "MmpUtil.hpp"
#include "MmpMpeg4Tool.hpp"
#include "MmpH264Tool.hpp"
#include "MmpImageTool.hpp"

#if (FFMPEG_VERSION == FFMPEG_VERSION_2_6_1)

/////////////////////////////////////////////////////////////
//CMmpEncoderVideo_Ffmpeg Member Functions

CMmpEncoderVideo_Ffmpeg::CMmpEncoderVideo_Ffmpeg(struct CMmpEncoderVideo::create_config *p_create_config) : 

CMmpEncoderVideo(p_create_config, "Ffmpeg")

,m_nEncodedStreamCount(0)
,m_is_dsi(MMP_FALSE)

#if (CMmpEncoderVideo_Ffmpeg_YUV_DUMP == 1)
,m_yuv_dump_hdl(NULL)
#endif

,m_ColorConvertFrameBuffer(NULL)
{
    
}

CMmpEncoderVideo_Ffmpeg::~CMmpEncoderVideo_Ffmpeg()
{

}

MMP_RESULT CMmpEncoderVideo_Ffmpeg::Open()
{
    MMP_RESULT mmpResult = MMP_SUCCESS;
    
    if(mmpResult == MMP_SUCCESS) {
        mmpResult=CMmpEncoderVideo::Open();
    }

    if(mmpResult == MMP_SUCCESS) {
        mmpResult=CMmpEncoderFfmpeg::Open();
    }
    
    if(mmpResult == MMP_SUCCESS) {
        MMPDEBUGMSG(MMPZONE_MONITOR, (TEXT("[CMmpEncoderVideo_Ffmpeg::Open] Success nForamt=(0x%08x %s) \n\r"), 
            this->get_fourcc_out(), this->get_fourcc_out_name() ));
    }

    return mmpResult;
}


MMP_RESULT CMmpEncoderVideo_Ffmpeg::Close()
{
    MMP_RESULT mmpResult;
    
    mmpResult=CMmpEncoderFfmpeg::Close();
    if(mmpResult!=MMP_SUCCESS)
    {
        MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpEncoderVideo_Ffmpeg::Close] CMmpEncoderFfmpeg::Close() \n\r")));
        return mmpResult;
    }

    mmpResult=CMmpEncoderVideo::Close();
    if(mmpResult!=MMP_SUCCESS)
    {
        MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpEncoderVideo_Ffmpeg::Close] CMmpEncoderVideo::Close() \n\r")));
        return mmpResult;
    }
    
    MMPDEBUGMSG(MMPZONE_MONITOR, (TEXT("[CMmpEncoderVideo_Ffmpeg::Close] Success nForamt=(0x%08x %s) \n\r"), 
                  this->get_fourcc_out(), this->get_fourcc_out_name() ));

    
#if (CMmpEncoderVideo_Ffmpeg_YUV_DUMP == 1)
    yuv_dump_destroy(m_yuv_dump_hdl);
    m_yuv_dump_hdl = NULL;
#endif

    if(m_ColorConvertFrameBuffer != NULL) {
        delete [] m_ColorConvertFrameBuffer;
        m_ColorConvertFrameBuffer = NULL;
    }

    return MMP_SUCCESS;
}

MMP_RESULT CMmpEncoderVideo_Ffmpeg::EncodeAu(class mmp_buffer_videoframe* p_buf_vf, class mmp_buffer_videostream* p_buf_vs) {

    AVPacket avpkt;
    int got_packet_ptr;
    int iret;
    MMP_U32 enc_start_tick, enc_end_tick;
    MMP_RESULT mmpResult;
    MMP_S32 pic_width = this->get_pic_width();
    MMP_S32 pic_height = this->get_pic_height();
    enum MMP_FOURCC fourcc_in = this->get_fourcc_in();
    MMP_U8 *Y, *U, *V;
    MMP_S32 luma_stride, chroma_stride;
        
    enc_start_tick = CMmpUtil::GetTickCount();

    p_buf_vs->set_stream_size(0);
    p_buf_vs->set_stream_offset(0);
    p_buf_vs->set_flag(mmp_buffer_media::FLAG_NULL);
    p_buf_vs->set_pts(p_buf_vf->get_pts());
    p_buf_vs->set_dsi_size(0);
    p_buf_vs->set_coding_dur(0);

    if(m_pAVCodecContext == NULL) {
        CMmpEncoderFfmpeg::config enc_config;

        enc_config.fourcc_out = this->get_fourcc_out();
        enc_config.pic_width = pic_width;
        enc_config.pic_height = pic_height;
        enc_config.fr = this->get_fr();
        enc_config.br = this->get_br();
        enc_config.ip = this->get_ip();
        
        mmpResult = CMmpEncoderFfmpeg::EncodeDSI(&enc_config);
        if(mmpResult != MMP_SUCCESS) {
            return mmpResult;
        }
    }

    if(fourcc_in != p_buf_vf->get_fourcc() ) {
        return MMP_FAILURE;        
    }


#if (CMmpEncoderVideo_Ffmpeg_YUV_DUMP == 1)
    if(m_yuv_dump_hdl == NULL) {
        m_yuv_dump_hdl = yuv_dump_write_create("MmeEncDump", pic_width, pic_height, fourcc_in);
    }
#endif

    switch(fourcc_in) {
        
        case MMP_FOURCC_IMAGE_ARGB8888:
        case MMP_FOURCC_IMAGE_ABGR8888:
        case MMP_FOURCC_IMAGE_RGB888:
        case MMP_FOURCC_IMAGE_BGR888:
            if(m_ColorConvertFrameBuffer == NULL) {
                m_ColorConvertFrameBuffer = new MMP_U8[  V4L2_VIDEO_SW_YUV420_FRAME_SIZE(pic_width, pic_height) ];
            }
            if(m_ColorConvertFrameBuffer == NULL) {
                return MMP_FAILURE;        
            }

            Y = m_ColorConvertFrameBuffer + V4L2_VIDEO_SW_YUV420_Y_FRAME_OFFSET(pic_width, pic_height);
            U = m_ColorConvertFrameBuffer + V4L2_VIDEO_SW_YUV420_U_FRAME_OFFSET(pic_width, pic_height);
            V = m_ColorConvertFrameBuffer + V4L2_VIDEO_SW_YUV420_V_FRAME_OFFSET(pic_width, pic_height);

            luma_stride = V4L2_VIDEO_SW_YUV420_LUMA_STRIDE(pic_width);
            chroma_stride = V4L2_VIDEO_SW_YUV420_CHROMA_STRIDE(pic_width);

            CMmpImageTool::ConvertRGBtoYUV420M((MMP_U8*)p_buf_vf->get_buf_vir_addr(), 
                                       pic_width, pic_height, 
                                       fourcc_in,
                                       Y, U, V,
                                       luma_stride, chroma_stride, chroma_stride);

#if (CMmpEncoderVideo_Ffmpeg_YUV_DUMP == 1)
            yuv_dump_write_data(m_yuv_dump_hdl, (MMP_U8*)p_buf_vf->get_buf_vir_addr(),  p_buf_vf->get_buf_size() );
#endif

            break;

        default:

            Y = (MMP_U8*)p_buf_vf->get_buf_vir_addr_y();
            U = (MMP_U8*)p_buf_vf->get_buf_vir_addr_u();
            V = (MMP_U8*)p_buf_vf->get_buf_vir_addr_v();

            luma_stride = p_buf_vf->get_stride_luma();
            chroma_stride = p_buf_vf->get_stride_chroma();

#if (CMmpEncoderVideo_Ffmpeg_YUV_DUMP == 1)
            yuv_dump_write_YUV420M(m_yuv_dump_hdl,
                                    Y, U, V,
                                    luma_stride, chroma_stride, chroma_stride
                                    );
#endif

            break;
    }
    
    av_init_packet (&avpkt);
    avpkt.data = NULL;
    avpkt.size = 0;
    avpkt.flags = 0;
    
    m_pAVFrame_Input->data[0] = Y;
    m_pAVFrame_Input->data[1] = U;
    m_pAVFrame_Input->data[2] = V;
        
    m_pAVFrame_Input->linesize[0] = luma_stride;
    m_pAVFrame_Input->linesize[1] = chroma_stride;
    m_pAVFrame_Input->linesize[2] = chroma_stride;

    m_pAVFrame_Input->format = m_pAVCodecContext->pix_fmt;
    m_pAVFrame_Input->width = m_pAVCodecContext->width;
    m_pAVFrame_Input->height = m_pAVCodecContext->height;
        
#if (MMP_PLATFORM==MMP_PLATFORM_WIN32)
    m_pAVFrame_Input->pts = 0;
#else
    m_pAVFrame_Input->pts = AV_NOPTS_VALUE;
#endif

    m_pAVFrame_Input->quality = 100;

    iret = avcodec_encode_video2(m_pAVCodecContext, &avpkt, m_pAVFrame_Input, &got_packet_ptr);
    if(iret == 0) /* Success */ {
        
        /* get the delayed frames */
        if(got_packet_ptr == 0) {

             iret = avcodec_encode_video2(m_pAVCodecContext, &avpkt, NULL, &got_packet_ptr);
             if (iret < 0) {
                got_packet_ptr = 0;    
             }
        }
         
        if(got_packet_ptr == 1) {

            MMP_S32 i;
            enum MMP_FOURCC fourcc_out = this->get_fourcc_out();
            MMP_U8* p_src_data = (MMP_U8*)avpkt.data;
            MMP_S32 src_size = (MMP_S32)avpkt.size;
            MMP_U8* p_dst_data = (MMP_U8*)p_buf_vs->get_buf_vir_addr();
            MMP_S32 dst_size = p_buf_vs->get_buf_size();
            
            if( this->m_is_dsi == MMP_FALSE) {

                switch(fourcc_out) {

                    case MMP_FOURCC_VIDEO_MPEG4:
                        i = CMmpMpeg4Parser::GetFirstVOPIndex(p_src_data, src_size);
                        if(i > 0) {
                            p_buf_vs->alloc_dsi_buffer(i+16);
                            p_buf_vs->set_dsi_size(i);
                            memcpy(p_buf_vs->get_dsi_buffer(), p_src_data, i);
                            
                            this->m_is_dsi = MMP_TRUE;
                        }
                        break;

                    case MMP_FOURCC_VIDEO_H264:
                        {
                            MMP_S32 sps_idx, sps_size = 0;
                            MMP_S32 pps_idx, pps_size = 0;
                            MMP_S32 prefix = 0x01000000, dsi_idx;
                            MMP_U8* dsi_buf;

                            CMmpH264Parser::IsThisStreamHas(p_src_data, src_size, NAL_TYPE_SEQ_PARAM_SET, &sps_idx, &sps_size);
                            CMmpH264Parser::IsThisStreamHas(p_src_data, src_size, NAL_TYPE_PIC_PARAM_SET, &pps_idx, &pps_size);

                            if( (sps_size > 0) && (pps_size > 0) ) {
                                                            
                                p_buf_vs->alloc_dsi_buffer(sps_size + pps_size + 16);
                                
                                dsi_buf = (MMP_U8*)p_buf_vs->get_dsi_buffer();

                                dsi_idx = 0;
                                memcpy(&dsi_buf[dsi_idx], &prefix, 4);  dsi_idx+=4;
                                memcpy(&dsi_buf[dsi_idx], &p_src_data[sps_idx], sps_size); dsi_idx+=sps_size;
                                memcpy(&dsi_buf[dsi_idx], &prefix, 4);  dsi_idx+=4;
                                memcpy(&dsi_buf[dsi_idx], &p_src_data[pps_idx], pps_size); dsi_idx+=pps_size;
                            
                                p_buf_vs->set_dsi_size(dsi_idx);

                                this->m_is_dsi = MMP_TRUE;
                            }

                        }
                        break;

                }
            }

            memcpy((void*)p_dst_data, (void*)p_src_data, src_size);
            p_buf_vs->set_stream_size(src_size);
            if(avpkt.flags&AV_PKT_FLAG_KEY) {
                p_buf_vs->set_flag(mmp_buffer_media::FLAG_VIDEO_KEYFRAME);
            }
        }

    }
    
    av_free_packet(&avpkt);

    enc_end_tick = CMmpUtil::GetTickCount();

    p_buf_vs->set_coding_dur(enc_end_tick - enc_start_tick);

    return MMP_SUCCESS;
}

#endif