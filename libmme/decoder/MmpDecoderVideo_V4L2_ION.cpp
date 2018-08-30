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

#include "MmpDecoderVideo_V4L2_ION.hpp"
#include "MmpUtil.hpp"
#include "mmp_buffer_mgr.hpp"

#include <sys/mman.h>
#include <poll.h>
#include "mfc/SsbSipMfcApi.h"
#include "csc/color_space_convertor.h"

#define DUMP_NV12MT 0
#define DECODE_MON 0
#define MMP_FOURCC_IMAGE_MFC_OUT  MMP_FOURCC_IMAGE_NV12MT

/////////////////////////////////////////////////////////////
//CMmpDecoderVideo_V4L2_ION Member Functions

CMmpDecoderVideo_V4L2_ION::CMmpDecoderVideo_V4L2_ION(struct mmp_decoder_video_create_config *p_create_config) : CMmpDecoderVideo(p_create_config, "V4L2_MFC")

,m_fp_dump(NULL)

,m_input_stream_count(0)
,m_decoded_picture_count(0)

,m_pic_width(0)
,m_pic_height(0)

,m_is_dsi(MMP_FALSE)
,m_v4l2_fd(-1)

,m_last_dst_dq_index(-1)
{
    
}

CMmpDecoderVideo_V4L2_ION::~CMmpDecoderVideo_V4L2_ION()
{

}

MMP_RESULT CMmpDecoderVideo_V4L2_ION::Open()
{
    MMP_RESULT mmpResult;
    int iret;
    MMP_S32 i;

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
        iret = v4l2_video_mfc_dec_set_config_src(this->m_v4l2_fd, this->get_fourcc_in(), 0x100000 /* 1M */ );
        if(iret != 0) {
            MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[%s:%s] FAIL: v4l2_video_mfc_dec_set_config_src \n\r"),  MMP_CLASS_NAME, MMP_CLASS_FUNC ));
            mmpResult = MMP_FAILURE;
        }
    }

    /* setting dst config */
    if(mmpResult == MMP_SUCCESS) {
        iret = v4l2_video_mfc_dec_set_config_dst(this->m_v4l2_fd, MMP_FOURCC_IMAGE_MFC_OUT);
        if(iret != 0) {
            MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[%s:%s] FAIL: v4l2_video_mfc_dec_set_config_dst \n\r"),  MMP_CLASS_NAME, MMP_CLASS_FUNC ));
            mmpResult = MMP_FAILURE;
        }
    }
    
    /* alloc src buf - req&query buf */
    if(mmpResult == MMP_SUCCESS) {
        iret = v4l2_video_mfc_dec_allocbuf_src(this->m_v4l2_fd, MAX_BUF_COUNT_SRC, NULL, NULL);
        if(iret != 0) {
            MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[%s:%s] FAIL: v4l2_video_mfc_dec_allocbuf_src \n\r"),  MMP_CLASS_NAME, MMP_CLASS_FUNC ));
            mmpResult = MMP_FAILURE;
        }
    }
    
    MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[%s:%s] Res=%d nForamt=(%s)  \n\r"),  MMP_CLASS_NAME, MMP_CLASS_FUNC, mmpResult, this->get_fourcc_in_name() ));

    return mmpResult;
}


MMP_RESULT CMmpDecoderVideo_V4L2_ION::Close()
{
    MMP_RESULT mmpResult;
    MMP_S32 i;
        
    mmpResult=CMmpDecoderVideo::Close();
    if(mmpResult!=MMP_SUCCESS)
    {
        MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpDecoderVideo_V4L2_ION::Close] CMmpDecoderVideo::Close() \n\r")));
        return mmpResult;
    }

    
    if(this->m_v4l2_fd >= 0) {
        v4l2_video_mfc_dec_close(this->m_v4l2_fd);
        this->m_v4l2_fd = -1;
    }
        
    MMPDEBUGMSG(MMPZONE_MONITOR, (TEXT("[CMmpDecoderVideo_V4L2_ION::Close] Success nForamt=(%s) \n\r"),  this->get_fourcc_in_name() ));

    return MMP_SUCCESS;
}

MMP_BOOL CMmpDecoderVideo_V4L2_ION::is_support_this_fourcc_in(enum MMP_FOURCC fourcc) {
    
    MMP_BOOL bflag;

    switch(fourcc) {
        //case MMP_FOURCC_VIDEO_H263:
        case MMP_FOURCC_VIDEO_H264:
        case MMP_FOURCC_VIDEO_MPEG4:
        //case MMP_FOURCC_VIDEO_MPEG2:
        //case MMP_FOURCC_VIDEO_VC1:
        //case MMP_FOURCC_VIDEO_WMV1:
        //case MMP_FOURCC_VIDEO_WMV2:
        //case MMP_FOURCC_VIDEO_WMV3:
        //case MMP_FOURCC_VIDEO_RV30:
        //case MMP_FOURCC_VIDEO_RV40:
            bflag = MMP_TRUE;
            break;

        default:
            bflag = MMP_FALSE;
            break;
    }

    return bflag;
}

MMP_BOOL CMmpDecoderVideo_V4L2_ION::is_support_this_fourcc_out(enum MMP_FOURCC fourcc) {

    MMP_BOOL bflag;

    switch(fourcc) {
        case MMP_FOURCC_IMAGE_YUV420:
        case MMP_FOURCC_IMAGE_YVU420:
        case MMP_FOURCC_IMAGE_YUV420M:
        case MMP_FOURCC_IMAGE_YVU420M:
        case MMP_FOURCC_IMAGE_NV12:
        case MMP_FOURCC_IMAGE_NV21:
        case MMP_FOURCC_IMAGE_NV12M:
        case MMP_FOURCC_IMAGE_NV21M:
            bflag = MMP_TRUE;
            break;

        default:
            bflag = MMP_FALSE;
            break;
    }

    return bflag;
}

MMP_RESULT CMmpDecoderVideo_V4L2_ION::DecodeDSI(class mmp_buffer_videostream* p_buf_vs, MMP_BOOL *is_reconfig) {

    MMP_RESULT mmpResult = MMP_SUCCESS;
    int i, j, iret;
    struct v4l2_format fmt;
    struct v4l2_pix_format_mplane pix_mp;
    MMP_U8* p_src_buf = (MMP_U8*)p_buf_vs->get_buf_vir_addr();
    
    /* src: queue buf*/
    if(mmpResult == MMP_SUCCESS) {
        
        struct v4l2_ion_buffer v4l2_ion_stream_src;

        v4l2_ion_stream_src.shared_fd = p_buf_vs->get_buf_shared_fd();
        v4l2_ion_stream_src.buf_size = p_buf_vs->get_buf_size();
        v4l2_ion_stream_src.vir_addr = p_buf_vs->get_buf_vir_addr();
        v4l2_ion_stream_src.mem_offset = 0;
        iret = v4l2_video_mfc_dec_qbuf_ion_src(this->m_v4l2_fd, &v4l2_ion_stream_src);
        if(iret != 0) {
            MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[%s:%s] FAIL: v4l2_video_mfc_dec_qbuf_ion_src \n\r"),  MMP_CLASS_NAME, MMP_CLASS_FUNC ));
            mmpResult = MMP_FAILURE;
        }
    }
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
            
            this->config_buffer(m_pic_width, m_pic_height, 4);
            if(is_reconfig) *is_reconfig = MMP_TRUE;
        }
        else {
            mmpResult = MMP_FAILURE;
            MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[%s:%s] FAIL: v4l2_video_mfc_dec_get_fmt_dst \n\r"),  MMP_CLASS_NAME, MMP_CLASS_FUNC ));
        }
    }
        
    /* dst : req&query buf (alloc buf) */
    if(mmpResult == MMP_SUCCESS) {
        iret = v4l2_video_mfc_dec_allocbuf_dst(this->m_v4l2_fd, MAX_BUF_COUNT_DST, NULL, NULL);
        if(iret == 0) {
            
        }
        else {
            MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[%s:%s] FAIL: v4l2_video_mfc_dec_allocbuf_dst \n\r"),  MMP_CLASS_NAME, MMP_CLASS_FUNC ));
            mmpResult = MMP_FAILURE;
        }
    }

#if 1
    /* dst : queue all the buffers into kernel  */
    if(mmpResult == MMP_SUCCESS) {

        for(i = 0; i < MAX_BUF_COUNT_DST; i++) {
            m_p_buf_vf_mfc[i] = mmp_buffer_mgr::get_instance()->alloc_media_videoframe(m_pic_width, m_pic_height, MMP_FOURCC_IMAGE_MFC_OUT); 
            if(m_p_buf_vf_mfc[i] == NULL) {
                MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[%s:%s] FAIL: alloc_media_videoframe(NV12MT)\n\r"),  MMP_CLASS_NAME, MMP_CLASS_FUNC));
                mmpResult = MMP_FAILURE;
                break;
            }
        }

        if(mmpResult == MMP_SUCCESS) {
            for(i = 0; i < MAX_BUF_COUNT_DST; i++) {
                //iret = v4l2_video_mfc_dec_qbuf_dst(this->m_v4l2_fd, &this->m_buf_dst[i], i, j);
                struct v4l2_ion_frame v4l2_ion_stream_dst;

                v4l2_ion_stream_dst.index = i;
                v4l2_ion_stream_dst.plane_count = 2;
                for(j = 0; j < v4l2_ion_stream_dst.plane_count; j++) {
                    v4l2_ion_stream_dst.plane[j].shared_fd = m_p_buf_vf_mfc[i]->get_buf_shared_fd(j);
                    v4l2_ion_stream_dst.plane[j].buf_size = m_p_buf_vf_mfc[i]->get_buf_size(j);
                    v4l2_ion_stream_dst.plane[j].vir_addr = (unsigned int)m_p_buf_vf_mfc[i]->get_buf_vir_addr(j);
                    v4l2_ion_stream_dst.plane[j].mem_offset = 0;
                }
                        
                iret = v4l2_video_mfc_dec_qbuf_ion_dst(this->m_v4l2_fd, &v4l2_ion_stream_dst);
                if(iret != 0) {
                    MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[%s:%s] FAIL: v4l2_video_mfc_dec_qbuf_dst  i=%d/%d sz=%d \n\r"),  MMP_CLASS_NAME, MMP_CLASS_FUNC, i, MAX_BUF_COUNT_DST, j ));
                    mmpResult = MMP_FAILURE;
                }
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
                        MMPDEBUGMSG(MMPZONE_MONITOR, (TEXT("[%s:%s] ln=%d dq_idx=%d \n\r"),  MMP_CLASS_NAME, MMP_CLASS_FUNC, __LINE__, dq_idx ));
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


MMP_RESULT CMmpDecoderVideo_V4L2_ION::DecodeAu(class mmp_buffer_videostream* p_buf_vs, class mmp_buffer_videoframe** pp_buf_vf, MMP_BOOL *is_reconfig) {

    MMP_RESULT mmpResult = MMP_SUCCESS;
    int iret;
    class mmp_buffer_videoframe* p_buf_vf_decoded;
    MMP_U32 dec_start_tick, dec_end_tick;
    MMP_U8* p_src_buf = (MMP_U8*)p_buf_vs->get_buf_vir_addr();
    
#if (DUMP_NV12MT == 1)
    if(m_fp_dump == NULL) {
        m_fp_dump = fopen("/root/dump_nv12mt.yuv", "wb");    
    }
#endif
        
    if(is_reconfig) *is_reconfig = MMP_FALSE;

    /* Init Parmeter */
    p_buf_vs->set_used_byte(0);
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

    if(mmpResult == MMP_SUCCESS) {
        struct v4l2_ion_buffer v4l2_ion_stream_src;

        v4l2_ion_stream_src.shared_fd = p_buf_vs->get_buf_shared_fd();
        v4l2_ion_stream_src.buf_size = p_buf_vs->get_buf_size();
        v4l2_ion_stream_src.vir_addr = p_buf_vs->get_buf_vir_addr();
        v4l2_ion_stream_src.mem_offset = 0;
        iret = v4l2_video_mfc_dec_qbuf_ion_src(this->m_v4l2_fd, &v4l2_ion_stream_src);
        if(iret != 0) {
            MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[%s:%s] FAIL: v4l2_video_mfc_dec_qbuf_ion_src \n\r"),  MMP_CLASS_NAME, MMP_CLASS_FUNC ));
            mmpResult = MMP_FAILURE;
        }
    }

    //dec_end_tick = CMmpUtil::GetTickCount();
    MMPDEBUGMSG(DECODE_MON, (TEXT("[%s:%s] strmidx=%d sz=%d strm=(%02x %02x %02x %02x %02x %02x %02x %02x ) \n\r"),  MMP_CLASS_NAME, MMP_CLASS_FUNC, 
                     m_input_stream_count,
                     p_buf_vs->get_stream_real_size(),
                     p_src_buf[0], p_src_buf[1], p_src_buf[2], p_src_buf[3], 
                     p_src_buf[4], p_src_buf[5], p_src_buf[6], p_src_buf[7] 
                     ));
        
    
    /* src:  dequeue buf_src  from kernel   when POLL-OUT */
    if(mmpResult == MMP_SUCCESS) {
        int poll_state, poll_revents, dq_idx;
        do {
            poll_state = v4l2_video_mfc_poll_out(this->m_v4l2_fd, &poll_revents, V4L2_VIDEO_MFC_DEC_POLL_WAIT_TIMEOUT);
            if (poll_state > 0) {
                if (poll_revents & POLLOUT) {
                    dq_idx= -1;
                    iret = v4l2_video_mfc_dec_dqbuf_src(this->m_v4l2_fd, &dq_idx);
                    if(iret == 0) {
                        MMPDEBUGMSG(DECODE_MON, (TEXT("[%s:%s] ln=%d src dq_idx=%d \n\r"),  MMP_CLASS_NAME, MMP_CLASS_FUNC, __LINE__, dq_idx ));
                        break;
                    }
                    else {
                        MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[%s:%s] ln=%d FAIL: v4l2_video_mfc_dec_dqbuf_src \n\r"),  MMP_CLASS_NAME, MMP_CLASS_FUNC, __LINE__));
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

    

    //dec_start_tick = CMmpUtil::GetTickCount();
    /* src:  dequeue buf_src  from kernel   when POLL-IN */
    if(mmpResult == MMP_SUCCESS) {
        int poll_state, poll_revents, dq_idx;
        //do {
            poll_state = v4l2_video_mfc_poll_in(this->m_v4l2_fd, &poll_revents, V4L2_VIDEO_MFC_DEC_POLL_WAIT_TIMEOUT);
            if (poll_state > 0) {
                if (poll_revents & POLLIN) {
                    MMPDEBUGMSG(DECODE_MON, (TEXT("[%s:%s] ln=%d POLL-IN \n\r"),  MMP_CLASS_NAME, MMP_CLASS_FUNC, __LINE__));
                    //break;
                } else if (poll_revents & POLLERR) {
                    //ALOGE("[%s] POLLERR\n", __func__);
                    MMPDEBUGMSG(DECODE_MON, (TEXT("[%s:%s] ln=%d POLLERR \n\r"),  MMP_CLASS_NAME, MMP_CLASS_FUNC, __LINE__));
                    mmpResult = MMP_FAILURE;
                    //break;
                } else {
                    //ALOGE("[%s] poll() returns 0x%x\n", __func__, poll_revents);
                    MMPDEBUGMSG(DECODE_MON, (TEXT("[%s:%s] ln=%d poll_revents=0x%x \n\r"),  MMP_CLASS_NAME, MMP_CLASS_FUNC, __LINE__, poll_revents));
                    //return MFC_GETOUTBUF_STATUS_NULL;
                    mmpResult = MMP_FAILURE;
                    //break;
                }
            } 
            else { //if (poll_state < 0) {
                //return MFC_GETOUTBUF_STATUS_NULL;
                MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[%s:%s] ln=%d poll_state=0x%x \n\r"),  MMP_CLASS_NAME, MMP_CLASS_FUNC, __LINE__, poll_state));
                mmpResult = MMP_FAILURE;
                //break;
            }
            
        //} while (poll_state == 0);
    }
    //dec_end_tick = CMmpUtil::GetTickCount();

    /* dst :  dequeue buf  from kernel  */
    if(mmpResult == MMP_SUCCESS) {
    
        int dq_idx;
        
        iret = v4l2_video_mfc_dec_dqbuf_dst(this->m_v4l2_fd, &dq_idx);
        if(iret == 0) {
            unsigned char* p_frame = m_p_buf_vf_mfc[dq_idx]->get_buf_vir_addr(0);
            unsigned char* p_frame1 = m_p_buf_vf_mfc[dq_idx]->get_buf_vir_addr(1);
            unsigned char* y_frame;
            unsigned char* u_frame;
            unsigned char* v_frame;
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
                        y_frame = p_buf_vf_decoded->get_buf_vir_addr();
                        u_frame = y_frame + V4L2_VIDEO_YUV420_U_FRAME_OFFSET(pic_width, pic_height);
                        v_frame = y_frame + V4L2_VIDEO_YUV420_V_FRAME_OFFSET(pic_width, pic_height);
                        break;

                    case MMP_FOURCC_IMAGE_YVU420:
                        y_frame = p_buf_vf_decoded->get_buf_vir_addr();
                        u_frame = y_frame + V4L2_VIDEO_YVU420_U_FRAME_OFFSET(pic_width, pic_height);
                        v_frame = y_frame + V4L2_VIDEO_YVU420_V_FRAME_OFFSET(pic_width, pic_height);
                        break;

                    case MMP_FOURCC_IMAGE_YUV420M:
                        y_frame = p_buf_vf_decoded->get_buf_vir_addr(0);
                        u_frame = p_buf_vf_decoded->get_buf_vir_addr(1);
                        v_frame = p_buf_vf_decoded->get_buf_vir_addr(2);
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
#if 1 //Croma 
                    csc_tiled_to_linear_uv_deinterleave_neon(u_frame, v_frame, p_frame1, pic_width, pic_height/2);
#endif

                    p_buf_vf_decoded->set_pts(p_buf_vs->get_pts());

                    dec_end_tick = CMmpUtil::GetTickCount();
                    p_buf_vf_decoded->set_coding_dur(dec_end_tick - dec_start_tick);
                    //CMmpDecoderVideo::DecodeMonitor(p_buf_vf_decoded);
                }

                if(pp_buf_vf) *pp_buf_vf = p_buf_vf_decoded;
            }

            if(m_fp_dump != NULL) {
                int wrsz;
                MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[%s:%s] 1 y_frame=0x%08x \n\r"),  MMP_CLASS_NAME, MMP_CLASS_FUNC, __LINE__, y_frame ));
                wrsz = fwrite(y_frame, 1, this->get_pic_width()*this->get_pic_height(), m_fp_dump);
                MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[%s:%s] 1 dump wrsz=%d \n\r"),  MMP_CLASS_NAME, MMP_CLASS_FUNC, wrsz ));
            }

            if(m_last_dst_dq_index >= 0) {

                struct v4l2_ion_frame v4l2_ion_stream_dst;
                int i,j;
                i = m_last_dst_dq_index;
                v4l2_ion_stream_dst.index = i;
                v4l2_ion_stream_dst.plane_count = 2;
                for(j = 0; j < v4l2_ion_stream_dst.plane_count; j++) {
                    v4l2_ion_stream_dst.plane[j].shared_fd = m_p_buf_vf_mfc[i]->get_buf_shared_fd(j);
                    v4l2_ion_stream_dst.plane[j].buf_size = m_p_buf_vf_mfc[i]->get_buf_size(j);
                    v4l2_ion_stream_dst.plane[j].vir_addr = (unsigned int)m_p_buf_vf_mfc[i]->get_buf_vir_addr(j);
                    v4l2_ion_stream_dst.plane[j].mem_offset = 0;
                }
                        
                iret = v4l2_video_mfc_dec_qbuf_ion_dst(this->m_v4l2_fd, &v4l2_ion_stream_dst);
                if(iret != 0) {
                    MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[%s:%s] FAIL: v4l2_video_mfc_dec_qbuf_dst  i=%d/%d sz=%d \n\r"),  MMP_CLASS_NAME, MMP_CLASS_FUNC, i, MAX_BUF_COUNT_DST, j ));
                    //mmpResult = MMP_FAILURE;
                }
                //v4l2_video_mfc_dec_qbuf_dst(this->m_v4l2_fd, m_last_dst_dq_index); 
            }
            m_last_dst_dq_index = dq_idx;
            //break;
        }
        else {
            /* Decoding Only */
            MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[%s:%s] FAIL: v4l2_video_mfc_dec_dqbuf_dst  ln=%d iret=%d m_input_stream_count=%d \n\r"),  MMP_CLASS_NAME, MMP_CLASS_FUNC, __LINE__, iret, m_input_stream_count ));
            //break;
        }
    }
       

    
    m_input_stream_count++;

	return MMP_SUCCESS;
}

