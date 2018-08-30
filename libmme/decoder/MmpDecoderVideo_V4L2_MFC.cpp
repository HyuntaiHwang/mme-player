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

#include "MmpDecoderVideo_V4L2_MFC.hpp"
#include "MmpUtil.hpp"
#include "mmp_buffer_mgr.hpp"

#include <sys/mman.h>
#include <poll.h>
#include "mfc/SsbSipMfcApi.h"
#include "csc/color_space_convertor.h"
#include "MmpH264Tool.hpp"

#define DUMP_NV12MT 0
#define DECODE_MON 0

static const enum MMP_FOURCC s_support_fourcc_out[]={
     MMP_FOURCC_IMAGE_YUV420,
     MMP_FOURCC_IMAGE_YUV420M,
     MMP_FOURCC_IMAGE_YVU420,
     MMP_FOURCC_IMAGE_YVU420M
};

/////////////////////////////////////////////////////////////
//CMmpDecoderVideo_V4L2_MFC Member Functions

CMmpDecoderVideo_V4L2_MFC::CMmpDecoderVideo_V4L2_MFC(struct CMmpDecoderVideo::create_config *p_create_config) : 
     
     CMmpDecoderVideo(p_create_config, "V4L2_MFC", s_support_fourcc_out, sizeof(s_support_fourcc_out)/sizeof(s_support_fourcc_out[0]))

,m_fp_dump(NULL)

,m_input_stream_count(0)
,m_decoded_picture_count(0)

,m_pic_width(0)
,m_pic_height(0)

,m_is_dsi(MMP_FALSE)
,m_v4l2_fd(-1)
,m_v4l2_fourcc_src((unsigned int)p_create_config->fourcc_in)
,m_v4l2_fourcc_dst(V4L2_PIX_FMT_NV12MT)
,m_framesize_src(1920*1088)
,m_framesize_dst(1920*1088*3/2)

,m_last_dst_dq_index(-1)
{
    MMP_S32 i;
    
    for(i = 0; i < MAX_BUF_COUNT_SRC; i++) {
        this->m_buf_src[i].type = V4L2_VIDEO_MFC_BUF_TYPE_DEC_SRC;
        this->m_buf_src[i].memory = V4L2_VIDEO_MFC_DEC_MEMORY_TYPE;
        this->m_buf_src[i].index = i;
        this->m_buf_src[i].m.planes = &this->m_planes_src[i][0];
        this->m_buf_src[i].length = 1; /* plane count */

        m_is_deque_src[i] = MMP_TRUE;
    }

    for(i = 0; i < MAX_BUF_COUNT_DST; i++) {
        this->m_buf_dst[i].type = V4L2_VIDEO_MFC_BUF_TYPE_DEC_DST;
        this->m_buf_dst[i].memory = V4L2_VIDEO_MFC_DEC_MEMORY_TYPE;
        this->m_buf_dst[i].index = i;
        this->m_buf_dst[i].m.planes = &this->m_planes_dst[i][0];
        this->m_buf_dst[i].length = 2; /* plane count */
    }
    
}

CMmpDecoderVideo_V4L2_MFC::~CMmpDecoderVideo_V4L2_MFC()
{

}

MMP_RESULT CMmpDecoderVideo_V4L2_MFC::Open()
{
    MMP_RESULT mmpResult;
    int iret;

    if(m_fp_dump != NULL) {
        fclose(m_fp_dump);
    }
    
    mmpResult=CMmpDecoderVideo::Open();
    if(mmpResult!=MMP_SUCCESS) {
        return mmpResult;
    }

    /* open v4l2 fd */
    if(mmpResult == MMP_SUCCESS) {
        this->m_v4l2_fd = v4l2_video_mfc_dec_open();
        if(this->m_v4l2_fd < 0) {
            MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[%s:%s] FAIL: v4l2_video_mfc_dec_open \n\r"),  MMP_CLASS_NAME, MMP_CLASS_FUNC ));
            mmpResult = MMP_FAILURE;
        }
    }

    /* setting src config */
    if(mmpResult == MMP_SUCCESS) {
        iret = v4l2_video_mfc_dec_set_config_src(this->m_v4l2_fd, this->m_v4l2_fourcc_src, m_framesize_src);
        if(iret != 0) {
            MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[%s:%s] FAIL: v4l2_video_mfc_dec_set_config_src \n\r"),  MMP_CLASS_NAME, MMP_CLASS_FUNC ));
            mmpResult = MMP_FAILURE;
        }
    }

#if 0
    /* setting dst config */
    if(mmpResult == MMP_SUCCESS) {
        iret = v4l2_video_mfc_dec_set_config_dst(this->m_v4l2_fd, this->m_v4l2_fourcc_dst);
        if(iret != 0) {
            MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[%s:%s] FAIL: v4l2_video_mfc_dec_set_config_dst \n\r"),  MMP_CLASS_NAME, MMP_CLASS_FUNC ));
            mmpResult = MMP_FAILURE;
        }
    }
#endif
    
    /* alloc src buf - req&query buf */
    if(mmpResult == MMP_SUCCESS) {
        iret = v4l2_video_mfc_dec_allocbuf_src(this->m_v4l2_fd, MAX_BUF_COUNT_SRC, this->m_buf_src, (int*)&m_buf_src_real_count);
        if(iret != 0) {
            MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[%s:%s] FAIL: v4l2_video_mfc_dec_allocbuf_src \n\r"),  MMP_CLASS_NAME, MMP_CLASS_FUNC ));
            mmpResult = MMP_FAILURE;
        }
        else {
        
            for(int i = 0; i < m_buf_src_real_count; i++) {
                m_p_src_buf[i] = (unsigned char*)mmap(NULL, this->m_buf_src[i].m.planes[0].length,  PROT_READ | PROT_WRITE, MAP_SHARED, this->m_v4l2_fd, this->m_buf_src[i].m.planes[0].m.mem_offset);
                MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[%s:%s] m_p_src_buf=0x%08x   this->m_buf_src[0].m.planes[0].length=%d   this->m_buf_src[0].m.planes[0].m.mem_offset=%d \n\r"),  
                              MMP_CLASS_NAME, MMP_CLASS_FUNC , __LINE__, 
                              m_p_src_buf,
                              this->m_buf_src[i].m.planes[0].length,
                              this->m_buf_src[i].m.planes[0].m.mem_offset
                              ));

                if(m_p_src_buf[i] == MAP_FAILED) {
                    MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[%s:%s] FAIL: srcbuf mmap \n\r"),  MMP_CLASS_NAME, MMP_CLASS_FUNC ));
                    mmpResult = MMP_FAILURE;
                    break;
                }
            }
        }
    }
    
    MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[%s:%s] Res=%d nForamt=(%s)  \n\r"),  MMP_CLASS_NAME, MMP_CLASS_FUNC, mmpResult, this->get_fourcc_in_name() ));

    return mmpResult;
}


MMP_RESULT CMmpDecoderVideo_V4L2_MFC::Close()
{
    MMP_RESULT mmpResult;
        
    mmpResult=CMmpDecoderVideo::Close();
    if(mmpResult!=MMP_SUCCESS)
    {
        MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpDecoderVideo_V4L2_MFC::Close] CMmpDecoderVideo::Close() \n\r")));
        return mmpResult;
    }

    
    if(this->m_v4l2_fd >= 0) {
        v4l2_video_mfc_dec_close(this->m_v4l2_fd);
        this->m_v4l2_fd = -1;
    }
        
    MMPDEBUGMSG(MMPZONE_MONITOR, (TEXT("[CMmpDecoderVideo_V4L2_MFC::Close] Success nForamt=(%s) \n\r"),  this->get_fourcc_in_name() ));

    return MMP_SUCCESS;
}


MMP_RESULT CMmpDecoderVideo_V4L2_MFC::DecodeDSI(class mmp_buffer_videostream* p_buf_vs, MMP_BOOL *is_reconfig) {

    MMP_RESULT mmpResult = MMP_SUCCESS;
    int i, j, iret;
    struct v4l2_format fmt;
    struct v4l2_pix_format_mplane pix_mp;
    unsigned char* p_src_buf;
    MMP_S32 src_stream_size;

    /* src: queue buf*/
    if(this->get_fourcc_in() == MMP_FOURCC_VIDEO_H264) {
    
        MMP_U8* p_dsi_src = (MMP_U8*)p_buf_vs->get_stream_real_ptr();

        if( (p_dsi_src[0] == 0) &&
            (p_dsi_src[1] == 0) &&
            (p_dsi_src[2] == 0) &&
            (p_dsi_src[3] == 1) ) {
        
            src_stream_size = p_buf_vs->get_stream_real_size();
            memcpy(this->m_p_src_buf[0], p_buf_vs->get_stream_real_ptr(), src_stream_size);
        }
        else {
            CMmpH264Parser::ConvertDSI_AVC1_To_H264(p_dsi_src /*MMP_U8* avc_dsi*/, (MMP_S32)p_buf_vs->get_stream_real_size() /*MMP_S32 avc_dsi_size*/, 
                                                  this->m_p_src_buf[0] /*MMP_U8* h264_dsi*/, &src_stream_size /*MMP_S32* h264_dsi_size*/ );    
        }

    }
    else {
        src_stream_size = p_buf_vs->get_stream_real_size();
        memcpy(this->m_p_src_buf[0], p_buf_vs->get_stream_real_ptr(), src_stream_size );
    }

    

    if(mmpResult == MMP_SUCCESS) {
        iret = v4l2_video_mfc_dec_qbuf_src(this->m_v4l2_fd, &this->m_buf_src[0], 0, p_buf_vs->get_stream_real_size());
        if(iret != 0) {
            MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[%s:%s] FAIL: v4l2_video_mfc_dec_qbuf_src \n\r"),  MMP_CLASS_NAME, MMP_CLASS_FUNC ));
            mmpResult = MMP_FAILURE;
        }
    }

    p_src_buf = this->m_p_src_buf[0];
    MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[%s:%s] sz=%d strm=(%02x %02x %02x %02x %02x %02x %02x %02x ) \n\r"),  MMP_CLASS_NAME, MMP_CLASS_FUNC, 
                     p_buf_vs->get_stream_real_size(),
                     p_src_buf[0], p_src_buf[1], p_src_buf[2], p_src_buf[3], 
                     p_src_buf[4], p_src_buf[5], p_src_buf[6], p_src_buf[7] 
                     ));

    /* src : streaming on */
    if(mmpResult == MMP_SUCCESS) {
        iret = v4l2_video_mfc_dec_stream_on_src(this->m_v4l2_fd);
        if(iret != 0) {
            MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[%s:%s] FAIL: v4l2_video_mfc_dec_stream_on_src \n\r"),  MMP_CLASS_NAME, MMP_CLASS_FUNC ));
            mmpResult = MMP_FAILURE;
        }
    }

    /*   check pic_width,pic_height
         As soon as starting streamming for src,  an interrupt occurs. 
         The function 'vidioc_g_fmt' in s5p_mfc_dec.c of kernel waits for being done interrupt-processing.
    */
    if(mmpResult == MMP_SUCCESS) {
        iret = v4l2_video_mfc_dec_get_fmt_dst(this->m_v4l2_fd, &fmt);
        MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[%s:%s] ln=%d  iret=%d \n\r"),  MMP_CLASS_NAME, MMP_CLASS_FUNC, __LINE__, iret ));
        if(iret == 0) {

            struct v4l2_crop crop;
            struct v4l2_crop* cr=&crop;

            v4l2_video_mfc_get_crop(this->m_v4l2_fd, &crop);

            pix_mp = fmt.fmt.pix_mp;
            MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[%s:%s] ln=%d  Buffer(W=%d H=%d) Cropy(%d %d %d %d) fourcc=%c%c%c%c \n\r"),  MMP_CLASS_NAME, MMP_CLASS_FUNC, __LINE__, 
                         pix_mp.width /*pix_mp.width mean buffer_width */,  pix_mp.height, /*pix_mp.width mean buffer_height */
                         cr->c.left, cr->c.top, cr->c.width, cr->c.height,
                         MMPGETFOURCCARG(pix_mp.pixelformat) 
                         ));

            m_pic_width = cr->c.width; 
            m_pic_height = cr->c.height;
                        
        }
        else {
            mmpResult = MMP_FAILURE;
            MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[%s:%s] FAIL: v4l2_video_mfc_dec_get_fmt_dst \n\r"),  MMP_CLASS_NAME, MMP_CLASS_FUNC ));
        }
    }
        
    /* dst : req&query buf (alloc buf) */
    if(mmpResult == MMP_SUCCESS) {
        iret = v4l2_video_mfc_dec_allocbuf_dst(this->m_v4l2_fd, MAX_BUF_COUNT_DST, this->m_buf_dst, (int*)&m_buf_dst_real_count);
        if(iret == 0) {

            this->init_buf_vf_from_decoder(m_pic_width, m_pic_height, this->get_fourcc_out(), m_buf_dst_real_count, m_buf_dst_real_count);
            if(is_reconfig) *is_reconfig = MMP_TRUE;
                     
            for(i = 0; i < m_buf_dst_real_count; i++) {

                m_p_dst_buf[i][0] = (unsigned char*)mmap(NULL, this->m_buf_dst[i].m.planes[0].length,  PROT_READ, MAP_SHARED, this->m_v4l2_fd, this->m_buf_dst[i].m.planes[0].m.mem_offset);
                m_p_dst_buf[i][1] = (unsigned char*)mmap(NULL, this->m_buf_dst[i].m.planes[1].length,  PROT_READ, MAP_SHARED, this->m_v4l2_fd, this->m_buf_dst[i].m.planes[1].m.mem_offset);
                
                MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[%s:%s] dstbuf %d. sz(%d %d  %d %d) moff(0x%08x 0x%08x) mmap_addr(0x%08x 0x%08x) \n\r"),  MMP_CLASS_NAME, MMP_CLASS_FUNC ,
                        i, 
                        this->m_buf_dst[i].m.planes[0].length, this->m_buf_dst[i].m.planes[1].length,
                        this->m_buf_dst[i].m.planes[0].bytesused, this->m_buf_dst[i].m.planes[1].bytesused,
                        this->m_buf_dst[i].m.planes[0].m.mem_offset, this->m_buf_dst[i].m.planes[1].m.mem_offset,
                        m_p_dst_buf[i][0], m_p_dst_buf[i][1]
                    ));
            }
        }
        else {
            MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[%s:%s] FAIL: v4l2_video_mfc_dec_allocbuf_dst \n\r"),  MMP_CLASS_NAME, MMP_CLASS_FUNC ));
            mmpResult = MMP_FAILURE;
        }
    }

#if 1
    /* dst : queue all the buffers into kernel  */
    if(mmpResult == MMP_SUCCESS) {
        m_framesize_dst = pix_mp.width*pix_mp.height*3/2;
        j = m_framesize_dst;
        for(i = 0; i < m_buf_dst_real_count; i++) {
            //iret = v4l2_video_mfc_dec_qbuf_dst(this->m_v4l2_fd, &this->m_buf_dst[i], i, j);
            iret = v4l2_video_mfc_dec_qbuf_dst(this->m_v4l2_fd, i);
            if(iret != 0) {
                MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[%s:%s] FAIL: v4l2_video_mfc_dec_qbuf_dst  i=%d/%d sz=%d \n\r"),  MMP_CLASS_NAME, MMP_CLASS_FUNC, i, m_buf_dst_real_count, j ));
                mmpResult = MMP_FAILURE;
            }
        }
    }

    /* dst : streaming on */
    if(mmpResult == MMP_SUCCESS) {
        iret = v4l2_video_mfc_dec_stream_on_dst(this->m_v4l2_fd);
        if(iret != 0) {
            MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[%s:%s] FAIL: v4l2_video_mfc_dec_stream_on_dst \n\r"),  MMP_CLASS_NAME, MMP_CLASS_FUNC ));
            mmpResult = MMP_FAILURE;
        }
    }
#endif

    /* src:  dequeue buf  from kernel   when POLL-OUT */
    if(mmpResult == MMP_SUCCESS) {
        int poll_state, poll_revents, dq_idx;
        do {
            poll_state = v4l2_video_mfc_poll_out(this->m_v4l2_fd, &poll_revents, V4L2_VIDEO_MFC_DEC_POLL_WAIT_TIMEOUT);
            if (poll_state > 0) {
                if (poll_revents & POLLOUT) {
                    dq_idx= -1;
                    iret = v4l2_video_mfc_dec_dqbuf_src(this->m_v4l2_fd, &dq_idx);
                    if(iret == 0) {
                        m_last_src_dq_index = dq_idx;
                        MMPDEBUGMSG(MMPZONE_MONITOR, (TEXT("[%s:%s] ln=%d dq_idx=%d \n\r"),  MMP_CLASS_NAME, MMP_CLASS_FUNC, __LINE__, dq_idx ));
                        break;
                    }
                    else {
                        MMPDEBUGMSG(MMPZONE_MONITOR, (TEXT("[%s:%s] ln=%d FAIL: v4l2_video_mfc_dec_dqbuf_src \n\r"),  MMP_CLASS_NAME, MMP_CLASS_FUNC, __LINE__ ));
                        mmpResult = MMP_FAILURE;
                        break;
                    }
                } else if (poll_revents & POLLERR) {
                    //ALOGE("[%s] POLLERR\n", __func__);
                    MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[%s:%s] ln=%d POLLERR \n\r"),  MMP_CLASS_NAME, MMP_CLASS_FUNC, __LINE__));
                    mmpResult = MMP_FAILURE;
                    break;
                } else {
                    //ALOGE("[%s] poll() returns 0x%x\n", __func__, poll_revents);
                    MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[%s:%s] ln=%d poll_revents=0x%x \n\r"),  MMP_CLASS_NAME, MMP_CLASS_FUNC, __LINE__, poll_revents));
                    //return MFC_GETOUTBUF_STATUS_NULL;
                    mmpResult = MMP_FAILURE;
                    break;
                }
            } 
            else if (poll_state < 0) {
                //return MFC_GETOUTBUF_STATUS_NULL;
                MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[%s:%s] ln=%d poll_state=0x%x \n\r"),  MMP_CLASS_NAME, MMP_CLASS_FUNC, __LINE__, poll_state));
                break;
            }
        } while (poll_state == 0);
    }

    
    return mmpResult;
}


MMP_RESULT CMmpDecoderVideo_V4L2_MFC::DecodeAu(class mmp_buffer_videostream* p_buf_vs, class mmp_buffer_videoframe** pp_buf_vf, MMP_BOOL *is_reconfig) {

    MMP_RESULT mmpResult = MMP_SUCCESS;
    int iret;
    class mmp_buffer_videoframe* p_buf_vf_decoded;
    MMP_U32 dec_start_tick, dec_end_tick;
    MMP_S32 i;
    unsigned char* p_src_buf;
    MMP_S32 src_stream_size;
    
#if (DUMP_NV12MT == 1)
    if(m_fp_dump == NULL) {
        m_fp_dump = fopen("/root/dump_nv12mt.yuv", "wb");    
    }
#endif
        
    if(is_reconfig) *is_reconfig = MMP_FALSE;

    /* Init Parmeter */
    if(pp_buf_vf != NULL) {
        *pp_buf_vf = NULL;
    }
    
    /* Decode DSI (Decoder Specific Info) */
    if(m_is_dsi == MMP_FALSE) {
        if(this->DecodeDSI(p_buf_vs, is_reconfig) != MMP_SUCCESS) {
            return MMP_FAILURE;
        }
        m_is_dsi = MMP_TRUE;
                
        return MMP_SUCCESS;
    }

    dec_start_tick = CMmpUtil::GetTickCount();

    for(i = 0; i < m_buf_src_real_count; i++) {
        if(m_is_deque_src[i] == MMP_TRUE) {
            m_last_src_dq_index = i;
            break;
        }
    }
    p_src_buf = this->m_p_src_buf[m_last_src_dq_index];

    /* src: queue buf */
    if(this->get_fourcc_in() == MMP_FOURCC_VIDEO_H264) {
        unsigned char* p_avc1 = (unsigned char*)p_buf_vs->get_stream_real_ptr();
        if( (p_avc1[0]==0) &&
            (p_avc1[1]==0) &&
            (p_avc1[2]==0) &&
            (p_avc1[3]==1) ) {
        
            src_stream_size = p_buf_vs->get_stream_real_size();
            memcpy(p_src_buf, p_avc1, src_stream_size );
        }
        else {
            //src_stream_size = p_buf_vs->get_stream_real_size();
            //CMmpH264Parser::ConvertAvc1ToH264(p_avc1, p_src_buf, src_stream_size);
            CMmpH264Parser::ConvertAvc1ToH264(p_avc1, p_buf_vs->get_stream_real_size(), 
                                              p_src_buf, &src_stream_size, MMP_FALSE);
        }
    }
    else {
        src_stream_size = p_buf_vs->get_stream_real_size();
        memcpy(p_src_buf, p_buf_vs->get_stream_real_ptr(), src_stream_size );
    }

    //dec_end_tick = CMmpUtil::GetTickCount();
    
    MMPDEBUGMSG(DECODE_MON, (TEXT("[%s:%s] strmidx=%d sz=%d qidx=%d strm=(%02x %02x %02x %02x %02x %02x %02x %02x ) \n\r"),  MMP_CLASS_NAME, MMP_CLASS_FUNC, 
                     m_input_stream_count,
                     src_stream_size,
                     m_last_src_dq_index,
                     p_src_buf[0], p_src_buf[1], p_src_buf[2], p_src_buf[3], 
                     p_src_buf[4], p_src_buf[5], p_src_buf[6], p_src_buf[7] 
                     ));
    

    if(mmpResult == MMP_SUCCESS) {
        iret = v4l2_video_mfc_dec_qbuf_src(this->m_v4l2_fd, &this->m_buf_src[m_last_src_dq_index], m_last_src_dq_index, src_stream_size);
        if(iret != 0) {
            MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[%s:%s] FAIL: v4l2_video_mfc_dec_qbuf_src   strmidx=%d strmsz=%d \n\r"),  MMP_CLASS_NAME, MMP_CLASS_FUNC, 
                     m_input_stream_count,
                     p_buf_vs->get_stream_real_size()  ));
            mmpResult = MMP_FAILURE;
        }
        else {
            m_is_deque_src[m_last_src_dq_index] = MMP_FALSE;
        }
    }

        
    
    /* src:  dequeue buf_src  from kernel   when POLL-OUT */
    if(mmpResult == MMP_SUCCESS) {
        int poll_state, poll_revents, dq_idx;
        do {
            poll_state = v4l2_video_mfc_poll_out(this->m_v4l2_fd, &poll_revents, 100);//V4L2_VIDEO_MFC_DEC_POLL_WAIT_TIMEOUT);
            if (poll_state > 0) {
                if (poll_revents & POLLOUT) {
                    dq_idx= -1;
                    iret = v4l2_video_mfc_dec_dqbuf_src(this->m_v4l2_fd, &dq_idx);
                    if(iret == 0) {
                        m_last_src_dq_index = dq_idx;
                        m_is_deque_src[dq_idx] = MMP_TRUE;

                        m_minheap_pts.Add(p_buf_vs->get_pts());

                        MMPDEBUGMSG(DECODE_MON, (TEXT("[%s:%s] ln=%d src dq_idx=%d \n\r"),  MMP_CLASS_NAME, MMP_CLASS_FUNC, __LINE__, dq_idx ));
                        break;
                    }
                    else {
                        MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[%s:%s] ln=%d FAIL: v4l2_video_mfc_dec_dqbuf_src  poll_revents=0x%08x \n\r"),  MMP_CLASS_NAME, MMP_CLASS_FUNC, __LINE__, poll_revents));
                    }
                } else if (poll_revents & POLLERR) {
                    //ALOGE("[%s] POLLERR\n", __func__);
                    MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[%s:%s] ln=%d POLLERR \n\r"),  MMP_CLASS_NAME, MMP_CLASS_FUNC, __LINE__));
                    mmpResult = MMP_FAILURE;
                    break;
                } else {
                    //ALOGE("[%s] poll() returns 0x%x\n", __func__, poll_revents);
                    MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[%s:%s] ln=%d poll_revents=0x%x \n\r"),  MMP_CLASS_NAME, MMP_CLASS_FUNC, __LINE__, poll_revents));
                    //return MFC_GETOUTBUF_STATUS_NULL;
                    mmpResult = MMP_FAILURE;
                    break;
                }
            } 
            else if (poll_state < 0) {
                //return MFC_GETOUTBUF_STATUS_NULL;
                MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[%s:%s] ln=%d poll_state=0x%x \n\r"),  MMP_CLASS_NAME, MMP_CLASS_FUNC, __LINE__, poll_state));
                break;
            }
            else if (poll_state == 0) {
            //    MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[%s:%s] ln=%d poll_state is Zero \n\r"),  MMP_CLASS_NAME, MMP_CLASS_FUNC, __LINE__));
            }
        } while (poll_state == 0);
    }

    
    MMP_BOOL is_poll_in = MMP_FALSE;
    //dec_start_tick = CMmpUtil::GetTickCount();
    /* src:  dequeue buf_src  from kernel   when POLL-IN */
    if(mmpResult == MMP_SUCCESS) {
        int poll_state, poll_revents, dq_idx;
        
        poll_state = v4l2_video_mfc_poll_in(this->m_v4l2_fd, &poll_revents, 100);//V4L2_VIDEO_MFC_DEC_POLL_WAIT_TIMEOUT);
        if (poll_state > 0) {
            if (poll_revents & POLLIN) {
                MMPDEBUGMSG(DECODE_MON, (TEXT("[%s:%s] ln=%d POLL-IN \n\r"),  MMP_CLASS_NAME, MMP_CLASS_FUNC, __LINE__));
                is_poll_in = MMP_TRUE;
            } else if (poll_revents & POLLERR) {
                //ALOGE("[%s] POLLERR\n", __func__);
                MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[%s:%s] ln=%d POLLERR \n\r"),  MMP_CLASS_NAME, MMP_CLASS_FUNC, __LINE__));
                mmpResult = MMP_FAILURE;
            } else {
                //ALOGE("[%s] poll() returns 0x%x\n", __func__, poll_revents);
                MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[%s:%s] ln=%d poll_revents=0x%x \n\r"),  MMP_CLASS_NAME, MMP_CLASS_FUNC, __LINE__, poll_revents));
                mmpResult = MMP_FAILURE;
            }
        } 
        else if (poll_state < 0) {
            //return MFC_GETOUTBUF_STATUS_NULL;
            MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[%s:%s] ln=%d poll_state=0x%x \n\r"),  MMP_CLASS_NAME, MMP_CLASS_FUNC, __LINE__, poll_state));
            mmpResult = MMP_FAILURE;
        }
        else  {
            MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[%s:%s] ln=%d poll_state=0x%x \n\r"),  MMP_CLASS_NAME, MMP_CLASS_FUNC, __LINE__, poll_state));
        }
        
    }
    //dec_end_tick = CMmpUtil::GetTickCount();

    /* dst :  dequeue buf  from kernel  */
    if( (mmpResult == MMP_SUCCESS) && (is_poll_in == MMP_TRUE) ) {
    
        int dq_idx;
        
        iret = v4l2_video_mfc_dec_dqbuf_dst(this->m_v4l2_fd, &dq_idx);
        if(iret == 0) {
            unsigned char* p_frame = m_p_dst_buf[dq_idx][0];
            unsigned char* p_frame1 = m_p_dst_buf[dq_idx][1];
            unsigned char* y_frame = NULL;
            unsigned char* u_frame = NULL;
            unsigned char* v_frame = NULL;
            int pic_width = m_pic_width;
            int pic_height = m_pic_height;
            enum MMP_FOURCC fourcc_decoded_buf;

            p_buf_vf_decoded = this->get_next_vf();
            if(p_buf_vf_decoded == NULL) {
                if(is_reconfig) *is_reconfig = MMP_TRUE;
            }
            else {
                fourcc_decoded_buf = p_buf_vf_decoded->get_fourcc();
                switch(fourcc_decoded_buf) {
                
                    case MMP_FOURCC_IMAGE_YUV420:
                        y_frame = (MMP_U8*)p_buf_vf_decoded->get_buf_vir_addr();
                        u_frame = y_frame + V4L2_VIDEO_YUV420_U_FRAME_OFFSET(pic_width, pic_height);
                        v_frame = y_frame + V4L2_VIDEO_YUV420_V_FRAME_OFFSET(pic_width, pic_height);
                        break;

                    case MMP_FOURCC_IMAGE_YVU420:

                        if( ((pic_width/2) % 16)  == 0 ) {
                            y_frame = (MMP_U8*)p_buf_vf_decoded->get_buf_vir_addr();
                            u_frame = y_frame + V4L2_VIDEO_YVU420_U_FRAME_OFFSET(pic_width, pic_height);
                            v_frame = y_frame + V4L2_VIDEO_YVU420_V_FRAME_OFFSET(pic_width, pic_height);
                        }
                        else {
                            /*
                                Diamond  rendering U 위치 비정상 
                                32를 빼야 한다. ???? 
                            */
                            y_frame = (MMP_U8*)p_buf_vf_decoded->get_buf_vir_addr();
                            v_frame = y_frame + V4L2_BYTE_ALIGN(pic_width,16) * pic_height;
                            u_frame = v_frame + V4L2_BYTE_ALIGN(pic_width/2, 16) * pic_height/2 - 32;
                        
#if 0      
                            {
                                int kkk = pic_width*V4L2_BYTE_ALIGN(pic_height,16)/4;// + (pic_width/2)*2;
                                memset(u_frame, 128, kkk);
                                memset(v_frame, 128, kkk);
                                //memset(u_frame, 0, V4L2_BYTE_ALIGN(pic_width/2, 16) * (pic_height/2) / 2  );
                            }
#endif
                        }


                        break;

                    case MMP_FOURCC_IMAGE_YUV420M:
                        y_frame = (MMP_U8*)p_buf_vf_decoded->get_buf_vir_addr(0);
                        u_frame = (MMP_U8*)p_buf_vf_decoded->get_buf_vir_addr(1);
                        v_frame = (MMP_U8*)p_buf_vf_decoded->get_buf_vir_addr(2);
                        break;

                    default:
                        p_buf_vf_decoded = NULL;
                        break;
                }


                if(p_buf_vf_decoded != NULL) {
#if 1 //Luma
                    csc_tiled_to_linear_y_neon(
                        (unsigned char *)y_frame, //pYUVBuf[0],
                        (unsigned char *)p_frame, //outputInfo.YVirAddr,
                        pic_width, //this->get_pic_width(),
                        pic_height  //this->get_pic_height()
                        );
#endif

#if 1 //chroma 
                    csc_tiled_to_linear_uv_deinterleave_neon(u_frame, v_frame, p_frame1, V4L2_VIDEO_YVU420_U_STRIDE(pic_width)*2, pic_height/2);
                    //csc_tiled_to_linear_uv_deinterleave_neon(u_frame, v_frame, p_frame1, pic_width, pic_height/2);
#else 

                    unsigned char* u_temp = new unsigned char[pic_width*pic_height];
                    unsigned char* v_temp = new unsigned char[pic_width*pic_height];
                    
                    unsigned char* u_t, *v_t;
                    //csc_tiled_to_linear_uv_deinterleave_neon(u_temp, v_temp, p_frame1, pic_width, pic_height/2);
                    memset(u_temp, 0, pic_width*pic_height);
                    memset(v_temp, 0, pic_width*pic_height);

                    
                        int x, y;
                        u_t = u_temp;
                        v_t = v_temp;
                        for(y = 0; y < pic_height/2; y+=2) {
                        //for(y = 0; y < 2; y+=2) {
                        //for(y = 0; y < 10; y+=2) {
                            
                            memcpy(u_frame, u_t, pic_width/2);
                            memcpy(v_frame, v_t, pic_width/2);
                            //memcpy(u_frame, u_t, V4L2_BYTE_ALIGN(pic_width/2 , 16) );
                            //memcpy(v_frame, v_t, V4L2_BYTE_ALIGN(pic_width/2 , 16) );
                            
                            //u_frame += V4L2_BYTE_ALIGN(pic_width/2 , 16);
                           // v_frame += V4L2_BYTE_ALIGN(pic_width/2 , 16);
                            u_frame += V4L2_BYTE_ALIGN(pic_width/2 , 16);
                            v_frame += V4L2_BYTE_ALIGN(pic_width/2 , 16);
                            
                            //u_t += pic_width/2;
                            //v_t += pic_width/2;
                        }

#if 0
                      {
                          static FILE* fp = NULL;
                          if(fp == NULL) {
                              fp = fopen("/data/aa.yuv", "wb");
                          }
                          if(fp != NULL) {
                               fwrite(y_frame, 1, pic_width*pic_height, fp);
                               fwrite(u_temp, 1, pic_width*pic_height/4, fp);
                               fwrite(v_temp, 1, pic_width*pic_height/4, fp);
                          }
                      }
#endif

                    delete [] u_temp;
                    delete [] v_temp;
                    
#endif


                    MMP_S64 buf_pts;
                    m_minheap_pts.Delete(buf_pts);
                    p_buf_vf_decoded->set_pts(buf_pts);
                    p_buf_vf_decoded->clear_own();

                    dec_end_tick = CMmpUtil::GetTickCount();
                    p_buf_vf_decoded->set_coding_dur(dec_end_tick - dec_start_tick);
                    //CMmpDecoderVideo::DecodeMonitor(p_buf_vf_decoded);
                }

                if(pp_buf_vf) *pp_buf_vf = p_buf_vf_decoded;

                if(m_fp_dump != NULL) {
                    int wrsz;
                    MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[%s:%s] 1 y_frame=0x%08x \n\r"),  MMP_CLASS_NAME, MMP_CLASS_FUNC, __LINE__, y_frame ));
                    wrsz = fwrite(y_frame, 1, this->get_decoder_pic_width()*this->get_decoder_pic_height(), m_fp_dump);
                    MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[%s:%s] 1 dump wrsz=%d \n\r"),  MMP_CLASS_NAME, MMP_CLASS_FUNC, wrsz ));
                }

            }

            
            if(m_last_dst_dq_index >= 0) {
                //v4l2_video_mfc_dec_qbuf_dst(this->m_v4l2_fd, &this->m_buf_dst[m_last_dst_dq_index], m_last_dst_dq_index, m_framesize_dst); 
                v4l2_video_mfc_dec_qbuf_dst(this->m_v4l2_fd, m_last_dst_dq_index); 
            }
            m_last_dst_dq_index = dq_idx;

            //MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[%s:%s] ln=%d dq_idx = %d "),  MMP_CLASS_NAME, MMP_CLASS_FUNC, __LINE__, dq_idx ));
            //break;
        }
        else {
            /* Decoding Only */
            MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[%s:%s] FAIL: v4l2_video_mfc_dec_dqbuf_dst  ln=%d iret=%d m_input_stream_count=%d \n\r"),  MMP_CLASS_NAME, MMP_CLASS_FUNC, __LINE__, iret, m_input_stream_count ));
            //break;
        }
    }
       

    
    m_input_stream_count++;

	return mmpResult;
}

