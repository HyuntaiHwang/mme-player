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

#include "MmpDecoderVpuIF.hpp"
#include "MmpUtil.hpp"
#include "vpuhelper.h"
#include "MmpH264Tool.hpp"
#include "mmp_buffer_mgr.hpp"
#include "mmp_lock.hpp"

#define VPU_ENC_TIMEOUT       1000
#define VPU_DEC_TIMEOUT       10000
#define VPU_WAIT_TIME_OUT	100		//should be less than normal decoding time to give a chance to fill stream. if this value happens some problem. we should fix VPU_WaitInterrupt function
//#define PARALLEL_VPU_WAIT_TIME_OUT 0 	//the value of timeout is 0 means we just check interrupt flag. do not wait any time to give a chance of an interrupt of the next core.


//#if PARALLEL_VPU_WAIT_TIME_OUT > 0 
//#undef VPU_DEC_TIMEOUT
//#define VPU_DEC_TIMEOUT       1000
//#endif


#define EXTRA_FRAME_BUFFER_NUM	1
#define STREAM_BUF_SIZE		 0x300000  // max bitstream size
#define STREAM_END_SIZE			0

#ifdef WIN32
/* this is test param on WIN32 */
int VPU_PIC_WIDTH = 1920;
int VPU_PIC_HEIGHT = 1080;
#endif

/////////////////////////////////////////////////////////////
//CMmpDecoderVpuIF Member Functions

CMmpDecoderVpuIF::CMmpDecoderVpuIF(enum MMP_FOURCC fourcc_in) :

m_fourcc_in(fourcc_in)

,m_p_vpu_if(NULL)
,m_vpu_instance_index(-1)
,m_reUseChunk(0)
,m_codec_idx(0)
,m_version(0)
,m_revision(0)
,m_productId(0)
,m_skipframeMode(0)
,m_randomaccess(MMP_PLAY_BACK)

,m_DecHandle(NULL)
,m_regFrameBufCount(0)

,m_p_stream_buffer(NULL)

,m_last_int_reason(0)
,m_input_stream_count(0)

#if (VPU_SUPPORT_THEORA == 1)
,m_thoParser(NULL)
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
    }
    
}

CMmpDecoderVpuIF::~CMmpDecoderVpuIF()
{
    
}

MMP_RESULT CMmpDecoderVpuIF::Open()
{
    RetCode vpu_ret;
    MMP_RESULT mmpResult = MMP_SUCCESS;
    
    if(mmpResult == MMP_SUCCESS) {
        m_p_vpu_if = mmp_vpu_if::create_object();
        if(m_p_vpu_if == NULL) {
            mmpResult = MMP_FAILURE;
            MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpDecoderVpuIF::Open] FAIL : mmp_vpu_if::create_object\n")));
            return mmpResult;
        }
    }
    
    class mmp_lock autolock((class mmp_oal_lock*)m_p_vpu_if->get_external_mutex());

    if(mmpResult == MMP_SUCCESS) {
            
        m_p_vpu_if->VPU_GetVersionInfo(m_codec_idx, &m_version, &m_revision, &m_productId);	

        MMPDEBUGMSG(MMPZONE_INFO, (TEXT("VPU coreNum : [%d]\n"), m_codec_idx));
        MMPDEBUGMSG(MMPZONE_INFO, (TEXT("Firmware Version => projectId : %x | version : %04d.%04d.%08d | revision : r%d\n"), 
                    (Uint32)(m_version>>16), (Uint32)((m_version>>(12))&0x0f), (Uint32)((m_version>>(8))&0x0f), (Uint32)((m_version)&0xff), m_revision));
        MMPDEBUGMSG(MMPZONE_INFO, (TEXT("Hardware Version => %04x\n"), m_productId));
        MMPDEBUGMSG(MMPZONE_INFO, (TEXT("API Version => %04x\n\n"), API_VERSION));
    }

    /* alloc dma buffer */
    if(mmpResult == MMP_SUCCESS) {
        
        m_p_stream_buffer = mmp_buffer_mgr::get_instance()->alloc_dma_buffer(STREAM_BUF_SIZE);
        if(m_p_stream_buffer == NULL) {
            MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpDecoderVpuIF::Open] FAIL :  alloc stream buffer\n")));
            mmpResult = MMP_FAILURE;
        }
        else {

            class mmp_buffer_addr buf_addr;

            buf_addr = this->m_p_stream_buffer->get_buf_addr();
            m_vpu_stream_buffer.base = buf_addr.m_vir_addr;
            m_vpu_stream_buffer.phys_addr = buf_addr.m_phy_addr;
            m_vpu_stream_buffer.size = buf_addr.m_size;
            m_vpu_stream_buffer.virt_addr = buf_addr.m_vir_addr;

            m_p_vpu_if->vdi_register_dma_memory(m_codec_idx, &m_vpu_stream_buffer);
        }
    }

    /* Decoder Open */
    if(mmpResult == MMP_SUCCESS) {

        switch(m_fourcc_in) {
    
            /* Video */
            case MMP_FOURCC_VIDEO_H263: this->make_decOP_H263(); break;
            case MMP_FOURCC_VIDEO_H264: this->make_decOP_H264(); break;
            case MMP_FOURCC_VIDEO_MPEG4: this->make_decOP_MPEG4(0); break;
			case MMP_FOURCC_VIDEO_FLV1: this->make_decOP_MPEG4(256); break;			
#if (VPU_SUPOORT_DIV5 == 1)
			case MMP_FOURCC_VIDEO_DIV5: this->make_decOP_MPEG4(1); break;
#endif
			case MMP_FOURCC_VIDEO_XVID: this->make_decOP_MPEG4(2); break;			
#if (VPU_SUPOORT_DIVX == 1)
			case MMP_FOURCC_VIDEO_DIVX: this->make_decOP_MPEG4(5); break;			
#endif
            case MMP_FOURCC_VIDEO_MPEG2: this->make_decOP_MPEG2(); break;
            
            case MMP_FOURCC_VIDEO_WMV3:  this->make_decOP_VC1(); break;

            case MMP_FOURCC_VIDEO_MSMPEG4V2:
            case MMP_FOURCC_VIDEO_MSMPEG4V3: this->make_decOP_MSMpeg4V3(); break;
            
            case MMP_FOURCC_VIDEO_RV30: this->make_decOP_RV30(); break;
            case MMP_FOURCC_VIDEO_RV40: this->make_decOP_RV40(); break;

            case MMP_FOURCC_VIDEO_VP80: this->make_decOP_VP80(); break;

#if (VPU_SUPPORT_THEORA == 1)
            case MMP_FOURCC_VIDEO_THEORA: this->make_decOP_Theora(); break;
#endif

            default:  
                mmpResult = MMP_FAILURE;
                MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpDecoderVpuIF::Open] FAIL : Not Support Format(%c%c%c%c) "), 
                            MMPGETFOURCC(m_fourcc_in, 0),MMPGETFOURCC(m_fourcc_in, 1),
                            MMPGETFOURCC(m_fourcc_in, 2),MMPGETFOURCC(m_fourcc_in, 3)
                          ));
                break;
        }

        if(mmpResult == MMP_SUCCESS) {

            vpu_ret = m_p_vpu_if->VPU_DecOpen(&m_DecHandle, &m_decOP);
	        if( vpu_ret != RETCODE_SUCCESS ) {
		        MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpDecoderVpuIF::Open] FAIL :  m_p_vpu_if->VPU_DecOpen (vpu_ret=%d) \n"), vpu_ret));
                mmpResult = MMP_FAILURE;
	        }
            else {

                m_vpu_instance_index = m_p_vpu_if->VPU_GetCodecInstanceIndex((void*)m_DecHandle);
               
                if (m_decOP.bitstreamMode == BS_MODE_PIC_END) {
                    m_p_vpu_if->VPU_DecSetRdPtr(m_DecHandle, m_decOP.bitstreamBuffer, 1);	
                }

                SecAxiUse		secAxiUse = {0};
                secAxiUse.useBitEnable  = USE_BIT_INTERNAL_BUF;
	            secAxiUse.useIpEnable   = USE_IP_INTERNAL_BUF;
	            secAxiUse.useDbkYEnable = USE_DBKY_INTERNAL_BUF;
	            secAxiUse.useDbkCEnable = USE_DBKC_INTERNAL_BUF;
	            secAxiUse.useBtpEnable  = USE_BTP_INTERNAL_BUF;
	            secAxiUse.useOvlEnable  = USE_OVL_INTERNAL_BUF;
	            m_p_vpu_if->VPU_DecGiveCommand(m_DecHandle, SET_SEC_AXI, &secAxiUse);


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
	            m_p_vpu_if->VPU_DecGiveCommand(m_DecHandle, SET_CACHE_CONFIG, &decCacheConfig);
            }
        }
	}

    MMPDEBUGMSG(MMPZONE_MONITOR, (TEXT("[CMmpDecoderVpuIF::Open] res=%d  CodecInstance=%d "), mmpResult, m_vpu_instance_index));
    
    return mmpResult;
}


MMP_RESULT CMmpDecoderVpuIF::Close()
{
    MMP_S32 i;
    
    if(m_p_vpu_if)  m_p_vpu_if->enter_critical_section();

    if(m_DecHandle != NULL) {

       m_p_vpu_if->VPU_DecUpdateBitstreamBuffer(m_DecHandle, STREAM_END_SIZE);
	   m_p_vpu_if->VPU_DecClose(m_DecHandle);
       m_DecHandle = NULL;

       if(m_p_stream_buffer != NULL) {

           m_p_vpu_if->vdi_unregister_dma_memory(m_codec_idx, &m_vpu_stream_buffer);
           mmp_buffer_mgr::get_instance()->free_buffer(m_p_stream_buffer);
           m_p_stream_buffer = NULL;
       }
    }
  
    for(i = 0; i < MAX_FRAMEBUFFER_COUNT; i++) {
        if(m_p_buf_vf_arr[i] != NULL)   {
            mmp_buffer_mgr::get_instance()->free_media_buffer(m_p_buf_vf_arr[i]);
        }
    }

    if(m_p_vpu_if) m_p_vpu_if->leave_critical_section();

    if(m_p_vpu_if != NULL) {
        mmp_vpu_if::destroy_object(m_p_vpu_if);
        m_p_vpu_if = NULL;
    }

    return MMP_SUCCESS;
}


MMP_RESULT CMmpDecoderVpuIF::DecodeDSI(class mmp_buffer_videostream* p_buf_vs) {

    MMP_U8* p_stream;
    MMP_S32 stream_size;

    MMP_RESULT mmpResult = MMP_SUCCESS;
    MMP_S32 size;
    RetCode vpu_ret;
    
    class mmp_lock autolock((class mmp_oal_lock*)m_p_vpu_if->get_external_mutex());

    p_stream = (MMP_U8*)p_buf_vs->get_buf_vir_addr();
    stream_size = p_buf_vs->get_stream_size();

    MMPDEBUGMSG(1, (TEXT("[CMmpDecoderVpuIF::DecodeDSI] ln=%d sz=%d (%02x %02x %02x %02x, %02x %02x %02x %02x, %02x %02x %02x %02x, %02x %02x %02x %02x ) "), 
                  __LINE__, stream_size,
                   p_stream[0], p_stream[1], p_stream[2], p_stream[3], 
                   p_stream[4], p_stream[5], p_stream[6], p_stream[7], 
                   p_stream[8], p_stream[9], p_stream[10], p_stream[11], 
                   p_stream[12], p_stream[13], p_stream[14], p_stream[15] 
          ));


    
    switch(m_fourcc_in) {

        case MMP_FOURCC_VIDEO_H264:
            mmpResult = this->make_seqheader_H264(p_buf_vs);
            break;

        case MMP_FOURCC_VIDEO_WMV3:
            mmpResult = this->make_seqheader_VC1(p_buf_vs);
            break;

        case MMP_FOURCC_VIDEO_MSMPEG4V2:
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
    
    /* Input DSI Stream */
    if(mmpResult == MMP_SUCCESS) {
        size = m_p_vpu_if->WriteBsBufFromBufHelper(m_codec_idx,  m_DecHandle, 
                                                  &m_vpu_stream_buffer, 
                                                  (MMP_U8*)p_buf_vs->get_dsi_buffer(), p_buf_vs->get_dsi_size(), 
                                                  m_decOP.streamEndian);
	    if (size <0) {
            MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpDecoderVpuIF::DecodeDSI] FAIL: WriteBsBufFromBufHelper ")));
		    mmpResult = MMP_FAILURE;
	    }
    }
    else {
        MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpDecoderVpuIF::DecodeDSI] FAIL: make seqheader ")));
    }

    
    /* RUN Seq Init */
    if(mmpResult == MMP_SUCCESS) {

        if(m_decOP.bitstreamMode == BS_MODE_PIC_END)
	    {
		    vpu_ret = m_p_vpu_if->VPU_DecGetInitialInfo(m_DecHandle, &m_dec_init_info);
		    if(vpu_ret != RETCODE_SUCCESS) {
                MMPDEBUGMSG(1, (TEXT("[CMmpDecoderVpuIF::DecodeDSI] FAIL: m_p_vpu_if->VPU_DecGetInitialInfo ln=%d  vpu_ret=%d "), __LINE__, vpu_ret));
                mmpResult = MMP_FAILURE;
		    }
		    m_p_vpu_if->VPU_ClearInterrupt(m_codec_idx);
	    }
    }

    if(mmpResult == MMP_SUCCESS) {
        mmpResult = this->SetVpuFrameBuffer();
    }

    MMPDEBUGMSG(1, (TEXT("[CMmpDecoderVpuIF::DecodeDSI] ln=%d  res=0x%x m_regFrameBufCount=%d m_dec_init_info.minFrameBufferCount=%d w=%d h=%d "), 
                              __LINE__, mmpResult, 
                              m_regFrameBufCount,
                              m_dec_init_info.minFrameBufferCount,
                              m_dec_init_info.picWidth, m_dec_init_info.picHeight
                              ));
            
    return mmpResult;
}

MMP_RESULT CMmpDecoderVpuIF::SetVpuFrameBuffer(void) {

    MMP_RESULT mmpResult = MMP_SUCCESS;
    RetCode vpu_ret;
    class mmp_buffer_videoframe* p_buf_vf;

    if(m_regFrameBufCount < this->GetVpuMinFrameBufferCount() ) {

        FrameBuffer user_frame[MAX_FRAMEBUFFER_COUNT];//*pUserFrame = NULL;
        class mmp_buffer_addr buf_addr;
        MMP_S32 i;
        MMP_S32 pic_width, pic_height;
        MMP_S32 framebufStride, framebufHeight;

        pic_width = this->GetVpuPicWidth();
        pic_height = this->GetVpuPicHeight();

        framebufStride = MMP_BYTE_ALIGN(pic_width, 16);
        framebufHeight = MMP_BYTE_ALIGN(pic_height, 16);

        m_regFrameBufCount = this->GetVpuMinFrameBufferCount()+2;
        for(i = 0; i < m_regFrameBufCount; i++) {
            m_p_buf_vf_arr[i] = mmp_buffer_mgr::get_instance()->alloc_media_videoframe((MMP_MEDIA_ID)this, framebufStride, framebufHeight, MMP_FOURCC_IMAGE_YVU420);
            if(m_p_buf_vf_arr[i] == NULL) {
                break;
            }
        }
                
        if( (i==m_regFrameBufCount) 
            && (m_dec_init_info.minFrameBufferCount >= 1) 
            && (m_dec_init_info.picWidth > 16)
            && (m_dec_init_info.picHeight > 16) )
        {
        
            for(i = 0; i < m_regFrameBufCount; i++) {
                    
                    memset(&user_frame[i], 0x00, sizeof(FrameBuffer));

                    p_buf_vf = m_p_buf_vf_arr[i];
                    
                    user_frame[i].bufY = p_buf_vf->get_buf_phy_addr_y();
                    user_frame[i].bufCb = p_buf_vf->get_buf_phy_addr_u();
                    user_frame[i].bufCr = p_buf_vf->get_buf_phy_addr_v();
                    user_frame[i].mapType = m_mapType;
                    user_frame[i].stride = framebufStride;
                    user_frame[i].height = framebufHeight;
                    user_frame[i].myIndex = i;
                    
            }

            vpu_ret = m_p_vpu_if->VPU_DecRegisterFrameBuffer(m_DecHandle, user_frame, m_regFrameBufCount, framebufStride, framebufHeight, m_mapType);
            if(vpu_ret != RETCODE_SUCCESS)  {
                MMPDEBUGMSG(1, (TEXT("[CMmpDecoderVpuIF::DecodeDSI] FAIL: m_p_vpu_if->VPU_DecRegisterFrameBuffer ")));
                mmpResult = MMP_FAILURE;
            }
            

        } /* end of if(m_dec_init_info.minFrameBufferCount >= 1) */
        else {
            MMPDEBUGMSG(1, (TEXT("[CMmpDecoderVpuIF::SetVpuFrameBuffer] ln=%d FAIL:  m_regFrameBufCount=(%d/%d) m_dec_init_info.minFrameBufferCount=%d w=%d h=%d "), 
                              __LINE__,
                              i, m_regFrameBufCount,
                              m_dec_init_info.minFrameBufferCount,
                              m_dec_init_info.picWidth, m_dec_init_info.picHeight
                              ));
            mmpResult = MMP_FAILURE;
        }
    }

    return mmpResult;
}

MMP_RESULT CMmpDecoderVpuIF::Play_Function_Tool(MMP_PLAY_FORMAT playformat, MMP_S64 curpos, MMP_S64 totalpos){
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

#define VDEC_DEBUG 0
MMP_RESULT CMmpDecoderVpuIF::DecodeAu_PinEnd(class mmp_buffer_videostream* p_buf_vs, class mmp_buffer_videoframe** pp_buf_vf) {

    MMP_RESULT mmpResult = MMP_SUCCESS;
    RetCode vpu_ret;
    DecParam		decParam	= {0};
    MMP_S32 int_reason, size;
    MMP_U32 start_tick, t1, t2, t3 ,t4;
    MMP_U8* p_stream;
    MMP_S32 stream_size;
    
    class mmp_lock autolock((class mmp_oal_lock*)m_p_vpu_if->get_external_mutex());

    start_tick = CMmpUtil::GetTickCount();

    decParam.iframeSearchEnable = 0;
    decParam.skipframeMode = m_skipframeMode;
    //decParam.skipframeMode = 0;
    decParam.DecStdParam.mp2PicFlush = 0;
    p_stream = (MMP_U8*)p_buf_vs->get_buf_vir_addr();
    stream_size = p_buf_vs->get_stream_size();

    MMPDEBUGMSG(VDEC_DEBUG, (TEXT("\n[CMmpDecoderVpuIF::DecodeAu_PinEnd] ln=%d cnt=%d sz=%d (%02x %02x %02x %02x, %02x %02x %02x %02x, %02x %02x %02x %02x, %02x %02x %02x %02x ) "), 
                  __LINE__, m_input_stream_count, stream_size,
                   p_stream[0], p_stream[1], p_stream[2], p_stream[3], 
                   p_stream[4], p_stream[5], p_stream[6], p_stream[7], 
                   p_stream[8], p_stream[9], p_stream[10], p_stream[11], 
                   p_stream[12], p_stream[13], p_stream[14], p_stream[15] 
    ));


#if 0
	if(m_randomaccess)
    {
    	int i;
        
        for(i=0; i<m_regFrameBufCount; i++)
        	VPU_DecClrDispFlag(m_DecHandle, i);sto
        
        vpu_ret = VPU_DecFrameBufferFlush(m_DecHandle);
        if( vpu_ret != RETCODE_SUCCESS )
        {
            MMPDEBUGMSG(MMPZONE_ERROR,(TEXT("VPU_DecGetBitstreamBuffer failed Error code is 0x%x \n"), vpu_ret));
            return MMP_FAILURE;
        }

		if((m_randomaccess == MMP_PLAY_FF) || (m_randomaccess == MMP_PLAY_REW))
		{
	        vpu_ret = VPU_DecGiveCommand(m_DecHandle, DEC_DISABLE_SKIP_REORDER, 0);
	        if( vpu_ret != RETCODE_SUCCESS )
	        {
				MMPDEBUGMSG(MMPZONE_ERROR,(TEXT("VPU_DecGiveCommand(DEC_DISABLE_SKIP_REORDER) failed Error code is 0x%x \n"), vpu_ret));
	            return MMP_FAILURE;
	        }
		}
		m_randomaccess = MMP_PLAY_BACK;    
    }
#endif



CHUNK_REUSE:
    /* check header */
    switch(m_fourcc_in) {

        case MMP_FOURCC_VIDEO_H264: 
            mmpResult = this->make_frameheader_H264(p_buf_vs);
            break;

        case MMP_FOURCC_VIDEO_WMV3:
            mmpResult = this->make_frameheader_VC1(p_buf_vs);
            break;

        case MMP_FOURCC_VIDEO_MSMPEG4V2:
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

#if 0
	if(m_input_stream_count==4)
	{
		printf("m_input_stream_count == 4  skipped \n");
		return mmpResult;
	}
#endif	

    MMPDEBUGMSG(VDEC_DEBUG, (TEXT("\n[CMmpDecoderVpuIF::DecodeAu_PinEnd] ln=%d cnt=%d sz=%d p_stream addr =0x%x(%02x %02x %02x %02x, %02x %02x %02x %02x, %02x %02x %02x %02x, %02x %02x %02x %02x ) "), 
                  __LINE__, m_input_stream_count, stream_size,p_stream,
                   p_stream[0], p_stream[1], p_stream[2], p_stream[3], 
                   p_stream[4], p_stream[5], p_stream[6], p_stream[7], 
                   p_stream[8], p_stream[9], p_stream[10], p_stream[11], 
                   p_stream[12], p_stream[13], p_stream[14], p_stream[15] 
    ));
#if 1
    m_reUseChunk = 0;
#endif

	if(m_reUseChunk==1)
	{
		MMPDEBUGMSG(0, (TEXT("--------- Reuse Process ---------- "))); 
	}
	else		
	{		
		if(m_input_stream_count) // AVC1 format sequence after pps data reuse
        	m_p_vpu_if->VPU_DecSetRdPtr(m_DecHandle, m_decOP.bitstreamBuffer, 1);	
		
		MMPDEBUGMSG(VDEC_DEBUG, (TEXT("--------- NormalProcess ---------- "))); 
       
        if(mmpResult == MMP_SUCCESS) {
    
            if(p_buf_vs->get_header_size() > 0) {
                size = m_p_vpu_if->WriteBsBufFromBufHelper(m_codec_idx, m_DecHandle, 
                                                           &m_vpu_stream_buffer, 
                                                           (BYTE*)p_buf_vs->get_header_buffer(), p_buf_vs->get_header_size(), 
                                                           m_decOP.streamEndian);
                if (size <0)
    	        {
    		        mmpResult = MMP_FAILURE;
                    MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpDecoderVpuIF::DecodeAu_PinEnd] FAIL: WriteBsBufFromBufHelper")));
    	        }
            }
			
            if(m_fourcc_in != MMP_FOURCC_VIDEO_THEORA)
            {
	            if(mmpResult == MMP_SUCCESS) {
	                size = m_p_vpu_if->WriteBsBufFromBufHelper(m_codec_idx, m_DecHandle, 
	                                                           &m_vpu_stream_buffer, 
	                                                           p_buf_vs->get_stream_real_ptr(), p_buf_vs->get_stream_real_size(), 
	                                                           m_decOP.streamEndian);
	                if (size <0)
	    	        {
	    		        mmpResult = MMP_FAILURE;
	                    MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpDecoderVpuIF::DecodeAu_PinEnd] FAIL: WriteBsBufFromBufHelper")));
	    	        }
            	}
            }
			
        }
        else {
            MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpDecoderVpuIF::DecodeAu_PinEnd] FAIL: make frame header")));
        }
        
	}    
    t1 = CMmpUtil::GetTickCount();
    #if 1
	{
		PhysicalAddress rdPtr, wrPtr;
		MMP_S32 room;
		m_p_vpu_if->VPU_DecGetBitstreamBuffer(m_DecHandle, &rdPtr, &wrPtr, (int*)&room);
		MMPDEBUGMSG(0, (TEXT("Pre Read ptr = 0x%08x | Write ptr = 0x%08x"), rdPtr, wrPtr));	
	}	
	#endif

	
    if(mmpResult == MMP_SUCCESS)  {

        if( (m_last_int_reason&(1<<INT_BIT_DEC_FIELD)) != 0) {
           m_p_vpu_if->VPU_ClearInterrupt(m_codec_idx);
           MMPDEBUGMSG(0, (TEXT("[second] INT_BIT_DEC_FIELD")));
        }
        else {

	        // Start decoding a frame.
START_DEC:            
	        vpu_ret = m_p_vpu_if->VPU_DecStartOneFrame(m_DecHandle, &decParam);
		    MMPDEBUGMSG(0, (TEXT("[first] VPU_DecStartOneFrame start")));
			if(vpu_ret == RETCODE_FRAME_NOT_COMPLETE)
			{
				 //MMPDEBUGMSG(0, (TEXT("[CMmpDecoderVpuIF::DecodeAu_PinEnd] wait instance")));
				 //printf(".");
				 //CMmpUtil::Sleep(1000);
				 goto START_DEC;
			}			
	        if (vpu_ret != RETCODE_SUCCESS) 
	        {
		        mmpResult = MMP_FAILURE;
                MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpDecoderVpuIF::DecodeAu_PinEnd] ln=%d FAIL: m_p_vpu_if->VPU_DecStartOneFrame vpu_ret=%d "), __LINE__, vpu_ret));
	        }
        }
    }

    t2 = CMmpUtil::GetTickCount();

    int_reason = 0;
    while(mmpResult == MMP_SUCCESS) {
    
        int_reason = m_p_vpu_if->VPU_WaitInterrupt(m_codec_idx, VPU_DEC_TIMEOUT);
        if (int_reason == (Uint32)-1) // timeout
		{
            MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpDecoderVpuIF::DecodeAu_PinEnd] ln=%d  FAIL: m_p_vpu_if->VPU_WaitInterrupt   TimeOut "), __LINE__));
			VPU_SWReset(m_codec_idx, SW_RESET_SAFETY, m_DecHandle);				
			mmpResult = MMP_FAILURE;
            int_reason = 0;
            break;
		}		
        
        if (int_reason & (1<<INT_BIT_DEC_FIELD)) {

            PhysicalAddress rdPtr, wrPtr;
            MMP_S32 room, chunkSize, picHeaderSize, seqHeaderSize;
            MMP_S32 remain_size;
            
            chunkSize = p_buf_vs->get_stream_size();
            picHeaderSize = p_buf_vs->get_header_size();
            seqHeaderSize = 0;

            m_p_vpu_if->VPU_DecGetBitstreamBuffer(m_DecHandle, &rdPtr, &wrPtr, (int*)&room);

            if(rdPtr <= wrPtr)  {
                remain_size = wrPtr - rdPtr;
            }
            else {
                remain_size = wrPtr-m_decOP.bitstreamBuffer;
                remain_size += m_decOP.bitstreamBufferSize - (rdPtr-m_decOP.bitstreamBuffer);
            }
            
            if(remain_size > 8) //(rdPtr-m_decOP.bitstreamBuffer) < (PhysicalAddress)(chunkSize+picHeaderSize+seqHeaderSize-8))	// there is full frame data in chunk data.
            {
				//m_p_vpu_if->VPU_DecSetRdPtr(m_DecHandle, rdPtr, 0);		//set rdPtr to the position of next field data.
                MMPDEBUGMSG(VDEC_DEBUG, (TEXT("[CMmpDecoderVpuIF::DecodeAu_PinEnd] INT_BIT_DEC_FIELD 0x%08x Wait Next Dec (0x%08x 0x%08x 0x%08x ) remain_size:%d"), int_reason , rdPtr, wrPtr , m_decOP.bitstreamBuffer, remain_size));
            }
            else {
                MMPDEBUGMSG(VDEC_DEBUG, (TEXT("[CMmpDecoderVpuIF::DecodeAu_PinEnd] INT_BIT_DEC_FIELD 0x%08x Wait Next Stream (0x%08x 0x%08x 0x%08x ) remain_size:%d"), int_reason , rdPtr, wrPtr , m_decOP.bitstreamBuffer, remain_size));
                mmpResult = MMP_FAILURE;
                break;
            }
            
        }

        if (int_reason) {
    		m_p_vpu_if->VPU_ClearInterrupt(m_codec_idx);
        }
    
        if (int_reason & (1<<INT_BIT_PIC_RUN))  {
            MMPDEBUGMSG(VDEC_DEBUG, (TEXT("[CMmpDecoderVpuIF::DecodeAu_PinEnd] INT_BIT_PIC_RUN 0x%08x "), int_reason ));
		    break;		
        }
    }

	#if 1
    {
	    PhysicalAddress rdPtr, wrPtr;
	    MMP_S32 room;
	    m_p_vpu_if->VPU_DecGetBitstreamBuffer(m_DecHandle, &rdPtr, &wrPtr, (int*)&room);
	    MMPDEBUGMSG(VDEC_DEBUG, (TEXT("Post Read ptr = 0x%08x | Write ptr = 0x%08x"), rdPtr, wrPtr));	
    }	
	#endif
	
    m_last_int_reason = int_reason;

	
    t3 = CMmpUtil::GetTickCount();
    
    if(mmpResult == MMP_SUCCESS) {

        memset(&m_output_info, 0x00, sizeof(m_output_info));
        vpu_ret = m_p_vpu_if->VPU_DecGetOutputInfo(m_DecHandle, &m_output_info);
        
        t4 = CMmpUtil::GetTickCount();

        MMPDEBUGMSG(VDEC_DEBUG, (TEXT("[CMmpDecoderVpuIF::DecodeAu_PinEnd] ln=%d dur=(%d %d %d %d) indexFrameDisplay(%d) interlace(%d) pict(%d %d) success(%d) topFieldFirst(%d)"), 
                                                            __LINE__, 
                                                            (t1-start_tick),(t2-start_tick),(t3-start_tick),(t4-start_tick), 
                                                            m_output_info.indexFrameDisplay,
                                                            m_output_info.interlacedFrame, 
                                                            m_output_info.picType, 
                                                            m_output_info.picTypeFirst, 
                                                            m_output_info.decodingSuccess,
                                                            m_output_info.topFieldFirst
                                                            ));
        if( (vpu_ret == RETCODE_SUCCESS) 
            && (m_output_info.picType<PIC_TYPE_MAX)
            ) 
        {
            if(m_output_info.indexFrameDisplay >= 0) {
            
                FrameBuffer frameBuf;
                m_p_vpu_if->VPU_DecGetFrameBuffer(m_DecHandle, m_output_info.indexFrameDisplay, &frameBuf);

                
                if(pp_buf_vf != NULL) {
                    *pp_buf_vf = m_p_buf_vf_arr[m_output_info.indexFrameDisplay];
                }
                
                m_p_vpu_if->VPU_DecClrDispFlag(m_DecHandle, m_output_info.indexFrameDisplay);
            }
            else {
                MMPDEBUGMSG(VDEC_DEBUG, (TEXT("[CMmpDecoderVpuIF::DecodeAu_PinEnd] ln=%d m_output_info.indexFrameDisplay=%d "), __LINE__, m_output_info.indexFrameDisplay));
            }
            
            
        }
        else {
            mmpResult = MMP_FAILURE;
            MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpDecoderVpuIF::DecodeAu_PinEnd] ln=%d FAIL: m_p_vpu_if->VPU_DecGetOutputInfo "), __LINE__));
        }
		
		if (m_output_info.numOfErrMBs) 
		{			
			MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpDecoderVpuIF::DecodeAu_PinEnd] Num of Error Mbs : %d "), m_output_info.numOfErrMBs));
		}	

		if(m_reUseChunk) // mpeg4 chunk reuse process
		{
			m_reUseChunk = 0;
			mmpResult = MMP_SUCCESS;
		    goto CHUNK_REUSE;
		}			

        if(m_output_info.chunkReuseRequired) 
        {
        	m_reUseChunk = 1;	
			MMPDEBUGMSG(0, (TEXT("[CMmpDecoderVpuIF::DecodeAu_PinEnd] m_output_info.chunkReuseRequired ")));
        }			
	
    }
    else
    {
		if(m_reUseChunk) // field interrupt process
		{
			m_reUseChunk = 0;
			mmpResult = MMP_SUCCESS;
		    goto CHUNK_REUSE;
		}	
    }
	
    m_input_stream_count++;
    MMPDEBUGMSG(VDEC_DEBUG, (TEXT("[CMmpDecoderVpuIF::DecodeAu_PinEnd] function end  m_reUseChunk =%d\n"),m_reUseChunk));
    return mmpResult;
}

MMP_RESULT CMmpDecoderVpuIF::DecodeAu_PinEnd_Run(class mmp_buffer_videostream* p_buf_vs) {

    MMP_RESULT mmpResult = MMP_SUCCESS;
    RetCode vpu_ret;
    DecParam		decParam	= {0};
    MMP_S32 size;
    MMP_U32 start_tick, t1, t2;
    MMP_U8* p_stream;
    MMP_S32 stream_size;
    
    class mmp_lock autolock((class mmp_oal_lock*)m_p_vpu_if->get_external_mutex());

    start_tick = CMmpUtil::GetTickCount();

    decParam.iframeSearchEnable = 0;
    decParam.skipframeMode = m_skipframeMode;
    //decParam.skipframeMode = 0;
    decParam.DecStdParam.mp2PicFlush = 0;
    p_stream = (MMP_U8*)p_buf_vs->get_buf_vir_addr();
    stream_size = p_buf_vs->get_stream_size();

    MMPDEBUGMSG(VDEC_DEBUG, (TEXT("\n[CMmpDecoderVpuIF::DecodeAu_PinEnd_Run] ln=%d cnt=%d sz=%d (%02x %02x %02x %02x, %02x %02x %02x %02x, %02x %02x %02x %02x, %02x %02x %02x %02x ) "), 
                  __LINE__, m_input_stream_count, stream_size,
                   p_stream[0], p_stream[1], p_stream[2], p_stream[3], 
                   p_stream[4], p_stream[5], p_stream[6], p_stream[7], 
                   p_stream[8], p_stream[9], p_stream[10], p_stream[11], 
                   p_stream[12], p_stream[13], p_stream[14], p_stream[15] 
    ));

    /* check header */
    switch(m_fourcc_in) {

        case MMP_FOURCC_VIDEO_H264: 
            mmpResult = this->make_frameheader_H264(p_buf_vs);
            break;

        case MMP_FOURCC_VIDEO_WMV3:
            mmpResult = this->make_frameheader_VC1(p_buf_vs);
            break;

        case MMP_FOURCC_VIDEO_MSMPEG4V2:
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


    
#if 1
    m_reUseChunk = 0;
#endif

	if(m_input_stream_count) // AVC1 format sequence after pps data reuse
        m_p_vpu_if->VPU_DecSetRdPtr(m_DecHandle, m_decOP.bitstreamBuffer, 1);	
		
	MMPDEBUGMSG(VDEC_DEBUG, (TEXT("--------- NormalProcess ---------- "))); 
       
    if(mmpResult == MMP_SUCCESS) {
    
        if(p_buf_vs->get_header_size() > 0) {
            size = m_p_vpu_if->WriteBsBufFromBufHelper(m_codec_idx, m_DecHandle, 
                                                        &m_vpu_stream_buffer, 
                                                        (BYTE*)p_buf_vs->get_header_buffer(), p_buf_vs->get_header_size(), 
                                                        m_decOP.streamEndian);
            if (size <0)
    	    {
    		    mmpResult = MMP_FAILURE;
                MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpDecoderVpuIF::DecodeAu_PinEnd_Run] FAIL: WriteBsBufFromBufHelper")));
    	    }
        }
			
        if(m_fourcc_in != MMP_FOURCC_VIDEO_THEORA)
        {
	        if(mmpResult == MMP_SUCCESS) {
	            size = m_p_vpu_if->WriteBsBufFromBufHelper(m_codec_idx, m_DecHandle, 
	                                                        &m_vpu_stream_buffer, 
	                                                        p_buf_vs->get_stream_real_ptr(), p_buf_vs->get_stream_real_size(), 
	                                                        m_decOP.streamEndian);
	            if (size <0)
	    	    {
	    		    mmpResult = MMP_FAILURE;
	                MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpDecoderVpuIF::DecodeAu_PinEnd] FAIL: WriteBsBufFromBufHelper")));
	    	    }
            }
        }
			
    }
    else {
        MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpDecoderVpuIF::DecodeAu_PinEnd] FAIL: make frame header")));
    }
        
	
    t1 = CMmpUtil::GetTickCount();
    #if 1
	{
		PhysicalAddress rdPtr, wrPtr;
		MMP_S32 room;
		m_p_vpu_if->VPU_DecGetBitstreamBuffer(m_DecHandle, &rdPtr, &wrPtr, (int*)&room);
		MMPDEBUGMSG(0, (TEXT("Pre Read ptr = 0x%08x | Write ptr = 0x%08x"), rdPtr, wrPtr));	
	}	
	#endif

	
    if(mmpResult == MMP_SUCCESS)  {

        if( (m_last_int_reason&(1<<INT_BIT_DEC_FIELD)) != 0) {
           m_p_vpu_if->VPU_ClearInterrupt(m_codec_idx);
           MMPDEBUGMSG(0, (TEXT("[second] INT_BIT_DEC_FIELD")));
        }
        else {

	        // Start decoding a frame.
START_DEC:            
	        vpu_ret = m_p_vpu_if->VPU_DecStartOneFrame(m_DecHandle, &decParam);
		    MMPDEBUGMSG(0, (TEXT("[first] VPU_DecStartOneFrame start")));
			if(vpu_ret == RETCODE_FRAME_NOT_COMPLETE)
			{
				 //MMPDEBUGMSG(0, (TEXT("[CMmpDecoderVpuIF::DecodeAu_PinEnd] wait instance")));
				 //printf(".");
				 //CMmpUtil::Sleep(1000);
				 goto START_DEC;
			}			
	        if (vpu_ret != RETCODE_SUCCESS) 
	        {
		        mmpResult = MMP_FAILURE;
                MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpDecoderVpuIF::DecodeAu_PinEnd] ln=%d FAIL: m_p_vpu_if->VPU_DecStartOneFrame vpu_ret=%d "), __LINE__, vpu_ret));
	        }
        }
    }

    t2 = CMmpUtil::GetTickCount();

    return MMP_SUCCESS;
    
}
    
MMP_RESULT CMmpDecoderVpuIF::DecodeAu_PinEnd_WaitDone(class mmp_buffer_videoframe** pp_buf_vf) {

    MMP_RESULT mmpResult = MMP_SUCCESS;
    RetCode vpu_ret;
    DecParam		decParam	= {0};
    MMP_S32 int_reason;
    MMP_U32  t2, t3 ,t4;
    
    class mmp_lock autolock((class mmp_oal_lock*)m_p_vpu_if->get_external_mutex());


    t2 = CMmpUtil::GetTickCount();

    int_reason = 0;
    while(mmpResult == MMP_SUCCESS) {
    
        int_reason = m_p_vpu_if->VPU_WaitInterrupt(m_codec_idx, VPU_DEC_TIMEOUT);
        if (int_reason == (Uint32)-1) // timeout
		{
            MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpDecoderVpuIF::DecodeAu_PinEnd_WaitDone] ln=%d  FAIL: m_p_vpu_if->VPU_WaitInterrupt   TimeOut "), __LINE__));
			VPU_SWReset(m_codec_idx, SW_RESET_SAFETY, m_DecHandle);				
			mmpResult = MMP_FAILURE;
            int_reason = 0;
            break;
		}		
        
        if (int_reason & (1<<INT_BIT_DEC_FIELD)) {

            PhysicalAddress rdPtr, wrPtr;
            MMP_S32 room, seqHeaderSize;
            MMP_S32 remain_size;
            
            seqHeaderSize = 0;

            m_p_vpu_if->VPU_DecGetBitstreamBuffer(m_DecHandle, &rdPtr, &wrPtr, (int*)&room);

            if(rdPtr <= wrPtr)  {
                remain_size = wrPtr - rdPtr;
            }
            else {
                remain_size = wrPtr-m_decOP.bitstreamBuffer;
                remain_size += m_decOP.bitstreamBufferSize - (rdPtr-m_decOP.bitstreamBuffer);
            }
            
            if(remain_size > 8) //(rdPtr-m_decOP.bitstreamBuffer) < (PhysicalAddress)(chunkSize+picHeaderSize+seqHeaderSize-8))	// there is full frame data in chunk data.
            {
				//m_p_vpu_if->VPU_DecSetRdPtr(m_DecHandle, rdPtr, 0);		//set rdPtr to the position of next field data.
                MMPDEBUGMSG(VDEC_DEBUG, (TEXT("[CMmpDecoderVpuIF::DecodeAu_PinEnd_WaitDone] INT_BIT_DEC_FIELD 0x%08x Wait Next Dec (0x%08x 0x%08x 0x%08x ) remain_size:%d"), int_reason , rdPtr, wrPtr , m_decOP.bitstreamBuffer, remain_size));
            }
            else {
                MMPDEBUGMSG(VDEC_DEBUG, (TEXT("[CMmpDecoderVpuIF::DecodeAu_PinEnd_WaitDone] INT_BIT_DEC_FIELD 0x%08x Wait Next Stream (0x%08x 0x%08x 0x%08x ) remain_size:%d"), int_reason , rdPtr, wrPtr , m_decOP.bitstreamBuffer, remain_size));
                mmpResult = MMP_FAILURE;
                break;
            }
            
        }

        if (int_reason) {
    		m_p_vpu_if->VPU_ClearInterrupt(m_codec_idx);
        }
    
        if (int_reason & (1<<INT_BIT_PIC_RUN))  {
            MMPDEBUGMSG(VDEC_DEBUG, (TEXT("[CMmpDecoderVpuIF::DecodeAu_PinEnd_WaitDone] INT_BIT_PIC_RUN 0x%08x "), int_reason ));
		    break;		
        }
    }

	#if 1
    {
	    PhysicalAddress rdPtr, wrPtr;
	    MMP_S32 room;
	    m_p_vpu_if->VPU_DecGetBitstreamBuffer(m_DecHandle, &rdPtr, &wrPtr, (int*)&room);
	    MMPDEBUGMSG(VDEC_DEBUG, (TEXT("Post Read ptr = 0x%08x | Write ptr = 0x%08x"), rdPtr, wrPtr));	
    }	
	#endif
	
    m_last_int_reason = int_reason;

	
    t3 = CMmpUtil::GetTickCount();
    
    if(mmpResult == MMP_SUCCESS) {

        memset(&m_output_info, 0x00, sizeof(m_output_info));
        vpu_ret = m_p_vpu_if->VPU_DecGetOutputInfo(m_DecHandle, &m_output_info);
        
        t4 = CMmpUtil::GetTickCount();

        MMPDEBUGMSG(VDEC_DEBUG, (TEXT("[CMmpDecoderVpuIF::DecodeAu_PinEnd_WaitDone] ln=%d indexFrameDisplay(%d) interlace(%d) pict(%d %d) success(%d) topFieldFirst(%d)"), 
                                                            __LINE__, 
                                                            m_output_info.indexFrameDisplay,
                                                            m_output_info.interlacedFrame, 
                                                            m_output_info.picType, 
                                                            m_output_info.picTypeFirst, 
                                                            m_output_info.decodingSuccess,
                                                            m_output_info.topFieldFirst
                                                            ));
        if( (vpu_ret == RETCODE_SUCCESS) 
            && (m_output_info.picType<PIC_TYPE_MAX)
            ) 
        {
            if(m_output_info.indexFrameDisplay >= 0) {
            
                FrameBuffer frameBuf;
                m_p_vpu_if->VPU_DecGetFrameBuffer(m_DecHandle, m_output_info.indexFrameDisplay, &frameBuf);

                
                if(pp_buf_vf != NULL) {
                    *pp_buf_vf = m_p_buf_vf_arr[m_output_info.indexFrameDisplay];
                }
                
                m_p_vpu_if->VPU_DecClrDispFlag(m_DecHandle, m_output_info.indexFrameDisplay);
            }
            else {
                MMPDEBUGMSG(VDEC_DEBUG, (TEXT("[CMmpDecoderVpuIF::DecodeAu_PinEnd_WaitDone] ln=%d m_output_info.indexFrameDisplay=%d "), __LINE__, m_output_info.indexFrameDisplay));
            }
            
            
        }
        else {
            mmpResult = MMP_FAILURE;
            MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpDecoderVpuIF::DecodeAu_PinEnd_WaitDone] ln=%d FAIL: m_p_vpu_if->VPU_DecGetOutputInfo "), __LINE__));
        }
		
		if (m_output_info.numOfErrMBs) 
		{			
			MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpDecoderVpuIF::DecodeAu_PinEnd_WaitDone] Num of Error Mbs : %d "), m_output_info.numOfErrMBs));
		}	
        		
        if(m_output_info.chunkReuseRequired) 
        {
        	m_reUseChunk = 1;	
			MMPDEBUGMSG(0, (TEXT("[CMmpDecoderVpuIF::DecodeAu_PinEnd_WaitDone] m_output_info.chunkReuseRequired ")));
        }			
	
    }
    
    m_input_stream_count++;
    MMPDEBUGMSG(VDEC_DEBUG, (TEXT("[CMmpDecoderVpuIF::DecodeAu_PinEnd_WaitDone] function end  m_reUseChunk =%d\n"),m_reUseChunk));
    return mmpResult;
}

void CMmpDecoderVpuIF::make_decOP_Common() {
    
    
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
    MMPDEBUGMSG(1, (TEXT("[CMmpDecoderVpuIF::make_decOP_Common] ln=%d 0x%08x %d"), __LINE__, m_decOP.bitstreamBuffer, m_decOP.bitstreamBufferSize));
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

void CMmpDecoderVpuIF::make_decOP_H263() {

    this->make_decOP_Common();

    m_decOP.bitstreamFormat = STD_H263; //0(H.264) / 1(VC1) / 2(MPEG2) / 3(MPEG4) / 4(H263) / 5(DIVX3) / 6(RV) / 7(AVS) / 11(VP8)
    m_decOP.mp4DeblkEnable = 0;
	m_decOP.mp4Class = 0; //MPEG4 CLASS 0(MPEG4) / 1(DIVX 5.0 or higher) / 2(XVID) / 5(DIVX 4.0) / 8(DIVX/XVID Auto Detect)/ 256(Sorenson spark) :
	
}

void CMmpDecoderVpuIF::make_decOP_H264() {
    
    this->make_decOP_Common();

    m_decOP.bitstreamFormat = STD_AVC; //0(H.264) / 1(VC1) / 2(MPEG2) / 3(MPEG4) / 4(H263) / 5(DIVX3) / 6(RV) / 7(AVS) / 11(VP8)
    m_decOP.avcExtension = 0;  // AVC extension 0(No) / 1(MVC) 
    
	
}

void CMmpDecoderVpuIF::make_decOP_MPEG4(int mp4class) {

    this->make_decOP_Common();

    m_decOP.bitstreamFormat = STD_MPEG4; //0(H.264) / 1(VC1) / 2(MPEG2) / 3(MPEG4) / 4(H263) / 5(DIVX3) / 6(RV) / 7(AVS) / 11(VP8)
    m_decOP.mp4DeblkEnable = 1;
	MMPDEBUGMSG(1, (TEXT("[CMmpDecoderVpuIF::make_decOP_MPEG4] MPEG4 class = %d"),mp4class));	
	m_decOP.mp4Class = mp4class; //MPEG4 CLASS 0(MPEG4) / 1(DIVX 5.0 or higher) / 2(XVID) / 5(DIVX 4.0) / 8(DIVX/XVID Auto Detect)/ 256(Sorenson spark) :
	
}

void CMmpDecoderVpuIF::make_decOP_MPEG2() {

    this->make_decOP_Common();

    m_decOP.bitstreamFormat = STD_MPEG2; //0(H.264) / 1(VC1) / 2(MPEG2) / 3(MPEG4) / 4(H263) / 5(DIVX3) / 6(RV) / 7(AVS) / 11(VP8)
    m_decOP.mp4DeblkEnable = 0;
	m_decOP.mp4Class = 0; //MPEG4 CLASS 0(MPEG4) / 1(DIVX 5.0 or higher) / 2(XVID) / 5(DIVX 4.0) / 8(DIVX/XVID Auto Detect)/ 256(Sorenson spark) :
	
}

void CMmpDecoderVpuIF::make_decOP_VC1() {

    this->make_decOP_Common();

    m_decOP.bitstreamFormat = STD_VC1; //0(H.264) / 1(VC1) / 2(MPEG2) / 3(MPEG4) / 4(H263) / 5(DIVX3) / 6(RV) / 7(AVS) / 11(VP8)
    m_decOP.mp4DeblkEnable = 0;
	m_decOP.mp4Class = 0; //MPEG4 CLASS 0(MPEG4) / 1(DIVX 5.0 or higher) / 2(XVID) / 5(DIVX 4.0) / 8(DIVX/XVID Auto Detect)/ 256(Sorenson spark) :
	
}

void CMmpDecoderVpuIF::make_decOP_MSMpeg4V3() {

    this->make_decOP_Common();

    m_decOP.bitstreamFormat = STD_DIV3; //0(H.264) / 1(VC1) / 2(MPEG2) / 3(MPEG4) / 4(H263) / 5(DIVX3) / 6(RV) / 7(AVS) / 11(VP8)
    m_decOP.mp4DeblkEnable = 0;
	m_decOP.mp4Class = 0; //MPEG4 CLASS 0(MPEG4) / 1(DIVX 5.0 or higher) / 2(XVID) / 5(DIVX 4.0) / 8(DIVX/XVID Auto Detect)/ 256(Sorenson spark) :
	
}

void CMmpDecoderVpuIF::make_decOP_RV30() {

    this->make_decOP_RV40();
}

void CMmpDecoderVpuIF::make_decOP_RV40() {

    this->make_decOP_Common();

    m_decOP.bitstreamFormat = STD_RV; //0(H.264) / 1(VC1) / 2(MPEG2) / 3(MPEG4) / 4(H263) / 5(DIVX3) / 6(RV) / 7(AVS) / 11(VP8)
    m_decOP.mp4DeblkEnable = 0;
	m_decOP.mp4Class = 0; //MPEG4 CLASS 0(MPEG4) / 1(DIVX 5.0 or higher) / 2(XVID) / 5(DIVX 4.0) / 8(DIVX/XVID Auto Detect)/ 256(Sorenson spark) :
	
}

void CMmpDecoderVpuIF::make_decOP_VP80() {

    this->make_decOP_Common();

    m_decOP.bitstreamFormat = STD_VP8; //0(H.264) / 1(VC1) / 2(MPEG2) / 3(MPEG4) / 4(H263) / 5(DIVX3) / 6(RV) / 7(AVS) / 11(VP8)
    m_decOP.mp4DeblkEnable = 0;
	m_decOP.mp4Class = 0; //MPEG4 CLASS 0(MPEG4) / 1(DIVX 5.0 or higher) / 2(XVID) / 5(DIVX 4.0) / 8(DIVX/XVID Auto Detect)/ 256(Sorenson spark) :
	
}

#if (VPU_SUPPORT_THEORA == 1)
void CMmpDecoderVpuIF::make_decOP_Theora() {

    this->make_decOP_Common();

    m_decOP.bitstreamFormat = STD_THO; //0(H.264) / 1(VC1) / 2(MPEG2) / 3(MPEG4) / 4(H263) / 5(DIVX3) / 6(RV) / 7(AVS) / 11(VP8)
    m_decOP.mp4DeblkEnable = 0;
	m_decOP.mp4Class = 0; //MPEG4 CLASS 0(MPEG4) / 1(DIVX 5.0 or higher) / 2(XVID) / 5(DIVX 4.0) / 8(DIVX/XVID Auto Detect)/ 256(Sorenson spark) :

	theora_parser_init((void **)&m_thoParser); // theora parser init	
	
}
#endif

MMP_RESULT CMmpDecoderVpuIF::make_seqheader_Common(class mmp_buffer_videostream* p_buf_vs) {

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

MMP_RESULT CMmpDecoderVpuIF::make_seqheader_H264(class mmp_buffer_videostream* p_buf_vs) {

    MMP_RESULT mmpResult;
    MMP_U8 *p_avc_dsi, *p_h264_dsi;
    MMP_S32 avc_dsi_size, h264_dsi_size;

    p_avc_dsi = (MMP_U8*)p_buf_vs->get_buf_vir_addr();
    avc_dsi_size = p_buf_vs->get_stream_size();

    mmpResult = p_buf_vs->alloc_dsi_buffer(128+avc_dsi_size);
    if(mmpResult == MMP_SUCCESS) {

        p_h264_dsi = (MMP_U8*)p_buf_vs->get_dsi_buffer();
        mmpResult = CMmpH264Parser::ConvertDSI_AVC1_To_H264(p_avc_dsi, avc_dsi_size, p_h264_dsi, &h264_dsi_size);
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

MMP_RESULT CMmpDecoderVpuIF::make_seqheader_VC1(class mmp_buffer_videostream* p_buf_vs) {

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
    }

    return mmpResult;
}


MMP_RESULT CMmpDecoderVpuIF::make_seqheader_DIV3(class mmp_buffer_videostream* p_buf_vs) {

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

MMP_RESULT CMmpDecoderVpuIF::make_seqheader_RV(class mmp_buffer_videostream* p_buf_vs) {

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

MMP_RESULT CMmpDecoderVpuIF::make_seqheader_VP8(class mmp_buffer_videostream* p_buf_vs) {

    MMP_RESULT mmpResult = MMP_SUCCESS;
    MMP_S32 stream_size;
    MMP_S32 w, h, framerate;
    
    MMP_U8 *pbHeader, *pstream;
    MMP_S32 header_size;

    pstream = (MMP_U8*)p_buf_vs->get_buf_vir_addr();
    stream_size = p_buf_vs->get_stream_size();

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
        p_buf_vs->set_stream_offset(0);
    }

    return mmpResult;
}

#if (VPU_SUPPORT_THEORA == 1)
MMP_RESULT CMmpDecoderVpuIF::make_seqheader_Theora(class mmp_buffer_videostream* p_buf_vs) {

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

MMP_RESULT CMmpDecoderVpuIF::make_frameheader_Common(class mmp_buffer_videostream* p_buf_vs) {

    p_buf_vs->set_header_size(0);
    p_buf_vs->set_stream_offset(0);
    return MMP_SUCCESS;
}

MMP_RESULT CMmpDecoderVpuIF::make_frameheader_H264(class mmp_buffer_videostream* p_buf_vs) {

    //this->DecodeAu_StreamRemake_AVC1((MMP_U8*)p_buf_vs->get_buf_vir_addr(), p_buf_vs->get_stream_size()); 
    
    MMP_S32 remain_size, stream_size;
    MMP_U32* ps;
    MMP_U8* pau;

    pau = (MMP_U8*)p_buf_vs->get_buf_vir_addr();
    remain_size = p_buf_vs->get_stream_size();

    while(remain_size > 0) {

        ps = (MMP_U32*)pau;
        stream_size = *ps;
        if(stream_size == 0x01000000) {
            break;
        }
        stream_size = MMP_SWAP_U32(stream_size);
        stream_size += 4;
        *ps = 0x01000000;
        pau+=stream_size;
        remain_size -= stream_size;
    }

    p_buf_vs->set_header_size(0);
    p_buf_vs->set_stream_offset(0);
        
    return MMP_SUCCESS;
}

MMP_RESULT CMmpDecoderVpuIF::make_frameheader_VC1(class mmp_buffer_videostream* p_buf_vs) {

    MMP_RESULT mmpResult = MMP_FAILURE;
    MMP_U8* p_header; 
    MMP_S32 stream_size;
    MMP_S64 pts;
    MMP_U32 flag;

    mmpResult = p_buf_vs->alloc_header_buffer(8);
    if(mmpResult == MMP_SUCCESS) {
        
        p_header = (MMP_U8*)p_buf_vs->get_header_buffer();
        pts = p_buf_vs->get_pts();
        stream_size = p_buf_vs->get_stream_size();
        flag = p_buf_vs->get_flag();

        if((flag&mmp_buffer_media::FLAG_VIDEO_KEYFRAME) != 0) {
            MMP_PUT_LE32(p_header,  stream_size | 0x80000000 );
        }
        else {
            MMP_PUT_LE32(p_header, stream_size | 0x00000000 );
        }

        MMP_PUT_LE32(p_header, pts/1000); /* milli_sec */

        p_buf_vs->set_header_size(8);
        p_buf_vs->set_stream_offset(0);
    }

    return MMP_SUCCESS;
}

MMP_RESULT CMmpDecoderVpuIF::make_frameheader_DIV3(class mmp_buffer_videostream* p_buf_vs) {

    MMP_RESULT mmpResult = MMP_FAILURE;
    MMP_U8* p_header; 
    MMP_S32 stream_size;
    
    mmpResult = p_buf_vs->alloc_header_buffer(12);
    if(mmpResult == MMP_SUCCESS) {
        
        p_header = (MMP_U8*)p_buf_vs->get_header_buffer();
        stream_size = p_buf_vs->get_stream_size();

        MMP_PUT_LE32(p_header, stream_size);
        MMP_PUT_LE32(p_header, 0);
        MMP_PUT_LE32(p_header, 0);

        p_buf_vs->set_header_size(12);
        p_buf_vs->set_stream_offset(0);
    }

    return MMP_SUCCESS;
}

MMP_RESULT CMmpDecoderVpuIF::make_frameheader_RV(class mmp_buffer_videostream* p_buf_vs) {

    MMP_RESULT mmpResult = MMP_FAILURE;
    MMP_U8 *p_header, *p_stream; 
    MMP_S32 stream_size, header_size;
    MMP_S32 cSlice, nSlice, offset, i;
    MMP_U32 val;

    p_stream = (MMP_U8*)p_buf_vs->get_buf_vir_addr();
    stream_size = p_buf_vs->get_stream_size();

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
        p_buf_vs->set_stream_offset(1+(cSlice*8));
    }

    return MMP_SUCCESS;
}

MMP_RESULT CMmpDecoderVpuIF::make_frameheader_VP8(class mmp_buffer_videostream* p_buf_vs) {

    MMP_RESULT mmpResult = MMP_FAILURE;
    MMP_U8* p_header; 
    MMP_S32 stream_size;
    
    mmpResult = p_buf_vs->alloc_header_buffer(12);
    if(mmpResult == MMP_SUCCESS) {
        
        p_header = (MMP_U8*)p_buf_vs->get_header_buffer();
        stream_size = p_buf_vs->get_stream_size();

        MMP_PUT_LE32(p_header, stream_size);
        MMP_PUT_LE32(p_header, 0);
        MMP_PUT_LE32(p_header, 0);

        p_buf_vs->set_header_size(12);
        p_buf_vs->set_stream_offset(0);
    }

    return MMP_SUCCESS;
}

#if (VPU_SUPPORT_THEORA == 1)
MMP_RESULT CMmpDecoderVpuIF::make_frameheader_Theora(class mmp_buffer_videostream* p_buf_vs) {

    MMP_RESULT mmpResult = MMP_FAILURE;
    MMP_U8* p_header; 
	MMP_U8* p_stream;
    MMP_S32 stream_size;
    int ret = 0;
	
    p_stream = (MMP_U8*)p_buf_vs->get_buf_vir_addr();
    stream_size = p_buf_vs->get_stream_size();	

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
        p_buf_vs->set_stream_offset(0);		
	}
	
    return MMP_SUCCESS;
}
#endif
