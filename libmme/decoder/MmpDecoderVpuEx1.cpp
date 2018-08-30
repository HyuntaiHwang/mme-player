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

#include "MmpDecoderVpuEx1.hpp"
#include "MmpUtil.hpp"
#include "vpuhelper.h"
#include "MmpH264Tool.hpp"
#include "mmp_buffer_mgr.hpp"
#include "MmpWMVTool.hpp"
#include "MmpImageTool.hpp"

#define VPU_DEC_TIMEOUT       5000
//#define VPU_WAIT_TIME_OUT	100		    //should be less than normal decoding time to give a chance to fill stream. if this value happens some problem. we should fix VPU_WaitInterrupt function
//#define PARALLEL_VPU_WAIT_TIME_OUT 0 	//the value of timeout is 0 means we just check interrupt flag. do not wait any time to give a chance of an interrupt of the next core.

#define EXTRA_FRAME_BUFFER_NUM	1
#define STREAM_BUF_SIZE		 0x300000  // max bitstream size
#define STREAM_END_SIZE			0

#ifdef WIN32
/* this is test param on WIN32 */
int VPU_PIC_WIDTH = 320; 
int VPU_PIC_HEIGHT = MMP_BYTE_ALIGN(240, 16);
#endif

/////////////////////////////////////////////////////////////
//CMmpDecoderVpuEx1 Member Functions

CMmpDecoderVpuEx1::CMmpDecoderVpuEx1(enum MMP_FOURCC fourcc_in) :

m_fourcc_in(fourcc_in)
,m_fourcc_out(MMP_FOURCC_IMAGE_YUV420)
,m_vpu_buf_width(0)
,m_vpu_buf_height(0)

,m_vpu_instance_index(-1)
,m_codec_idx(0)
,m_version(0)
,m_revision(0)
,m_productId(0)
,m_iframeSearchEnable(0)
,m_skipframeMode(0)
,m_randomaccess(MMP_PLAY_BACK)

,m_regFrameBufCount(0)

,m_DecHandle(NULL)

,m_p_stream_buffer(NULL)


,m_is_alloc_vf(MMP_FALSE)
,m_is_complete_decode_dsi(MMP_FALSE)

,m_last_int_reason(0)
,m_input_stream_count(0)

#if (VPU_SUPPORT_THEORA == 1)
,m_thoParser(NULL)
#endif

,m_is_streamming_on(MMP_FALSE)

/* h264, avc */
,m_is_avc(0)
,m_nal_length_size(0)
, m_last_display_index(-1)

,m_copyed_aubuf(NULL)
,m_copyed_aubuf_max_size(0)

,m_DEBUG_VPUDec(0)

#ifdef CMmpDecoderVpuEx1_DUMP_RAW_STREAM
,m_fp_dump_raw_stream(NULL)
#endif

{
    int i;

    memset(&m_decOP, 0x00, sizeof(m_decOP));
    memset(&m_dec_init_info, 0x00, sizeof(m_dec_init_info));

    for(i = 0; i < MAX_FRAMEBUFFER_COUNT; i++) {
        memset(&m_vbFrame[i], 0x00, sizeof(vpu_buffer_t));
    }
    
    for(i = 0; i < MAX_FRAMEBUFFER_COUNT; i++) {
        m_p_buf_vf_arr[i] = NULL;
        m_buf_used_count[i] = 0;
        m_p_buf_vf_req_clear_before_dsi[i] = NULL;
    }

    m_DEBUG_VPUDec = CMmpUtil::IsDebugEnable_VPUDec();
    
#ifdef CMmpDecoderVpuEx1_DUMP_RAW_STREAM
    sprintf(m_filename_dump_raw_stream, "/data/vpu_raw_stream-%d.bin", CMmpUtil::GetTickCount() );
    ALOGI("[VPU] Open Dump RawStream File: %s ", m_filename_dump_raw_stream);
    m_fp_dump_raw_stream = fopen(m_filename_dump_raw_stream, "wb");
#endif
}

CMmpDecoderVpuEx1::~CMmpDecoderVpuEx1()
{
#ifdef CMmpDecoderVpuEx1_DUMP_RAW_STREAM
    if(m_fp_dump_raw_stream) {
        fclose(m_fp_dump_raw_stream);
        ALOGI("[VPU] Close Dump RawStream File: %s ", m_filename_dump_raw_stream);
    }
#endif
}

MMP_RESULT CMmpDecoderVpuEx1::Open(class mmp_vpu_if *p_vpu_if)
{
    RetCode vpu_ret;
    MMP_RESULT mmpResult = MMP_SUCCESS;
        
    /* alloc dma buffer */
    if(mmpResult == MMP_SUCCESS) {
        
        m_p_stream_buffer = mmp_buffer_mgr::get_instance()->alloc_dma_buffer(STREAM_BUF_SIZE);
        if(m_p_stream_buffer == NULL) {
            MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpDecoderVpuEx1::Open] FAIL :  alloc stream buffer\n")));
            mmpResult = MMP_FAILURE;
        }
        else {

            class mmp_buffer_addr buf_addr;

            buf_addr = this->m_p_stream_buffer->get_buf_addr();
            m_vpu_stream_buffer.base = buf_addr.m_vir_addr;
            m_vpu_stream_buffer.phys_addr = buf_addr.m_phy_addr;
            m_vpu_stream_buffer.size = buf_addr.m_size;
            m_vpu_stream_buffer.virt_addr = buf_addr.m_vir_addr;

            p_vpu_if->vdi_register_dma_memory(m_codec_idx, &m_vpu_stream_buffer);
        }
    }

    /* Decoder Open */
    if(mmpResult == MMP_SUCCESS) {

        switch(m_fourcc_in) {
    
            /* Video */
            case MMP_FOURCC_VIDEO_H263: this->make_decOP_H263(); break;
            case MMP_FOURCC_VIDEO_H264: this->make_decOP_H264(); break;
            
            case MMP_FOURCC_VIDEO_MPEG4: this->make_decOP_MPEG4(MPEG4_CLASS_BASE);  break; 
			case MMP_FOURCC_VIDEO_FLV1: this->make_decOP_MPEG4(MPEG4_CLASS_SORENSON_SPARK); break;			
/*
#if (VPU_SUPOORT_DIV5 == 1)
			case MMP_FOURCC_VIDEO_DIV5: this->make_decOP_MPEG4(1); break;
#endif
			case MMP_FOURCC_VIDEO_XVID: this->make_decOP_MPEG4(2); break;			
#if (VPU_SUPOORT_DIVX == 1)
			case MMP_FOURCC_VIDEO_DIVX: this->make_decOP_MPEG4(5); break;			
#endif
*/
            case MMP_FOURCC_VIDEO_MPEG2: this->make_decOP_MPEG2(); break;
            
            case MMP_FOURCC_VIDEO_WMV1:  
            case MMP_FOURCC_VIDEO_WMV2:  
            case MMP_FOURCC_VIDEO_WMV3:  
            case MMP_FOURCC_VIDEO_WVC1:  
                this->make_decOP_VC1(); 
                break;

            case MMP_FOURCC_VIDEO_MSMPEG4V3: 
                this->make_decOP_MSMpeg4V3(); 
                break;
            
            case MMP_FOURCC_VIDEO_RV30: this->make_decOP_RV30(); break;
            case MMP_FOURCC_VIDEO_RV40: this->make_decOP_RV40(); break;

            case MMP_FOURCC_VIDEO_VP80: this->make_decOP_VP80(); break;

#if (VPU_SUPPORT_THEORA == 1)
            case MMP_FOURCC_VIDEO_THEORA: this->make_decOP_Theora(); break;
#endif

            default:  
                mmpResult = MMP_FAILURE;
                MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpDecoderVpuEx1::Open] FAIL : Not Support Format(%c%c%c%c) "), 
                            MMPGETFOURCC(m_fourcc_in, 0),MMPGETFOURCC(m_fourcc_in, 1),
                            MMPGETFOURCC(m_fourcc_in, 2),MMPGETFOURCC(m_fourcc_in, 3)
                          ));
                break;
        }

        if(mmpResult == MMP_SUCCESS) {

            vpu_ret = p_vpu_if->VPU_DecOpen(&m_DecHandle, &m_decOP);
	        if( vpu_ret != RETCODE_SUCCESS ) {
		        MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpDecoderVpuEx1::Open] FAIL :  p_vpu_if->VPU_DecOpen (vpu_ret=%d) \n"), vpu_ret));
                mmpResult = MMP_FAILURE;
	        }
            else {

                m_vpu_instance_index = p_vpu_if->VPU_GetCodecInstanceIndex((void*)m_DecHandle);
               
                if (m_decOP.bitstreamMode == BS_MODE_PIC_END) {
                    p_vpu_if->VPU_DecSetRdPtr(m_DecHandle, m_decOP.bitstreamBuffer, 1);	
                }

                SecAxiUse		secAxiUse = {0,0,0,0,0,0};
                secAxiUse.useBitEnable  = USE_BIT_INTERNAL_BUF;
	            secAxiUse.useIpEnable   = USE_IP_INTERNAL_BUF;
	            secAxiUse.useDbkYEnable = USE_DBKY_INTERNAL_BUF;
	            secAxiUse.useDbkCEnable = USE_DBKC_INTERNAL_BUF;
	            secAxiUse.useBtpEnable  = USE_BTP_INTERNAL_BUF;
	            secAxiUse.useOvlEnable  = USE_OVL_INTERNAL_BUF;
	            p_vpu_if->VPU_DecGiveCommand(m_DecHandle, SET_SEC_AXI, &secAxiUse);


                // MaverickCache configure
                MaverickCacheConfig decCacheConfig;

                //decConfig.frameCacheBypass   = 0;
                //    decConfig.frameCacheBurst    = 0;
                //    decConfig.frameCacheMerge    = 3;
                //    decConfig.frameCacheWayShape = 15;		
	            MaverickCache2Config(
		            &decCacheConfig, 
		            1, //decoder
		            m_decOP.cbcrInterleave, // cb cr interleave
		            0,//decConfig.frameCacheBypass,
		            0,//decConfig.frameCacheBurst,
		            3, //decConfig.frameCacheMerge,
                    m_mapType,
		            15 //decConfig.frameCacheWayShape
                    );
	            p_vpu_if->VPU_DecGiveCommand(m_DecHandle, SET_CACHE_CONFIG, &decCacheConfig);
            }
        }
	}

    MMPDEBUGMSG(MMPZONE_MONITOR, (TEXT("[CMmpDecoderVpuEx1::Open] res=%d  CodecInstance=%d "), mmpResult, m_vpu_instance_index));
    
    return mmpResult;
}


MMP_RESULT CMmpDecoderVpuEx1::Close(class mmp_vpu_if *p_vpu_if)
{
    if(m_DecHandle != NULL) {

       p_vpu_if->VPU_DecUpdateBitstreamBuffer(m_DecHandle, STREAM_END_SIZE);
	   p_vpu_if->VPU_DecClose(m_DecHandle);
       m_DecHandle = NULL;

       if(m_p_stream_buffer != NULL) {

           p_vpu_if->vdi_unregister_dma_memory(m_codec_idx, &m_vpu_stream_buffer);
           mmp_buffer_mgr::get_instance()->free_buffer(m_p_stream_buffer);
           m_p_stream_buffer = NULL;
       }
    }
  
    this->DeinitVpuFrameBuffer();

    if(m_copyed_aubuf != NULL) {
        delete [] m_copyed_aubuf;
        m_copyed_aubuf = NULL;
    }
        
    MMPDEBUGMSG(MMPZONE_MONITOR, (TEXT("[CMmpDecoderVpuEx1::Close] CodecInstance=%d "), m_vpu_instance_index));

    return MMP_SUCCESS;
}
   

MMP_RESULT CMmpDecoderVpuEx1::DecodeDSI(class mmp_vpu_if *p_vpu_if, class mmp_buffer_videostream* p_buf_vs) {

    MMP_U8* p_stream;
    MMP_S32 stream_size;

    MMP_RESULT mmpResult = MMP_SUCCESS;
    MMP_S32 size;
    RetCode vpu_ret;

    MMP_U8* extra_data;
    MMP_S32 extra_data_size;

    
    p_stream = (MMP_U8*)p_buf_vs->get_stream_real_ptr();
    stream_size = p_buf_vs->get_stream_real_size();

#ifdef CMmpDecoderVpuEx1_DUMP_RAW_STREAM
    if(m_fp_dump_raw_stream) {
        fwrite(p_stream, 1, stream_size, m_fp_dump_raw_stream);
    }
#endif

    MMPDEBUGMSG(1, (TEXT("[CMmpDecoderVpuEx1::DecodeDSI] ln=%d  %c%c%c%c sz=%d CodIdx=%d (%02x %02x %02x %02x, %02x %02x %02x %02x, %02x %02x %02x %02x, %02x %02x %02x %02x ) "), 
                  __LINE__, 
                  MMPGETFOURCCARG(m_fourcc_in),
                  stream_size,
                  m_vpu_instance_index,
                   p_stream[0], p_stream[1], p_stream[2], p_stream[3], 
                   p_stream[4], p_stream[5], p_stream[6], p_stream[7], 
                   p_stream[8], p_stream[9], p_stream[10], p_stream[11], 
                   p_stream[12], p_stream[13], p_stream[14], p_stream[15] 
          ));

    /*
        check if VC1, WMV3
    */
    switch(m_fourcc_in) {
        case MMP_FOURCC_VIDEO_WMV3:
        case MMP_FOURCC_VIDEO_WVC1:

            extra_data = (MMP_U8*)p_buf_vs->get_stream_real_ptr();
            extra_data_size = p_buf_vs->get_stream_real_size();

            m_fourcc_in = CMmpWMVParser::get_fourcc_with_extradata(extra_data, extra_data_size);
            if( (m_fourcc_in == MMP_FOURCC_VIDEO_WMV3)
              || (m_fourcc_in == MMP_FOURCC_VIDEO_WVC1) ) {
                /* Nothing to do */
            }
            else {
                mmpResult = MMP_ERROR_NOT_SUPPORT;
                MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpDecoderVpuEx1::DecodeDSI] FAIL: check WMV3, VC1 with extradata(Not Support)")));
            }
            
            break;
    }
    
    if(mmpResult == MMP_SUCCESS) {
        switch(m_fourcc_in) {

            case MMP_FOURCC_VIDEO_H264:
                mmpResult = this->make_seqheader_H264(p_buf_vs);
                break;

            case MMP_FOURCC_VIDEO_WVC1:
                mmpResult = this->make_seqheader_Common(p_buf_vs); /* Ref vpuhelper.c  ln2295 */
                break;

            case MMP_FOURCC_VIDEO_WMV3:
                mmpResult = this->make_seqheader_WMV3(p_buf_vs);
                break;

            case MMP_FOURCC_VIDEO_MSMPEG4V3: 
                mmpResult = this->make_seqheader_DIV3(p_buf_vs);
                break;
            
            case MMP_FOURCC_VIDEO_RV30:
            case MMP_FOURCC_VIDEO_RV40:
                mmpResult = this->make_seqheader_RV(p_buf_vs);
                break;

            case MMP_FOURCC_VIDEO_VP80:
                mmpResult = this->make_seqheader_VP8(p_buf_vs);
                break;

    #if (VPU_SUPPORT_THEORA == 1)
            case MMP_FOURCC_VIDEO_THEORA:
                mmpResult = this->make_seqheader_Theora(p_buf_vs);
                break;
    #endif

            default:
                mmpResult = this->make_seqheader_Common(p_buf_vs);
        }
    }
    
    /* Input DSI Stream */
    if(mmpResult == MMP_SUCCESS) {
        size = p_vpu_if->WriteBsBufFromBufHelper(m_codec_idx,  m_DecHandle, 
                                                  &m_vpu_stream_buffer, 
                                                  (MMP_U8*)p_buf_vs->get_dsi_buffer(), p_buf_vs->get_dsi_size(), 
                                                  m_decOP.streamEndian);
	    if (size <0) {
            MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpDecoderVpuEx1::DecodeDSI] FAIL: WriteBsBufFromBufHelper ")));
		    mmpResult = MMP_FAILURE;
	    }
    }
    else {
        MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpDecoderVpuEx1::DecodeDSI] FAIL: make seqheader ")));
    }

    
    /* RUN Seq Init */
    if(mmpResult == MMP_SUCCESS) {

        if(m_decOP.bitstreamMode == BS_MODE_PIC_END)
	    {
		    vpu_ret = p_vpu_if->VPU_DecGetInitialInfo(m_DecHandle, &m_dec_init_info);
		    if(vpu_ret != RETCODE_SUCCESS) {
                MMPDEBUGMSG(1, (TEXT("[CMmpDecoderVpuEx1::DecodeDSI] FAIL: p_vpu_if->VPU_DecGetInitialInfo ln=%d  vpu_ret=%d %c%c%c%c "), __LINE__, vpu_ret, MMPGETFOURCCARG(m_fourcc_in) ));
                mmpResult = MMP_FAILURE;
		    }
            else {
                m_is_complete_decode_dsi = MMP_TRUE;
            }
            p_vpu_if->VPU_ClearInterrupt(m_codec_idx);
	    }
    }


    if(mmpResult == MMP_SUCCESS) {
        m_vpu_buf_width = mmp_buffer_videoframe::s_get_stride(m_fourcc_out, m_dec_init_info.picWidth, MMP_BUFTYPE_HW);
        m_vpu_buf_height= mmp_buffer_videoframe::s_get_height(m_fourcc_out, m_dec_init_info.picHeight, MMP_BUFTYPE_HW);
    }

    MMPDEBUGMSG(1, (TEXT("[CMmpDecoderVpuEx1::DecodeDSI] ln=%d  res=0x%x m_regFrameBufCount=%d m_dec_init_info.minFrameBufferCount=%d Pic(w=%d h=%d) Buf(w=%d h=%d) "), 
                              __LINE__, mmpResult, 
                              m_regFrameBufCount,
                              m_dec_init_info.minFrameBufferCount,
                              m_dec_init_info.picWidth, m_dec_init_info.picHeight,
                              m_vpu_buf_width, m_vpu_buf_height
                              ));
    
    return mmpResult;
}

MMP_RESULT CMmpDecoderVpuEx1::InitVpuFrameBuffer(class mmp_vpu_if *p_vpu_if) {

    MMP_RESULT mmpResult = MMP_SUCCESS;
    RetCode vpu_ret;
    class mmp_buffer_videoframe* p_buf_vf;
    FrameBuffer* user_frame;//[MAX_FRAMEBUFFER_COUNT];//*pUserFrame = NULL;
    MMP_S32 i;
    MMP_S32 real_bufcnt, min_bufcnt;
    MMP_S32 vpu_pic_width, vpu_pic_height; /* value extracted by VPU */
    MMP_S32 framebufStride, framebufHeight;
    enum MMP_FOURCC fourcc;

    user_frame = new FrameBuffer[MAX_FRAMEBUFFER_COUNT];
    
    real_bufcnt = this->GetVpuRealFrameBufferCount();
    min_bufcnt = this->GetVpuMinFrameBufferCount();
    vpu_pic_width = this->GetVpuPicWidth();
    vpu_pic_height = this->GetVpuPicHeight();
        
    if( (min_bufcnt >= 1) 
        && (real_bufcnt >= 1)  
        && (vpu_pic_width > 16)
        && (vpu_pic_height > 16) )    {

        framebufStride = m_p_buf_vf_arr[0]->get_buf_stride();
        framebufHeight = m_p_buf_vf_arr[0]->get_buf_height();

        for(i = 0; i < real_bufcnt; i++) {
                    
                memset(&user_frame[i], 0x00, sizeof(FrameBuffer));

                p_buf_vf = m_p_buf_vf_arr[i];

                user_frame[i].bufY = p_buf_vf->get_buf_phy_addr_y();
                user_frame[i].bufCb = p_buf_vf->get_buf_phy_addr_u();
                user_frame[i].bufCr = p_buf_vf->get_buf_phy_addr_v();
                user_frame[i].mapType = m_mapType;
                user_frame[i].stride = framebufStride;
                user_frame[i].height = framebufHeight;
                user_frame[i].myIndex = i;

                fourcc = p_buf_vf->get_fourcc();
                    
                MMPDEBUGMSG(1, (TEXT("[CMmpDecoderVpuEx1::InitVpuFrameBuffer] FrameBuf_PhyAddr %d/%d = (0x%08x 0x%08x 0x%08x )  fourcc=%c%c%c%c vf_buf(%d %d) "), 
                           i, real_bufcnt,
                           user_frame[i].bufY, user_frame[i].bufCb, user_frame[i].bufCr,
                           MMPGETFOURCCARG(fourcc),
                           p_buf_vf->get_buf_stride(), p_buf_vf->get_buf_height()
                    ));
        }

        vpu_ret = p_vpu_if->VPU_DecRegisterFrameBuffer(m_DecHandle, user_frame, m_regFrameBufCount, framebufStride, framebufHeight, m_mapType);

        if(vpu_ret != RETCODE_SUCCESS)  {
            MMPDEBUGMSG(1, (TEXT("[CMmpDecoderVpuEx1::InitVpuFrameBuffer] FAIL: p_vpu_if->VPU_DecRegisterFrameBuffer vpu_ret=%d reg_cnt=%d stride=%d height=%d"), 
                vpu_ret,
                m_regFrameBufCount,
                framebufStride,
                framebufHeight
                ));
            mmpResult = MMP_FAILURE;
        }
            

    } /* end of if(m_dec_init_info.minFrameBufferCount >= 1) */
    else {
        MMPDEBUGMSG(1, (TEXT("[CMmpDecoderVpuEx1::InitVpuFrameBuffer] ln=%d FAIL: real_bufcnt=%d min_bufcnt=%d vpu_w=%d  vpu_h=%d "), 
                            __LINE__,
                            real_bufcnt,
                            min_bufcnt,
                            vpu_pic_width, vpu_pic_height
                            ));
        mmpResult = MMP_FAILURE;
    }
    
    delete [] user_frame;

    return mmpResult;
}

MMP_RESULT CMmpDecoderVpuEx1::DeinitVpuFrameBuffer() {

    MMP_S32 i;

    if(m_is_alloc_vf == MMP_TRUE) {
        for(i = 0; i < MAX_FRAMEBUFFER_COUNT; i++) {
            if(m_p_buf_vf_arr[i] != NULL)   {
                mmp_buffer_mgr::get_instance()->free_media_buffer(m_p_buf_vf_arr[i]);
            }
        }
    }

    return MMP_SUCCESS;
}

MMP_RESULT CMmpDecoderVpuEx1::AllocVpuFrameBuffer(class mmp_vpu_if *p_vpu_if) {

    MMP_RESULT mmpResult = MMP_FAILURE;
    MMP_S32 i, pic_width, pic_height;
    MMP_S32 framebufStride, framebufHeight;
    MMP_S32 min_bufcnt, real_bufcnt;

    this->DeinitVpuFrameBuffer();

    min_bufcnt = this->GetVpuMinFrameBufferCount();
    real_bufcnt = min_bufcnt;

    pic_width = this->GetVpuPicWidth();
    pic_height = this->GetVpuPicHeight();
    framebufStride = MMP_BYTE_ALIGN(pic_width, 16);
    framebufHeight = MMP_BYTE_ALIGN(pic_height, 16);

    for(i = 0; i < real_bufcnt; i++) {
        m_is_alloc_vf = MMP_TRUE;
        
        /*
            2016,9,29   CTS-meida/android.media.cts.DecoderTest/testEOSBehaviorH264 
                        contents�� �� �� Decoding�ϰ� �� ���� Frame�񱳵� �Ѵ�.
                        SW���۸� �����ϴµ�  ION_CACHE���带 �����ϸ�  SW Dest���ۿ� copy�� ��  ��ġ�� ���� �߻��ϰ� CTS FAIL �� �߻��Ѵ�.
                        ������ CACHE������ �޸𸮸� �Ҵ��ؼ��� �ȵȴ�.
                        �ӵ��� �ſ� ������ ������ �����Ǵµ�.. ���� �ٽ� Check�� �ʿ��ϴ�.
        */           
        //m_p_buf_vf_arr[i] = mmp_buffer_mgr::get_instance()->alloc_media_videoframe((MMP_MEDIA_ID)this, framebufStride, framebufHeight, MMP_FOURCC_IMAGE_YUV420, mmp_buffer::ION, mmp_buffer::FLAG_CACHED);
        m_p_buf_vf_arr[i] = mmp_buffer_mgr::get_instance()->alloc_media_videoframe((MMP_MEDIA_ID)this, framebufStride, framebufHeight, MMP_FOURCC_IMAGE_YUV420, mmp_buffer::ION, mmp_buffer::FLAG_NONE);
        if(m_p_buf_vf_arr[i] == NULL) {
            break;
        }
    }
    if(i == real_bufcnt) {
        this->SetVpuRealFrameBufferCount(real_bufcnt); 
        mmpResult = this->InitVpuFrameBuffer(p_vpu_if);
    }

    return mmpResult;
}

MMP_RESULT CMmpDecoderVpuEx1::SetVpuFrameBuffer(class mmp_vpu_if *p_vpu_if, class mmp_buffer_videoframe** p_buf_vf_arr, MMP_S32 bufcnt) {

    MMP_RESULT mmpResult = MMP_FAILURE;
    MMP_S32 i, j, pic_width, pic_height;
    MMP_S32 framebufStride, framebufHeight;
    MMP_S32 min_bufcnt, real_bufcnt;

    this->DeinitVpuFrameBuffer();

    real_bufcnt = bufcnt;
    min_bufcnt = this->GetVpuMinFrameBufferCount();

    if(real_bufcnt >= min_bufcnt) {

        pic_width = this->GetVpuPicWidth();
        pic_height = this->GetVpuPicHeight();
        framebufStride = MMP_BYTE_ALIGN(pic_width, 16);
        framebufHeight = MMP_BYTE_ALIGN(pic_height, 16);

        
        m_is_alloc_vf = MMP_FALSE;
        for(i = 0; i < real_bufcnt; i++) {
            m_p_buf_vf_arr[i] = p_buf_vf_arr[i];
            m_buf_used_count[i] = 1;
            
            for(j = 0; j < MAX_FRAMEBUFFER_COUNT; j++) {
                if(m_p_buf_vf_req_clear_before_dsi[j] == p_buf_vf_arr[i]) {
                    m_buf_used_count[i] = 0;
                    break;
                }
            }
        }

        for(i = 0; i < real_bufcnt; i++) {
            if(m_buf_used_count[i] == 1) {
                p_vpu_if->VPU_DecGiveCommand(m_DecHandle, DEC_SET_DISPLAY_FLAG, &i);
            }
        }
            

        this->SetVpuRealFrameBufferCount(real_bufcnt); 
        mmpResult = this->InitVpuFrameBuffer(p_vpu_if);

        
    }

    return mmpResult;
}

MMP_RESULT CMmpDecoderVpuEx1::flush_buffer_in() {

    return MMP_SUCCESS;
}
    
MMP_RESULT CMmpDecoderVpuEx1::flush_buffer_out(class mmp_vpu_if *p_vpu_if) {

    int buf_idx, buf_cnt;
    
    /*
        VPU_DecFrameBufferFlush�� �����ϸ� ramdom access ���� ������ ��������.
    */
    p_vpu_if->VPU_DecFrameBufferFlush(m_DecHandle);

    buf_cnt = GetVpuRealFrameBufferCount();
    for(buf_idx = 0; buf_idx < buf_cnt; buf_idx++) {
        p_vpu_if->VPU_DecClrDispFlag(m_DecHandle, buf_idx);
        m_buf_used_count[buf_idx] = 0;
    }
    
    /* 
        add by hthwang 2017,1,16
        �� FLAG�� clear���� ������, MXPlayer, OneLoop-Play ���� ������ �߻��Ѵ�. 
    */
    p_vpu_if->VPU_DecGiveCommand(m_DecHandle, CLEAR_STREAM_END_FLAG, NULL); 
    
    return MMP_SUCCESS;
}

MMP_RESULT CMmpDecoderVpuEx1::ClearDisplayFlag(class mmp_vpu_if *p_vpu_if, class mmp_buffer_videoframe* p_buf_vf) {

    int i, buf_cnt, buf_idx;

    if(m_is_complete_decode_dsi == MMP_FALSE) {
        for(i = 0; i < MAX_FRAMEBUFFER_COUNT; i++) {
            if(m_p_buf_vf_req_clear_before_dsi[i] == NULL) {
                m_p_buf_vf_req_clear_before_dsi[i] = p_buf_vf;
                break;
            }
        }
    }

    if(m_is_alloc_vf == MMP_FALSE) {
    
        buf_cnt = GetVpuRealFrameBufferCount();
        for(i = 0; i < buf_cnt; i++) {
            if(m_p_buf_vf_arr[i] == p_buf_vf) {

                buf_idx = i;  
                
                if(m_buf_used_count[buf_idx] > 0) {
                    p_vpu_if->VPU_DecClrDispFlag(m_DecHandle, buf_idx);
                    m_buf_used_count[buf_idx]--;
                }
                
           if (m_DEBUG_VPUDec == 1) {
                char msg[256], str[32];
                msg[0] = '\0';
                for(i = 0; i < buf_cnt; i++) {
                    sprintf(str, "%d ", m_buf_used_count[i]);
                    strcat(msg, str);
                }
                
                MMPDEBUGMSG(m_DEBUG_VPUDec, (TEXT("[CMmpDecoderVpuEx1::ClearDisplayFlag] ln=%d  index=%d/%d (%s) "), __LINE__, buf_idx, buf_cnt , msg));
            }
                
                break;
            }
        }

    }

    return MMP_SUCCESS;
}

MMP_RESULT CMmpDecoderVpuEx1::Play_Function_Tool(MMP_PLAY_FORMAT playformat, MMP_S64 curpos, MMP_S64 totalpos){
	MMP_RESULT mmpResult = MMP_SUCCESS;

	if(playformat == MMP_PLAY_BACK)
	{
		m_skipframeMode = 0;
		m_randomaccess = MMP_PLAY_BACK;
	}
	else if(playformat == MMP_PLAY_FF)
	{
		m_randomaccess = MMP_PLAY_FF; // flush frame / reordering off frames
		MMPDEBUGMSG(1, (TEXT("------------------- play MMP_PLAY_FF        ------------------ "))); 
	}
	else if(playformat == MMP_PLAY_REW)
	{	
		m_randomaccess = MMP_PLAY_REW; // flush frame / reordering off frames
		MMPDEBUGMSG(1, (TEXT("------------------- play MMP_PLAY_REW       ------------------ "))); 	
	}
	else if(playformat == MMP_PLAY_RAND)
	{
		m_randomaccess = MMP_PLAY_RAND; // flush frame 
		MMPDEBUGMSG(1, (TEXT("------------------- play MMP_PLAY_RAND       ------------------ "))); 	
	}

	return mmpResult;
}



MMP_RESULT CMmpDecoderVpuEx1::DecodeAu_PicEnd_Run(class mmp_vpu_if *p_vpu_if, class mmp_buffer_videostream* p_buf_vs) {

    MMP_RESULT mmpResult = MMP_SUCCESS;
    RetCode vpu_ret;
    DecParam		decParam;
    MMP_S32 size;
    MMP_U32 start_tick, t1, t2;
    MMP_U8* p_stream;
    MMP_S32 stream_size;
    MMP_U32 vs_flag = (MMP_U32)p_buf_vs->get_flag();
    MMP_S32 copyed_ausize = 0;
    
    start_tick = CMmpUtil::GetTickCount();

    memset(&decParam, 0x00, sizeof(decParam));
    decParam.iframeSearchEnable = m_iframeSearchEnable; /* I Frame Search */
    decParam.skipframeMode = m_skipframeMode; /* PB Frame Skip */
    decParam.DecStdParam.mp2PicFlush = 0;
    m_input_stream_size = 0;

    MMPDEBUGMSG(m_DEBUG_VPUDec, (TEXT("\n------------ VPU Dec Start ---------------------------- CodIdx=%d  %c%c%c%c %dx%d"), m_vpu_instance_index, MMPGETFOURCCARG(m_fourcc_in), m_vpu_buf_width, m_vpu_buf_height));

    if( (vs_flag&(MMP_U32)mmp_buffer_media::FLAG_VIDEO_CHUNK_REUSE) != 0) {
        MMPDEBUGMSG(m_DEBUG_VPUDec, (TEXT("[CMmpDecoderVpuEx1::DecodeAu_PicEnd_Run] ln=%d Chunk Reuse "),   __LINE__  ));
    }
    else {
        p_stream = (MMP_U8*)p_buf_vs->get_stream_real_ptr();
        stream_size = p_buf_vs->get_stream_real_size();
         
#ifdef CMmpDecoderVpuEx1_DUMP_RAW_STREAM
        if(m_fp_dump_raw_stream) {
            fwrite(p_stream, 1, stream_size, m_fp_dump_raw_stream);
        }
#endif

        if(stream_size >= 16) {
            if(m_fourcc_in == MMP_FOURCC_VIDEO_H264) {

                unsigned char checksum = 0x00;
                for (int ic = 0; ic < stream_size; ic++) {
                    unsigned char cc = (unsigned char)p_stream[ic];
                    checksum += cc;
                }

                MMPDEBUGMSG(m_DEBUG_VPUDec, (TEXT("[CMmpDecoderVpuEx1::DecodeAu_PicEnd_Run] ln=%d chksum=0x%x cnt=%d sz=%d CodIdx=%d is_avc(%d) nalsz(%d) pts=%lld (%02x %02x %02x %02x, %02x %02x %02x %02x, %02x %02x %02x %02x, %02x %02x %02x %02x ) "), 
                                __LINE__, checksum, m_input_stream_count, stream_size,
                                m_vpu_instance_index,
                                m_is_avc, m_nal_length_size,
                                p_buf_vs->get_pts(), 
                                p_stream[0], p_stream[1], p_stream[2], p_stream[3], 
                                p_stream[4], p_stream[5], p_stream[6], p_stream[7], 
                                p_stream[8], p_stream[9], p_stream[10], p_stream[11], 
                                p_stream[12], p_stream[13], p_stream[14], p_stream[15] ));
            }
            else {

                unsigned char checksum = 0x00;
                for (int ic = 0; ic < stream_size; ic++) {
                    unsigned char cc = (unsigned char)p_stream[ic];
                    checksum += cc;
                }

                MMPDEBUGMSG(m_DEBUG_VPUDec, (TEXT("[CMmpDecoderVpuEx1::DecodeAu_PicEnd_Run] ln=%d chksum=0x%x cnt=%d sz=%d CodIdx=%d pts=%lld (%02x %02x %02x %02x, %02x %02x %02x %02x, %02x %02x %02x %02x, %02x %02x %02x %02x ) "), 
                                __LINE__, checksum, m_input_stream_count, stream_size,
                                m_vpu_instance_index,
                                p_buf_vs->get_pts(), 
                                p_stream[0], p_stream[1], p_stream[2], p_stream[3], 
                                p_stream[4], p_stream[5], p_stream[6], p_stream[7], 
                                p_stream[8], p_stream[9], p_stream[10], p_stream[11], 
                                p_stream[12], p_stream[13], p_stream[14], p_stream[15] ));
            }
        }
        else {
            MMPDEBUGMSG(m_DEBUG_VPUDec, (TEXT("[CMmpDecoderVpuEx1::DecodeAu_PicEnd_Run] ln=%d cnt=%d sz=%d CodIdx=%d pts=%lld  "), 
                            __LINE__, m_input_stream_count, stream_size,
                            m_vpu_instance_index,
                            p_buf_vs->get_pts() ));
        
        }
        

        /* check header */
        switch(m_fourcc_in) {

            case MMP_FOURCC_VIDEO_H264: 
                mmpResult = this->make_frameheader_H264(p_buf_vs, &copyed_ausize);
                break;

            case MMP_FOURCC_VIDEO_WVC1:
                mmpResult = this->make_frameheader_WVC1(p_buf_vs);
                break;

            case MMP_FOURCC_VIDEO_WMV3:
                mmpResult = this->make_frameheader_WMV3(p_buf_vs);
                break;

            case MMP_FOURCC_VIDEO_MSMPEG4V3:
                mmpResult = this->make_frameheader_DIV3(p_buf_vs);
                break;

            case MMP_FOURCC_VIDEO_RV30:
            case MMP_FOURCC_VIDEO_RV40:
                mmpResult = this->make_frameheader_RV(p_buf_vs);
                break;

            case MMP_FOURCC_VIDEO_VP80:
                mmpResult = this->make_frameheader_VP8(p_buf_vs);
                break;

    #if (VPU_SUPPORT_THEORA == 1)
            case MMP_FOURCC_VIDEO_THEORA:
                mmpResult = this->make_frameheader_Theora(p_buf_vs);
                break;
    #endif

            default:
                mmpResult = this->make_frameheader_Common(p_buf_vs);
        }
    
        if(m_input_stream_count) // AVC1 format sequence after pps data reuse
            p_vpu_if->VPU_DecSetRdPtr(m_DecHandle, m_decOP.bitstreamBuffer, 1);	
			    
        if(mmpResult == MMP_SUCCESS) {
    
            if(p_buf_vs->get_header_size() > 0) {
                size = p_vpu_if->WriteBsBufFromBufHelper(m_codec_idx, m_DecHandle, 
                                                            &m_vpu_stream_buffer, 
                                                            (BYTE*)p_buf_vs->get_header_buffer(), p_buf_vs->get_header_size(), 
                                                            m_decOP.streamEndian);
                if (size <0)
    	        {
    		        mmpResult = MMP_FAILURE;
                    MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpDecoderVpuEx1::DecodeAu_PicEnd_Run] FAIL: WriteBsBufFromBufHelper")));
    	        }
                else {
                    m_input_stream_size += size;
                }
            }
			
            if(m_fourcc_in != MMP_FOURCC_VIDEO_THEORA)
            {
	            if(mmpResult == MMP_SUCCESS) {

                    MMP_U8* p_au;
                    MMP_S32 au_size, aligned_au_size;
                    
                    if(copyed_ausize > 0) {
                        p_au = m_copyed_aubuf;
                        au_size = copyed_ausize;
                    }
                    else {
                        p_au = p_buf_vs->get_stream_real_ptr();
                        au_size = p_buf_vs->get_stream_real_size();
                    }
                   /*
                       2017,01,16 comment by hthwang 
                                  Write�ҋ� ���� align�� �����ʿ�����. (������ ���� Test�Ϸ� )
                                  web-browser���� KBS News 480p ������  �� �ڵ����� ���� ������ �߻��Ѵ�.
                    */
                     aligned_au_size = MMP_BYTE_ALIGN(au_size, 512);
                     if( (aligned_au_size-au_size) > 0) {
                        memset(&p_au[au_size], 0x00, aligned_au_size-au_size);
                     }
                     //au_size = aligned_au_size;
                    
                    
                
                    
                    size = p_vpu_if->WriteBsBufFromBufHelper(m_codec_idx, m_DecHandle, 
	                                                            &m_vpu_stream_buffer, 
	                                                            p_au, au_size, 
	                                                            m_decOP.streamEndian);

        
	                if (size <0)
	    	        {
	    		        mmpResult = MMP_FAILURE;
	                    MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpDecoderVpuEx1::DecodeAu_PicEnd_Run] FAIL: WriteBsBufFromBufHelper")));
	    	        }
                    else {
                        m_input_stream_size += size;

                        unsigned char checksum = 0x00;
                        unsigned char *vpu_buf = (unsigned char*)m_vpu_stream_buffer.virt_addr;
                        for (int ic = 0; ic < au_size; ic++) {
                            unsigned char cc = (unsigned char)vpu_buf[ic];
                            checksum += cc;
                        }
                        
                            MMPDEBUGMSG(m_DEBUG_VPUDec, (TEXT("[CMmpDecoderVpuEx1::DecodeAu_PicEnd_Run]11  ln=%d chksum=0x%x cnt=%d sz=%d CodIdx=%d is_avc(%d) nalsz(%d) (%02x %02x %02x %02x, %02x %02x %02x %02x, %02x %02x %02x %02x, %02x %02x %02x %02x ) "),
                                __LINE__, checksum, m_input_stream_count, au_size,
                                m_vpu_instance_index,
                                m_is_avc, m_nal_length_size,
                                vpu_buf[0], vpu_buf[1], vpu_buf[2], vpu_buf[3],
                                vpu_buf[4], vpu_buf[5], vpu_buf[6], vpu_buf[7],
                                vpu_buf[8], vpu_buf[9], vpu_buf[10], vpu_buf[11],
                                vpu_buf[12], vpu_buf[13], vpu_buf[14], vpu_buf[15]));

                    }
                }
            }
			
        }
        else {
            MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpDecoderVpuEx1::DecodeAu_PicEnd_Run] FAIL: make frame header")));
        }
        
    } /* end of  if( (vs_flag&(MMP_U32)mmp_buffer_media::FLAG_VIDEO_CHUNK_REUSE) == 0)  */
	
    t1 = CMmpUtil::GetTickCount();
    #if 1
	{
		PhysicalAddress rdPtr, wrPtr;
		p_vpu_if->VPU_DecGetBitstreamBuffer(m_DecHandle, &rdPtr, &wrPtr, (int*)&m_room1);
		MMPDEBUGMSG(m_DEBUG_VPUDec, (TEXT("Pre Read ptr = 0x%08x | Write ptr = 0x%08x  room1=%d "), rdPtr, wrPtr, m_room1));	
	}	
	#endif

	
    if(mmpResult == MMP_SUCCESS)  {

        if( (m_last_int_reason&(1<<INT_BIT_DEC_FIELD)) != 0) {
           p_vpu_if->VPU_ClearInterrupt(m_codec_idx);
           MMPDEBUGMSG(0, (TEXT("[second] INT_BIT_DEC_FIELD")));
        }
        else {

	        // Start decoding a frame.
START_DEC:            
	        vpu_ret = p_vpu_if->VPU_DecStartOneFrame(m_DecHandle, &decParam);
		    MMPDEBUGMSG(0, (TEXT("[first] VPU_DecStartOneFrame start")));
			if(vpu_ret == RETCODE_FRAME_NOT_COMPLETE)
			{
				 //MMPDEBUGMSG(0, (TEXT("[CMmpDecoderVpuEx1::DecodeAu_PicEnd] wait instance")));
				 //printf(".");
				 //CMmpUtil::Sleep(1000);
				 goto START_DEC;
			}			
	        if (vpu_ret != RETCODE_SUCCESS) 
	        {
		        mmpResult = MMP_FAILURE;
                MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpDecoderVpuEx1::DecodeAu_PicEnd_Run] ln=%d FAIL: p_vpu_if->VPU_DecStartOneFrame vpu_ret=%d "), __LINE__, vpu_ret));
	        }
        }
    }

    t2 = CMmpUtil::GetTickCount();
    
    return mmpResult;
}


MMP_RESULT CMmpDecoderVpuEx1::DecodeAu_PicEnd_WaitDone(class mmp_vpu_if *p_vpu_if, class mmp_buffer_videostream* p_buf_vs, class mmp_buffer_videoframe** pp_buf_vf, MMP_U32 *vpu_result) {

    MMP_RESULT mmpResult = MMP_SUCCESS;
    RetCode vpu_ret;
    MMP_S32 int_reason;
    MMP_U32  t2, t3 ,t4;
        
    *vpu_result = 0;

    t2 = CMmpUtil::GetTickCount();
        
    int_reason = 0;
    while(mmpResult == MMP_SUCCESS) {
    
        int_reason = p_vpu_if->VPU_WaitInterrupt(m_codec_idx, VPU_DEC_TIMEOUT);
        if (int_reason == -1) // timeout
		{
            MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpDecoderVpuEx1::DecodeAu_PicEnd_WaitDone] ln=%d  FAIL: p_vpu_if->VPU_WaitInterrupt   TimeOut "), __LINE__));
			VPU_SWReset(m_codec_idx, SW_RESET_SAFETY, m_DecHandle);				
			mmpResult = MMP_FAILURE;
            int_reason = 0;
            *vpu_result = VPU_TIMEOUT;
            break;
		}		
        
        if (int_reason & (1<<INT_BIT_DEC_FIELD)) {

            PhysicalAddress rdPtr, wrPtr;
            MMP_S32 room, seqHeaderSize;
            MMP_S32 remain_size;
            
            seqHeaderSize = 0;

            p_vpu_if->VPU_DecGetBitstreamBuffer(m_DecHandle, &rdPtr, &wrPtr, (int*)&room);

            if(rdPtr <= wrPtr)  {
                remain_size = wrPtr - rdPtr;
            }
            else {
                remain_size = wrPtr-m_decOP.bitstreamBuffer;
                remain_size += m_decOP.bitstreamBufferSize - (rdPtr-m_decOP.bitstreamBuffer);
            }
            
            if(remain_size > 8) //(rdPtr-m_decOP.bitstreamBuffer) < (PhysicalAddress)(chunkSize+picHeaderSize+seqHeaderSize-8))	// there is full frame data in chunk data.
            {
				//p_vpu_if->VPU_DecSetRdPtr(m_DecHandle, rdPtr, 0);		//set rdPtr to the position of next field data.
                MMPDEBUGMSG(m_DEBUG_VPUDec, (TEXT("[CMmpDecoderVpuEx1::DecodeAu_PicEnd_WaitDone] INT_BIT_DEC_FIELD 0x%08x Wait Next Dec (0x%08x 0x%08x 0x%08x ) remain_size:%d"), int_reason , rdPtr, wrPtr , m_decOP.bitstreamBuffer, remain_size));
            }
            else {
                MMPDEBUGMSG(m_DEBUG_VPUDec, (TEXT("[CMmpDecoderVpuEx1::DecodeAu_PicEnd_WaitDone] INT_BIT_DEC_FIELD 0x%08x Wait Next Stream (0x%08x 0x%08x 0x%08x ) remain_size:%d"), int_reason , rdPtr, wrPtr , m_decOP.bitstreamBuffer, remain_size));
                mmpResult = MMP_FAILURE;
                break;
            }
            
        }

        if (int_reason) {
    		p_vpu_if->VPU_ClearInterrupt(m_codec_idx);
        }
    
        if (int_reason & (1<<INT_BIT_PIC_RUN))  {
            MMPDEBUGMSG(m_DEBUG_VPUDec, (TEXT("[CMmpDecoderVpuEx1::DecodeAu_PicEnd_WaitDone] INT_BIT_PIC_RUN 0x%08x "), int_reason ));
		    break;		
        }
    }

    #if 1
	{
		PhysicalAddress rdPtr, wrPtr;
		p_vpu_if->VPU_DecGetBitstreamBuffer(m_DecHandle, &rdPtr, &wrPtr, (int*)&m_room2);
		MMPDEBUGMSG(m_DEBUG_VPUDec, (TEXT("Pre Read ptr = 0x%08x | Write ptr = 0x%08x  room2=%d "), rdPtr, wrPtr, m_room2));	
	}	
	#endif
    	
    m_last_int_reason = int_reason;

	
    t3 = CMmpUtil::GetTickCount();
    
    if(mmpResult == MMP_SUCCESS) {

        memset(&m_output_info, 0x00, sizeof(m_output_info));
        vpu_ret = p_vpu_if->VPU_DecGetOutputInfo(m_DecHandle, &m_output_info);
        
        t4 = CMmpUtil::GetTickCount();

        if(m_DEBUG_VPUDec==1) {
            MMP_U32 end_flag = 0xFFFFFFFF;
            p_vpu_if->VPU_DecGiveCommand(m_DecHandle, GET_STREAM_END_FLAG, &end_flag); 
            MMPDEBUGMSG(m_DEBUG_VPUDec, (TEXT("[CMmpDecoderVpuEx1::DecodeAu_PicEnd_WaitDone] ln=%d indexFrame(Disp:%d(%d) Dec:%d) interlace(%d) pict(%d %d) success(%d) topFieldFirst(%d) m_iframeSearchEnable=%d end_flag=0x%08x "), 
                                                                __LINE__, 
                                                                m_output_info.indexFrameDisplay, m_last_display_index,
                                                                m_output_info.indexFrameDecoded,
                                                                m_output_info.interlacedFrame, 
                                                                m_output_info.picType, 
                                                                m_output_info.picTypeFirst, 
                                                                m_output_info.decodingSuccess,
                                                                m_output_info.topFieldFirst,
                                                            
                                                                m_iframeSearchEnable,
                                                                end_flag
                                                                ));
        }

       MMPDEBUGMSG(m_DEBUG_VPUDec, (TEXT("[CMmpDecoderVpuEx1::DecodeAu_PicEnd_WaitDone] ln=%d sequenceChanged(%d) decRes(%d %d) dispRes(%d %d)"), 
                                                            __LINE__, 
                                                            m_output_info.sequenceChanged,
                                                            m_output_info.decPicWidth,  m_output_info.decPicHeight,
                                                            m_output_info.dispPicWidth,  m_output_info.dispPicHeight
                                                            ));

        //if(m_output_info.sequenceChanged == 1) {
        //    m_dec_init_info.picWidth = m_output_info.decPicWidth;
        //    m_dec_init_info.picHeight = m_output_info.decPicHeight;
        //    m_vpu_buf_width = mmp_buffer_videoframe::s_get_stride(m_fourcc_out, m_dec_init_info.picWidth, MMP_BUFTYPE_HW);
        //    m_vpu_buf_height= mmp_buffer_videoframe::s_get_height(m_fourcc_out, m_dec_init_info.picHeight, MMP_BUFTYPE_HW);
        //}

        //if( (vpu_ret == RETCODE_SUCCESS) && (m_output_info.picType < PIC_TYPE_MAX) ) {
        if(vpu_ret == RETCODE_SUCCESS) {

            if(m_output_info.indexFrameDisplay >= 0) {
            
                FrameBuffer frameBuf;
                p_vpu_if->VPU_DecGetFrameBuffer(m_DecHandle, m_output_info.indexFrameDisplay, &frameBuf);
                
                if( (m_last_display_index == m_output_info.indexFrameDisplay) && (p_buf_vs->get_stream_real_size() == 0) ) {
                      /* EOF Error */
                    MMPDEBUGMSG(m_DEBUG_VPUDec, (TEXT("[CMmpDecoderVpuEx1::DecodeAu_PicEnd_WaitDone] ln=%d EOF Error last disaplay index is not updated!! (%d,%d) "), 
                                   __LINE__, m_last_display_index, m_output_info.indexFrameDisplay));
                }
                else if(m_output_info.numOfErrMBs > 0) {			
                    /*
                       **2016,1,21
                        �ϱ׷��� ������ ���ؼ� �������� �ʴ°� ���ڴ�. 
                        err MB�� �����ϸ� Decoding FAIL �� �����Ѵ�. 
                    */
                    p_vpu_if->VPU_DecClrDispFlag(m_DecHandle, m_output_info.indexFrameDisplay);
                    MMPDEBUGMSG(m_DEBUG_VPUDec, (TEXT("[CMmpDecoderVpuEx1::DecodeAu_PicEnd_WaitDone] ln=%d MBErr, m_output_info.indexFrameDisplay=%d  m_is_alloc_vf=%d m_output_info.numOfErrMBs=%d "), __LINE__, m_output_info.indexFrameDisplay, m_is_alloc_vf, m_output_info.numOfErrMBs));
                }
                else {

                    m_last_display_index = m_output_info.indexFrameDisplay;

                    if(pp_buf_vf != NULL) {
                        *pp_buf_vf = m_p_buf_vf_arr[m_output_info.indexFrameDisplay];
#ifdef CMmpDecoderVpuEx1_CAPTURE_FRAME
                        {
                            unsigned char *Y, *U, *V;
                            unsigned char *RGB;
                            static int is_capture = 0;
                            int pic_width, pic_height;
                            int luma_stride, chroma_stride;
                            
                            pic_width = (*pp_buf_vf)->get_pic_width();
                            pic_height = (*pp_buf_vf)->get_pic_height();
                            luma_stride = (*pp_buf_vf)->get_stride_luma();
                            chroma_stride = (*pp_buf_vf)->get_stride_chroma();

                            MMPDEBUGMSG(m_DEBUG_VPUDec, (TEXT("[CMmpDecoderVpuEx1::DecodeAu_PicEnd_WaitDone] Ccccccc pic_width=%d"), pic_width));

                            if(is_capture ==0 && pic_width==176) {

                                RGB = new unsigned char[ (pic_width+32) * (pic_height+32) * 4];

                                luma_stride = 176;
                                chroma_stride = 176/2;

                                Y = (unsigned char*)(*pp_buf_vf)->get_buf_vir_addr_y();
                                U = (unsigned char*)(*pp_buf_vf)->get_buf_vir_addr_u();
                                V = (unsigned char*)(*pp_buf_vf)->get_buf_vir_addr_v();
                                CMmpImageTool::ConvertYUV420MtoRGB(Y, U, V,
                                                                   luma_stride, chroma_stride, chroma_stride, 
                                                                   pic_width, pic_height, 
                                                                   RGB,
                                                                   MMP_FOURCC_IMAGE_ABGR8888 );

                                CMmpImageTool::Bmp_SaveFile("/data/a.bmp", 
                                                    pic_width, pic_height, pic_width*4, 
                                                    (MMP_U8*)RGB, 
                                                    MMP_FOURCC_IMAGE_ABGR8888 );

                                   
                                delete [] RGB;
                                is_capture = 1;
                            }
                        }
#endif
                    }
                    m_buf_used_count[m_output_info.indexFrameDisplay]++;

                    /* In case of Android-Buffer, clear when return the buffer */
                    MMPDEBUGMSG(m_DEBUG_VPUDec, (TEXT("[CMmpDecoderVpuEx1::DecodeAu_PicEnd_WaitDone] ln=%d m_output_info.indexFrameDisplay=%d  m_is_alloc_vf=%d"), __LINE__, m_output_info.indexFrameDisplay, m_is_alloc_vf));
                    if(m_is_alloc_vf == MMP_TRUE) {
                        p_vpu_if->VPU_DecClrDispFlag(m_DecHandle, m_output_info.indexFrameDisplay);
                        m_buf_used_count[m_output_info.indexFrameDisplay]--;
                    }
                
                    /*
                        �Էµ� Stream�� ���� DisplayFrame�� �����ϴ� ���쿡��, ChunkReuse�� �ǹ̰� �ִ�. 
                        DiaplayFrame�� ���ٴ� �ǹ̴� BFrame ������ Delay�� PFrame�� ���̻� ���� �����̴�.
                    */
                    if(m_output_info.chunkReuseRequired)  {
                       *vpu_result |= CHUNK_REUSE;
        	            MMPDEBUGMSG(m_DEBUG_VPUDec, (TEXT("[CMmpDecoderVpuEx1::DecodeAu_PicEnd_WaitDone] m_output_info.chunkReuseRequired ")));
                    }

                }

                m_iframeSearchEnable = 0;
    
            }
            else if(m_output_info.picType == 4) /* Skip Type */{
               *vpu_result |= SKIP_TYPE;
               MMPDEBUGMSG(m_DEBUG_VPUDec, (TEXT("[CMmpDecoderVpuEx1::DecodeAu_PicEnd_WaitDone] SKIP_TYPE ")));
            }
            else if( (m_output_info.picType == 6) && (m_fourcc_in==MMP_FOURCC_VIDEO_H264) ) /* H264 SPS/PPS */ {
               *vpu_result |= H264_SPS_PPS;
               MMPDEBUGMSG(m_DEBUG_VPUDec, (TEXT("[CMmpDecoderVpuEx1::DecodeAu_PicEnd_WaitDone] ln=%d H264 SPS/PPS m_output_info.indexFrameDisplay=%d "), __LINE__, m_output_info.indexFrameDisplay));
            }
            else {
                MMPDEBUGMSG(m_DEBUG_VPUDec, (TEXT("[CMmpDecoderVpuEx1::DecodeAu_PicEnd_WaitDone] ln=%d m_output_info.indexFrameDisplay=%d "), __LINE__, m_output_info.indexFrameDisplay));
            }
            
            if(m_output_info.sequenceChanged) {
                *vpu_result |= SEQUENCE_CHANGE;
            }
            
        }
        else {
            mmpResult = MMP_FAILURE;
            MMPDEBUGMSG(m_DEBUG_VPUDec, (TEXT("[CMmpDecoderVpuEx1::DecodeAu_PicEnd_WaitDone] ln=%d FAIL: p_vpu_if->VPU_DecGetOutputInfo "), __LINE__));
        }
		
		if(m_output_info.numOfErrMBs > 0) {			
			MMPDEBUGMSG(m_DEBUG_VPUDec, (TEXT("[CMmpDecoderVpuEx1::DecodeAu_PicEnd_WaitDone] Num of Error Mbs : %d "), m_output_info.numOfErrMBs));
		}	
    }

    MMPDEBUGMSG(m_DEBUG_VPUDec, (TEXT("---------------- VPU Dec End ------- CodIdx=%d room2-room1 = %d    m_input_stream_size=%d --------------\n\n"), 
                    m_vpu_instance_index, m_room2-m_room1,  m_input_stream_size ));
    
    m_input_stream_count++;

    return mmpResult;
}

void CMmpDecoderVpuEx1::make_decOP_Common() {
    
    
    m_mapType = 0; //Map Type 0(Linear) / 1(Frame-V) / 2(Frame-H) / 3(Field-V) / 4(Mix-V) / 5(Frame-MB) / 6(Filed-MB) / 16(Tile2Linear)
        
    m_decOP.bitstreamFormat = (CodStd)STD_AVC_DEC; //0(H.264) / 1(VC1) / 2(MPEG2) / 3(MPEG4) / 4(H263) / 5(DIVX3) / 6(RV) / 7(AVS) / 11(VP8)
    m_decOP.avcExtension = 0;  // AVC extension 0(No) / 1(MVC) 
    m_decOP.coreIdx = m_codec_idx; 
#if 0
    m_decOP.bitstreamBuffer = m_vbStream.phys_addr;
	m_decOP.bitstreamBufferSize = m_vbStream.size;
#else

    m_decOP.bitstreamBuffer = this->m_p_stream_buffer->get_phy_addr();
	m_decOP.bitstreamBufferSize = this->m_p_stream_buffer->get_buf_size();
    MMPDEBUGMSG(1, (TEXT("[CMmpDecoderVpuEx1::make_decOP_Common] ln=%d 0x%08x %d"), __LINE__, m_decOP.bitstreamBuffer, m_decOP.bitstreamBufferSize));
#endif
	m_decOP.mp4DeblkEnable = 0;
	m_decOP.mp4Class = 0; //MPEG4 CLASS 0(MPEG4) / 1(DIVX 5.0 or higher) / 2(XVID) / 5(DIVX 4.0) / 8(DIVX/XVID Auto Detect)/ 256(Sorenson spark) :
	m_decOP.tiled2LinearEnable = (m_mapType>>4)&0x1;
    m_decOP.bitstreamMode = BS_MODE_PIC_END; //Bitstream Mode(0: Interrupt mode, 1: Rollback mode, 2: PicEnd mode) 
    m_decOP.cbcrInterleave = CBCR_INTERLEAVE;
   	m_decOP.bwbEnable	  = VPU_ENABLE_BWB;
    m_decOP.frameEndian	= VPU_FRAME_ENDIAN;
	m_decOP.streamEndian   = VPU_STREAM_ENDIAN;
}

void CMmpDecoderVpuEx1::make_decOP_H263() {

    this->make_decOP_Common();

    m_decOP.bitstreamFormat = STD_H263; //0(H.264) / 1(VC1) / 2(MPEG2) / 3(MPEG4) / 4(H263) / 5(DIVX3) / 6(RV) / 7(AVS) / 11(VP8)
    m_decOP.mp4DeblkEnable = 0;
	m_decOP.mp4Class = 0; //MPEG4 CLASS 0(MPEG4) / 1(DIVX 5.0 or higher) / 2(XVID) / 5(DIVX 4.0) / 8(DIVX/XVID Auto Detect)/ 256(Sorenson spark) :
	
}

void CMmpDecoderVpuEx1::make_decOP_H264() {
    
    this->make_decOP_Common();

    m_decOP.bitstreamFormat = STD_AVC; //0(H.264) / 1(VC1) / 2(MPEG2) / 3(MPEG4) / 4(H263) / 5(DIVX3) / 6(RV) / 7(AVS) / 11(VP8)
    m_decOP.avcExtension = 0;  // AVC extension 0(No) / 1(MVC) 
    
	
}

void CMmpDecoderVpuEx1::make_decOP_MPEG4(enum MPEG4_CLASS mp4class) {

    this->make_decOP_Common();

    m_decOP.bitstreamFormat = STD_MPEG4; //0(H.264) / 1(VC1) / 2(MPEG2) / 3(MPEG4) / 4(H263) / 5(DIVX3) / 6(RV) / 7(AVS) / 11(VP8)
    m_decOP.mp4DeblkEnable = 1;
	MMPDEBUGMSG(1, (TEXT("[CMmpDecoderVpuEx1::make_decOP_MPEG4] MPEG4 class = %d"),mp4class));	
	m_decOP.mp4Class = (int)mp4class; //MPEG4 CLASS 0(MPEG4) / 1(DIVX 5.0 or higher) / 2(XVID) / 5(DIVX 4.0) / 8(DIVX/XVID Auto Detect)/ 256(Sorenson spark) :
	
}

void CMmpDecoderVpuEx1::make_decOP_MPEG2() {

    this->make_decOP_Common();

    m_decOP.bitstreamFormat = STD_MPEG2; //0(H.264) / 1(VC1) / 2(MPEG2) / 3(MPEG4) / 4(H263) / 5(DIVX3) / 6(RV) / 7(AVS) / 11(VP8)
    m_decOP.mp4DeblkEnable = 0;
	m_decOP.mp4Class = 0; //MPEG4 CLASS 0(MPEG4) / 1(DIVX 5.0 or higher) / 2(XVID) / 5(DIVX 4.0) / 8(DIVX/XVID Auto Detect)/ 256(Sorenson spark) :
	
}

void CMmpDecoderVpuEx1::make_decOP_VC1() {

    this->make_decOP_Common();

    m_decOP.bitstreamFormat = STD_VC1; //0(H.264) / 1(VC1) / 2(MPEG2) / 3(MPEG4) / 4(H263) / 5(DIVX3) / 6(RV) / 7(AVS) / 11(VP8)
    m_decOP.mp4DeblkEnable = 0;
	m_decOP.mp4Class = 0; //MPEG4 CLASS 0(MPEG4) / 1(DIVX 5.0 or higher) / 2(XVID) / 5(DIVX 4.0) / 8(DIVX/XVID Auto Detect)/ 256(Sorenson spark) :
	
}

void CMmpDecoderVpuEx1::make_decOP_MSMpeg4V3() {

    this->make_decOP_Common();

    m_decOP.bitstreamFormat = STD_DIV3; //0(H.264) / 1(VC1) / 2(MPEG2) / 3(MPEG4) / 4(H263) / 5(DIVX3) / 6(RV) / 7(AVS) / 11(VP8)
    m_decOP.mp4DeblkEnable = 1;
	m_decOP.mp4Class = 0; //MPEG4 CLASS 0(MPEG4) / 1(DIVX 5.0 or higher) / 2(XVID) / 5(DIVX 4.0) / 8(DIVX/XVID Auto Detect)/ 256(Sorenson spark) :
	
}

void CMmpDecoderVpuEx1::make_decOP_RV30() {

    this->make_decOP_RV40();
}

void CMmpDecoderVpuEx1::make_decOP_RV40() {

    this->make_decOP_Common();

    m_decOP.bitstreamFormat = STD_RV; //0(H.264) / 1(VC1) / 2(MPEG2) / 3(MPEG4) / 4(H263) / 5(DIVX3) / 6(RV) / 7(AVS) / 11(VP8)
    m_decOP.mp4DeblkEnable = 0;
	m_decOP.mp4Class = 0; //MPEG4 CLASS 0(MPEG4) / 1(DIVX 5.0 or higher) / 2(XVID) / 5(DIVX 4.0) / 8(DIVX/XVID Auto Detect)/ 256(Sorenson spark) :
	
}

void CMmpDecoderVpuEx1::make_decOP_VP80() {

    this->make_decOP_Common();

    m_decOP.bitstreamFormat = STD_VP8; //0(H.264) / 1(VC1) / 2(MPEG2) / 3(MPEG4) / 4(H263) / 5(DIVX3) / 6(RV) / 7(AVS) / 11(VP8)
    m_decOP.mp4DeblkEnable = 0;
	m_decOP.mp4Class = 0; //MPEG4 CLASS 0(MPEG4) / 1(DIVX 5.0 or higher) / 2(XVID) / 5(DIVX 4.0) / 8(DIVX/XVID Auto Detect)/ 256(Sorenson spark) :
	
}

#if (VPU_SUPPORT_THEORA == 1)
void CMmpDecoderVpuEx1::make_decOP_Theora() {

    this->make_decOP_Common();

    m_decOP.bitstreamFormat = STD_THO; //0(H.264) / 1(VC1) / 2(MPEG2) / 3(MPEG4) / 4(H263) / 5(DIVX3) / 6(RV) / 7(AVS) / 11(VP8)
    m_decOP.mp4DeblkEnable = 0;
	m_decOP.mp4Class = 0; //MPEG4 CLASS 0(MPEG4) / 1(DIVX 5.0 or higher) / 2(XVID) / 5(DIVX 4.0) / 8(DIVX/XVID Auto Detect)/ 256(Sorenson spark) :

	theora_parser_init((void **)&m_thoParser); // theora parser init	
	
}
#endif

MMP_RESULT CMmpDecoderVpuEx1::make_seqheader_Common(class mmp_buffer_videostream* p_buf_vs) {

    MMP_RESULT mmpResult;
    MMP_U8 *p_stream, *p_dsi;
    MMP_S32 stream_size, dsi_size;

    p_stream = (MMP_U8*)p_buf_vs->get_buf_vir_addr();
    stream_size = p_buf_vs->get_stream_size();

    mmpResult = p_buf_vs->alloc_dsi_buffer(stream_size);
    if(mmpResult == MMP_SUCCESS) {

        p_dsi = (MMP_U8*)p_buf_vs->get_dsi_buffer();
        dsi_size = stream_size;
        MMP_MEMCPY(p_dsi, p_stream, dsi_size);
        p_buf_vs->set_dsi_size(dsi_size);
    }

    p_buf_vs->set_stream_offset(0);

    return mmpResult;
}

MMP_RESULT CMmpDecoderVpuEx1::make_seqheader_H264(class mmp_buffer_videostream* p_buf_vs) {

    MMP_RESULT mmpResult;
    MMP_U8 *p_avc_dsi, *p_h264_dsi;
    MMP_S32 avc_dsi_size, h264_dsi_size;

    p_avc_dsi = (MMP_U8*)p_buf_vs->get_buf_vir_addr();
    avc_dsi_size = p_buf_vs->get_stream_size();

    mmpResult = p_buf_vs->alloc_dsi_buffer(128+avc_dsi_size);
    if(mmpResult == MMP_SUCCESS) {

        p_h264_dsi = (MMP_U8*)p_buf_vs->get_dsi_buffer();
        mmpResult = CMmpH264Parser::ConvertDSI_AVC1_To_H264(p_avc_dsi, avc_dsi_size, p_h264_dsi, &h264_dsi_size, &m_is_avc, &m_nal_length_size);
        if(mmpResult == MMP_SUCCESS) {
            
        }
        else {
            MMP_MEMCPY(p_h264_dsi, p_avc_dsi, avc_dsi_size);
            h264_dsi_size = avc_dsi_size;
        }

        p_buf_vs->set_dsi_size(h264_dsi_size);
        mmpResult = MMP_SUCCESS;
    }

    p_buf_vs->set_stream_offset(0);

    return mmpResult;
}

MMP_RESULT CMmpDecoderVpuEx1::make_seqheader_WMV3(class mmp_buffer_videostream* p_buf_vs) {

    MMP_RESULT mmpResult = MMP_SUCCESS;
    MMP_S32 stream_size;
    MMP_S32 framerate, bitrate, w, h;
    
    MMP_U8 *pbHeader, *pstream;
    MMP_S32 header_size;

    pstream = (MMP_U8*)p_buf_vs->get_buf_vir_addr();
    stream_size = p_buf_vs->get_stream_size();

    mmpResult = p_buf_vs->alloc_dsi_buffer(128+stream_size);
    if(mmpResult == MMP_SUCCESS) {
    
        framerate = p_buf_vs->get_player_framerate();
        bitrate = p_buf_vs->get_player_bitrate();
        w = p_buf_vs->get_pic_width();
        h = p_buf_vs->get_pic_height();

        pbHeader = (MMP_U8*)p_buf_vs->get_dsi_buffer();

        header_size = 0;
        MMP_PUT_LE32(pbHeader, ((0xC5 << 24)|0));   
        header_size += 4; //version
        MMP_PUT_LE32(pbHeader, stream_size);
        header_size += 4;
        if(stream_size > 0) {
            MMP_PUT_BUFFER(pbHeader, pstream, stream_size);
            header_size += stream_size;
        }
        MMP_PUT_LE32(pbHeader, h);
        header_size += 4;
        MMP_PUT_LE32(pbHeader, w);
        header_size += 4;
        MMP_PUT_LE32(pbHeader, 12);
        header_size += 4;
        MMP_PUT_LE32(pbHeader, 2 << 29 | 1 << 28 | 0x80 << 24 | 1 << 0);
        header_size += 4; // STRUCT_B_FRIST (LEVEL:3|CBR:1:RESERVE:4:HRD_BUFFER|24)
        MMP_PUT_LE32(pbHeader, bitrate);
        header_size += 4; // hrd_rate
        MMP_PUT_LE32(pbHeader, framerate);            
        header_size += 4; // frameRate

        p_buf_vs->set_dsi_size(header_size);
        p_buf_vs->set_stream_offset(0);

        MMPDEBUGMSG(m_DEBUG_VPUDec, (TEXT("[CMmpDecoderVpuEx1::make_seqheader_WMV3] header_size=%d bitrate=%d framerate=%d "), header_size, bitrate, framerate ));
    }

    return mmpResult;
}


MMP_RESULT CMmpDecoderVpuEx1::make_seqheader_DIV3(class mmp_buffer_videostream* p_buf_vs) {

    MMP_RESULT mmpResult = MMP_SUCCESS;
    MMP_S32 stream_size;
    MMP_S32 w, h;
    
    MMP_U8 *pbHeader, *pstream;
    MMP_S32 header_size;

    pstream = (MMP_U8*)p_buf_vs->get_buf_vir_addr();
    stream_size = p_buf_vs->get_stream_size();

    mmpResult = p_buf_vs->alloc_dsi_buffer(128);
    if(mmpResult == MMP_SUCCESS) {
    
        w = p_buf_vs->get_pic_width();
        h = p_buf_vs->get_pic_height();

        pbHeader = (MMP_U8*)p_buf_vs->get_dsi_buffer();

        header_size = 0;
        MMP_PUT_LE32(pbHeader, MMP_MKTAG('C', 'N', 'M', 'V')); //signature 'CNMV'
        MMP_PUT_LE16(pbHeader, 0x00);                      //version
        MMP_PUT_LE16(pbHeader, 0x20);                      //length of header in bytes
        MMP_PUT_LE32(pbHeader, MMP_MKTAG('D', 'I', 'V', '3')); //codec FourCC
        MMP_PUT_LE16(pbHeader, w);                //width
        MMP_PUT_LE16(pbHeader, h);               //height
        MMP_PUT_LE32(pbHeader, 0 /*st->r_frame_rate.num*/);      //frame rate
        MMP_PUT_LE32(pbHeader, 0 /*st->r_frame_rate.den*/);      //time scale(?)
        MMP_PUT_LE32(pbHeader, 0 /*st->nb_index_entries*/);      //number of frames in file
        MMP_PUT_LE32(pbHeader, 0); //unused
        header_size += 32;

        p_buf_vs->set_dsi_size(header_size);
        p_buf_vs->set_stream_offset(0);
    }

    return mmpResult;
}

MMP_RESULT CMmpDecoderVpuEx1::make_seqheader_RV(class mmp_buffer_videostream* p_buf_vs) {

    MMP_RESULT mmpResult = MMP_SUCCESS;
    MMP_S32 stream_size;
    MMP_S32 w, h, framerate;
    
    MMP_U8 *pbHeader, *pstream;
    MMP_S32 header_size;

    pstream = (MMP_U8*)p_buf_vs->get_buf_vir_addr();
    stream_size = p_buf_vs->get_stream_size();

    mmpResult = p_buf_vs->alloc_dsi_buffer(128 + stream_size);
    if(mmpResult == MMP_SUCCESS) {
    
        framerate = p_buf_vs->get_player_framerate();
        w = p_buf_vs->get_pic_width();
        h = p_buf_vs->get_pic_height();

        pbHeader = (MMP_U8*)p_buf_vs->get_dsi_buffer();

        header_size = 26 + stream_size;
        MMP_PUT_BE32(pbHeader, header_size); //Length
        MMP_PUT_LE32(pbHeader, MMP_MKTAG('V', 'I', 'D', 'O')); //MOFTag
        MMP_PUT_LE32(pbHeader, m_fourcc_in); //SubMOFTagl
        MMP_PUT_BE16(pbHeader, w);
        MMP_PUT_BE16(pbHeader, h);
        MMP_PUT_BE16(pbHeader, 0x0c); //BitCount;
        MMP_PUT_BE16(pbHeader, 0x00); //PadWidth;
        MMP_PUT_BE16(pbHeader, 0x00); //PadHeight;
        
	    MMP_PUT_BE32(pbHeader, framerate<<16);
        MMP_PUT_BUFFER(pbHeader, pstream, stream_size); //OpaqueDatata
        
        p_buf_vs->set_dsi_size(header_size);
        p_buf_vs->set_stream_offset(0);
    }

    return mmpResult;
}

MMP_RESULT CMmpDecoderVpuEx1::make_seqheader_VP8(class mmp_buffer_videostream* p_buf_vs) {

    MMP_RESULT mmpResult = MMP_SUCCESS;
    MMP_S32 stream_size;
    MMP_S32 w, h, framerate;
    
    MMP_U8 *pbHeader, *pstream;
    MMP_S32 header_size;

    pstream = (MMP_U8*)p_buf_vs->get_stream_real_ptr();
    stream_size = p_buf_vs->get_stream_real_size();

    mmpResult = p_buf_vs->alloc_dsi_buffer(128);
    if(mmpResult == MMP_SUCCESS) {
    
        framerate = p_buf_vs->get_player_framerate();
        w = p_buf_vs->get_pic_width();
        h = p_buf_vs->get_pic_height();

        pbHeader = (MMP_U8*)p_buf_vs->get_dsi_buffer();

        header_size = 32;
        MMP_PUT_LE32(pbHeader, MMP_MKTAG('D', 'K', 'I', 'F')); //signature 'DKIF'
        MMP_PUT_LE16(pbHeader, 0x00);                      //version
        MMP_PUT_LE16(pbHeader, 0x20);                      //length of header in bytes
        MMP_PUT_LE32(pbHeader, MMP_MKTAG('V', 'P', '8', '0')); //codec FourCC
        MMP_PUT_LE16(pbHeader, w);                //width
        MMP_PUT_LE16(pbHeader, h);               //height
        MMP_PUT_LE32(pbHeader, 0 /*st->r_frame_rate.num*/);      //frame rate
        MMP_PUT_LE32(pbHeader, 0 /*st->r_frame_rate.den*/);      //time scale(?)
        MMP_PUT_LE32(pbHeader, 0 /*st->nb_index_entries*/);      //number of frames in file
        MMP_PUT_LE32(pbHeader, 0); //unused
        
        p_buf_vs->set_dsi_size(header_size);
    }

    return mmpResult;
}

#if (VPU_SUPPORT_THEORA == 1)
MMP_RESULT CMmpDecoderVpuEx1::make_seqheader_Theora(class mmp_buffer_videostream* p_buf_vs) {

    MMP_RESULT mmpResult = MMP_SUCCESS;
    MMP_U8 *p_stream, *p_dsi;
    MMP_S32 stream_size, dsi_size;
	MMP_S32 header_size;

    p_stream = (MMP_U8*)p_buf_vs->get_buf_vir_addr();
    stream_size = p_buf_vs->get_stream_size();
	p_buf_vs->alloc_dsi_buffer(128);
	MMPDEBUGMSG(1, (TEXT("alloc_dsi_buffer ")));			
	p_dsi = (MMP_U8*)p_buf_vs->get_dsi_buffer();
	MMPDEBUGMSG(1, (TEXT("make_seqheader_Theora p_stream =0x%x stream_size = 0x%x p_dsi = 0x%x"),p_stream, stream_size,p_dsi ));	
	m_thoParser->open(m_thoParser->handle, p_stream, stream_size, (int *)&m_dec_init_info.thoScaleInfo);	
	MMPDEBUGMSG(1, (TEXT("m_thoParser->open ")));		
	header_size = theora_make_stream((void *)m_thoParser->handle,p_dsi , SEQ_INIT);
	MMPDEBUGMSG(1, (TEXT("theora_make_stream ")));
	p_buf_vs->set_dsi_size(header_size);
	p_buf_vs->set_stream_offset(0);		

    return mmpResult;
}
#endif

MMP_RESULT CMmpDecoderVpuEx1::make_frameheader_Common(class mmp_buffer_videostream* p_buf_vs) {

    p_buf_vs->set_header_size(0);
    
    return MMP_SUCCESS;
}

MMP_RESULT CMmpDecoderVpuEx1::make_frameheader_H264(class mmp_buffer_videostream* p_buf_vs, MMP_S32* copyed_ausize) {

    //this->DecodeAu_StreamRemake_AVC1((MMP_U8*)p_buf_vs->get_buf_vir_addr(), p_buf_vs->get_stream_size()); 
    MMP_RESULT mmpResult;

    if(copyed_ausize) *copyed_ausize = 0;

    if(m_is_avc == 1) {

        MMP_U8* p_avc1 = (MMP_U8*)p_buf_vs->get_stream_real_ptr();
        MMP_S32 vs_size = p_buf_vs->get_stream_real_size();
        MMP_S32 h264_size;
        
        if(vs_size > 4) {

            if( (p_avc1[0]==0) &&
                (p_avc1[1]==0) &&
                (p_avc1[2]==0) &&
                (p_avc1[3]==1) ) {
        
                /* Nothing to do */
            }
            else {

                if(m_copyed_aubuf_max_size < (vs_size+1024) ) {
                    if(m_copyed_aubuf != NULL) {
                        delete [] m_copyed_aubuf;
                    }
                    m_copyed_aubuf_max_size = vs_size+(1024*128);
                    m_copyed_aubuf = new MMP_U8[m_copyed_aubuf_max_size];
                }

                mmpResult  = CMmpH264Parser::ConvertAvc1ToH264(p_avc1, vs_size,  m_nal_length_size, m_copyed_aubuf, (MMP_S32*)&h264_size, MMP_FALSE);
                if(mmpResult == MMP_SUCCESS) {
                    if(copyed_ausize) *copyed_ausize = h264_size;   
                }
            }

        }

    }

    p_buf_vs->set_header_size(0);

    /*
    MMP_S32 remain_size, stream_size;
    MMP_U32* ps;
    MMP_U8* pau;

    pau = (MMP_U8*)p_buf_vs->get_buf_vir_addr();
    remain_size = p_buf_vs->get_stream_size();

    while(remain_size > 4) {

        ps = (MMP_U32*)pau;
        stream_size = *ps;
        if(stream_size == 0x01000000) {
            break;
        }
        stream_size = MMP_SWAP_U32(stream_size);
        stream_size += 4;

        if( (stream_size < 4)
           || (stream_size > remain_size) 
        )
        {
            break;
        }

        
        *ps = 0x01000000;
        pau+=stream_size;
        remain_size -= stream_size;
    }

    p_buf_vs->set_header_size(0);
    p_buf_vs->set_stream_offset(0);
    */
        
    return MMP_SUCCESS;
}

MMP_RESULT CMmpDecoderVpuEx1::make_frameheader_WVC1(class mmp_buffer_videostream* p_buf_vs) {

    MMP_RESULT mmpResult = MMP_FAILURE;
    MMP_U8* pbChunk;
    MMP_S32 stream_size;
    MMP_BOOL flag = MMP_FALSE;

    pbChunk = (MMP_U8*)p_buf_vs->get_stream_real_ptr();
    stream_size = p_buf_vs->get_stream_real_size();
    if(stream_size >= 3) {
        
        if( pbChunk[0] != 0 || pbChunk[1] != 0 || pbChunk[2] != 1) // check start code as prefix (0x00, 0x00, 0x01) 
        {
            flag = MMP_TRUE;
        }
        else {
            mmpResult = MMP_SUCCESS;
        }
    }
    else {
        //MMPDEBUGMSG(1, (TEXT("[CMmpDecoderVpuEx1::make_frameheader_WVC1] Error : stream size is too small (%d byte) "), stream_size ));
        flag = MMP_TRUE;
        mmpResult = MMP_SUCCESS;
    }

    if(flag == MMP_TRUE) {
        mmpResult = p_buf_vs->alloc_header_buffer(4);
        if(mmpResult == MMP_SUCCESS) {

            MMP_U8* pbHeader = (MMP_U8*)p_buf_vs->get_header_buffer();

            pbHeader[0] = 0x00;
            pbHeader[1] = 0x00;
            pbHeader[2] = 0x01;
            pbHeader[3] = 0x0D;	// replace to the correct picture header to indicate as frame				

            p_buf_vs->set_header_size(4);
        }
    }

    
    return mmpResult;
}

MMP_RESULT CMmpDecoderVpuEx1::make_frameheader_WMV3(class mmp_buffer_videostream* p_buf_vs) {

    MMP_RESULT mmpResult = MMP_FAILURE;
    MMP_U8* p_header; 
    MMP_S32 stream_size;
    MMP_S64 pts;
    MMP_U32 flag;

    mmpResult = p_buf_vs->alloc_header_buffer(8);
    if(mmpResult == MMP_SUCCESS) {
        
        p_header = (MMP_U8*)p_buf_vs->get_header_buffer();
        pts = p_buf_vs->get_pts();
        stream_size = p_buf_vs->get_stream_real_size();
        flag = p_buf_vs->get_flag();

        if((flag&mmp_buffer_media::FLAG_VIDEO_KEYFRAME) != 0) {
            MMP_PUT_LE32(p_header,  stream_size | 0x80000000 );
        }
        else {
            MMP_PUT_LE32(p_header, stream_size | 0x00000000 );
        }

        MMP_PUT_LE32(p_header, pts/1000); /* milli_sec */

        p_buf_vs->set_header_size(8);
    }

    return MMP_SUCCESS;
}

MMP_RESULT CMmpDecoderVpuEx1::make_frameheader_DIV3(class mmp_buffer_videostream* p_buf_vs) {

    MMP_RESULT mmpResult = MMP_FAILURE;
    MMP_U8* p_header; 
    MMP_S32 stream_size;
    
    mmpResult = p_buf_vs->alloc_header_buffer(12);
    if(mmpResult == MMP_SUCCESS) {
        
        p_header = (MMP_U8*)p_buf_vs->get_header_buffer();
        stream_size = p_buf_vs->get_stream_real_size();

        MMP_PUT_LE32(p_header, stream_size);
        MMP_PUT_LE32(p_header, 0);
        MMP_PUT_LE32(p_header, 0);

        p_buf_vs->set_header_size(12);
    }

    return MMP_SUCCESS;
}

MMP_RESULT CMmpDecoderVpuEx1::make_frameheader_RV(class mmp_buffer_videostream* p_buf_vs) {

    MMP_RESULT mmpResult = MMP_FAILURE;
    MMP_U8 *p_header, *p_stream; 
    MMP_S32 stream_size, header_size;
    MMP_S32 cSlice, nSlice, offset, i;
    MMP_U32 val;

    p_stream = (MMP_U8*)p_buf_vs->get_stream_real_ptr();
    stream_size = p_buf_vs->get_stream_real_size();

    cSlice = ((MMP_S32)p_stream[0])&0xFF;
    cSlice++;
    nSlice = stream_size - 1 - (cSlice * 8);
    header_size = 20 + (cSlice*8);
    
    mmpResult = p_buf_vs->alloc_header_buffer(header_size);
    if(mmpResult == MMP_SUCCESS) {
        
        p_header = (MMP_U8*)p_buf_vs->get_header_buffer();
        
        MMP_PUT_BE32(p_header, nSlice);
        MMP_PUT_LE32(p_header, 0);   /* time stamp (milesec) */
        MMP_PUT_BE16(p_header, 0);//m_input_stream_count);
        MMP_PUT_BE16(p_header, 0x02); //Flags
        MMP_PUT_BE32(p_header, 0x00); //LastPacket
        MMP_PUT_BE32(p_header, cSlice); //NumSegments

        offset = 1;
        for (i = 0; i < cSlice; i++)   {

            val = (p_stream[offset+3] << 24) | (p_stream[offset+2] << 16) | (p_stream[offset+1] << 8) | p_stream[offset];
            MMP_PUT_BE32(p_header, val); //isValid
            offset += 4;
            val = (p_stream[offset+3] << 24) | (p_stream[offset+2] << 16) | (p_stream[offset+1] << 8) | p_stream[offset];
            MMP_PUT_BE32(p_header, val); //Offset
            offset += 4;
        }

        p_buf_vs->set_header_size(header_size);
        p_buf_vs->inc_stream_offset(1+(cSlice*8));
    }

    return MMP_SUCCESS;
}

MMP_RESULT CMmpDecoderVpuEx1::make_frameheader_VP8(class mmp_buffer_videostream* p_buf_vs) {

    MMP_RESULT mmpResult = MMP_FAILURE;
    MMP_U8* p_header; 
    MMP_S32 stream_size;
    
    mmpResult = p_buf_vs->alloc_header_buffer(12);
    if(mmpResult == MMP_SUCCESS) {
        
        p_header = (MMP_U8*)p_buf_vs->get_header_buffer();
        stream_size = p_buf_vs->get_stream_real_size();

        MMP_PUT_LE32(p_header, stream_size);
        MMP_PUT_LE32(p_header, 0);
        MMP_PUT_LE32(p_header, 0);

        p_buf_vs->set_header_size(12);
    }

    return MMP_SUCCESS;
}

#if (VPU_SUPPORT_THEORA == 1)
MMP_RESULT CMmpDecoderVpuEx1::make_frameheader_Theora(class mmp_buffer_videostream* p_buf_vs) {

    MMP_RESULT mmpResult = MMP_FAILURE;
    MMP_U8* p_header; 
	MMP_U8* p_stream;
    MMP_S32 stream_size;
    int ret = 0;
	
    p_stream = (MMP_U8*)p_buf_vs->get_steream_real_ptr();
    stream_size = p_buf_vs->get_stream_real_size();	

	if((ret  = m_thoParser->read_frame((void *)m_thoParser->handle, p_stream, stream_size)) < 0) 
	{
		MMPDEBUGMSG(1, (TEXT("read_frame error !!!!")));
		return mmpResult;
	}
	
    mmpResult = p_buf_vs->alloc_header_buffer(SIZE_THO_STREAM);
	
	if(mmpResult == MMP_SUCCESS) {
		
        p_header = (MMP_U8*)p_buf_vs->get_header_buffer();
		/* refer to 6.2.3.2 Macroblock order Matching,
		6.2.3.3  Macroblock Packing in Programmer User Guide */		
        stream_size = theora_make_stream((void *)m_thoParser->handle, p_header, PIC_RUN);

        p_buf_vs->set_header_size(stream_size);
	}
	
    return MMP_SUCCESS;
}
#endif
