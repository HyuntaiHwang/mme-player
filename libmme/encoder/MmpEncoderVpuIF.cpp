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

#include "MmpEncoderVpuIF.hpp"
#include "MmpUtil.hpp"
#include "vpuhelper.h"
#include "MmpH264Tool.hpp"
#include "mmp_buffer_mgr.hpp"
#include "mmp_lock.hpp"
#include "MmpImageTool.hpp"
#include "mmp_oal_mutex.hpp"

#define VPU_ENC_TIMEOUT       1000
#define VPU_DEC_TIMEOUT       1000
#define VPU_WAIT_TIME_OUT	 100		//should be less than normal decoding time to give a chance to fill stream. if this value happens some problem. we should fix VPU_WaitInterrupt function

#define STREAM_BUF_SIZE		 0x300000  // max bitstream size


/////////////////////////////////////////////////////////////
//CMmpEncoderVpuIF Member Functions

CMmpEncoderVpuIF::CMmpEncoderVpuIF(MMP_FOURCC fourcc_in, MMP_FOURCC fourcc_out, MMP_S32 pic_width, MMP_S32 pic_height) :

m_fourcc_in(fourcc_in)
,m_fourcc_out(fourcc_out)

,m_pic_width(pic_width)
,m_pic_height(pic_height)

,m_codec_idx(0)
,m_version(0)
,m_revision(0)
,m_productId(0)

,m_mapType(LINEAR_FRAME_MAP) //Map Type 0(Linear) / 1(Frame-V) / 2(Frame-H) / 3(Field-V) / 4(Mix-V) / 5(Frame-MB) / 6(Filed-MB) / 16(Tile2Linear)
,m_regFrameBufCount(0)

,m_EncHandle(NULL)

,m_p_enc_buffer(NULL)
,m_p_src_frame_buffer(NULL)

,m_is_streamming_on(MMP_FALSE)

,m_is_cbcr_interleave(0)
,m_DEBUG_VPUEnc(MMP_FALSE)
,m_encoed_frame_count(0)
{
    MMP_S32 i;

    memset(&m_encOP, 0x00, sizeof(m_encOP));
    memset(&m_enc_init_info, 0x00, sizeof(m_enc_init_info));

    for(i = 0; i < MAX_FRAMEBUFFER_COUNT; i++) {
        m_p_frame_buffer[i] = NULL;
    }
        
    m_DEBUG_VPUEnc = CMmpUtil::IsDebugEnable_VPUEnc();
}

CMmpEncoderVpuIF::~CMmpEncoderVpuIF()
{
    
}

MMP_RESULT CMmpEncoderVpuIF::Open(class mmp_vpu_if *p_vpu_if, class mmp_buffer_videoframe* p_buf_vf)
{
    RetCode vpu_ret;
    MMP_RESULT mmpResult = MMP_SUCCESS;

    if( (p_buf_vf->get_fourcc()  == MMP_FOURCC_IMAGE_NV12)
        || (p_buf_vf->get_fourcc()  == MMP_FOURCC_IMAGE_NV21) ) {
        m_is_cbcr_interleave = 1;
    }

    if(mmpResult == MMP_SUCCESS) {
    
        p_vpu_if->VPU_GetVersionInfo(m_codec_idx, &m_version, &m_revision, &m_productId);	

        MMPDEBUGMSG(MMPZONE_INFO, (TEXT("[VPUEnc] VPU coreNum : [%d]\n"), m_codec_idx));
        MMPDEBUGMSG(MMPZONE_INFO, (TEXT("[VPUEnc] Firmware Version => projectId : %x | version : %04d.%04d.%08d | revision : r%d\n"), 
                    (Uint32)(m_version>>16), (Uint32)((m_version>>(12))&0x0f), (Uint32)((m_version>>(8))&0x0f), (Uint32)((m_version)&0xff), m_revision));
        MMPDEBUGMSG(MMPZONE_INFO, (TEXT("[VPUEnc]  Hardware Version => %04x  API Version => %04x\n"), m_productId, API_VERSION));
        MMPDEBUGMSG(MMPZONE_INFO, (TEXT("[VPUEnc]  foucc_in=%c%c%c%c fourcc_out=%c%c%c%c pic_width=%d pic_height=%d  m_is_cbcr_interleave=%d \n"),
                                  MMPGETFOURCCARG(m_fourcc_in),  MMPGETFOURCCARG(m_fourcc_out),
                                  m_pic_width, m_pic_height, 
                                  m_is_cbcr_interleave
                               ));
    }

    /* alloc dma buffer */
    if(mmpResult == MMP_SUCCESS) {
        
#if 1
        m_p_enc_buffer = mmp_buffer_mgr::get_instance()->alloc_dma_buffer(STREAM_BUF_SIZE);
        if(m_p_enc_buffer == NULL) {
            MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpEncoderVpuIF::Open] FAIL :  alloc stream buffer\n")));
            mmpResult = MMP_FAILURE;
        }
        else {

            class mmp_buffer_addr buf_addr;

            buf_addr = this->m_p_enc_buffer->get_buf_addr();
            m_vpu_enc_buffer.base = buf_addr.m_vir_addr;
            m_vpu_enc_buffer.phys_addr = buf_addr.m_phy_addr;
            m_vpu_enc_buffer.size = buf_addr.m_size;
            m_vpu_enc_buffer.virt_addr = buf_addr.m_vir_addr;

            p_vpu_if->vdi_register_dma_memory(m_codec_idx, &m_vpu_enc_buffer);
        }
#else
        m_vbStream.size	 = STREAM_BUF_SIZE;
        iret = vdi_allocate_dma_memory(m_codec_idx, &m_vbStream);
	    if(iret < 0)  {
            m_vbStream.base = 0;
		    MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpEncoderVpuIF::Open] FAIL :  vdi_allocate_dma_memory (iret=%d) \n"), iret));
            mmpResult = MMP_FAILURE;
	    }
#endif

    }

    /* Decoder Open */
    if(mmpResult == MMP_SUCCESS) {

        switch(this->m_fourcc_out) {
    
            /* Video */
            case MMP_FOURCC_VIDEO_H263: this->make_encOP_H263(); break;
            case MMP_FOURCC_VIDEO_H264: this->make_encOP_H264(); break;
            case MMP_FOURCC_VIDEO_MPEG4: this->make_encOP_MPEG4(); break;
            default:  
                mmpResult = MMP_FAILURE;
                MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpEncoderVpuIF::Open] FAIL :  Not Suport FouccOut \n")));
                break;
        }

        if(mmpResult == MMP_SUCCESS) {
            vpu_ret = p_vpu_if->VPU_EncOpen(&m_EncHandle, &m_encOP);
	        if( vpu_ret != RETCODE_SUCCESS ) {
		        MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpEncoderVpuIF::Open] FAIL :  p_vpu_if->VPU_EncOpen (vpu_ret=%d) \n"), vpu_ret));
                mmpResult = MMP_FAILURE;
	        }
            else {
               // ret = p_vpu_if->VPU_DecGiveCommand(m_EncHandle, GET_DRAM_CONFIG, &dramCfg);
	           // if( ret != RETCODE_SUCCESS ) {
		       //     VLOG(ERR, "p_vpu_if->VPU_DecGiveCommand[GET_DRAM_CONFIG] failed Error code is 0x%x \n", ret );
		       //     goto ERR_DEC_OPEN;
	           // }

            
                SecAxiUse		secAxiUse;
                memset(&secAxiUse, 0x00, sizeof(SecAxiUse));

                secAxiUse.useBitEnable = USE_BIT_INTERNAL_BUF;
	            secAxiUse.useIpEnable = USE_IP_INTERNAL_BUF;
	            secAxiUse.useDbkYEnable = USE_DBKY_INTERNAL_BUF;
	            secAxiUse.useDbkCEnable = USE_DBKC_INTERNAL_BUF;
	            secAxiUse.useBtpEnable = USE_BTP_INTERNAL_BUF;
	            secAxiUse.useOvlEnable = USE_OVL_INTERNAL_BUF;
	            p_vpu_if->VPU_EncGiveCommand(m_EncHandle, SET_SEC_AXI, &secAxiUse);
            }
        }
        
	}
        
    return mmpResult;
}


MMP_RESULT CMmpEncoderVpuIF::Close(class mmp_vpu_if *p_vpu_if)
{
    MMP_S32 i;

    if(m_EncHandle != NULL) {

        if(m_p_enc_buffer != NULL) {
           p_vpu_if->vdi_unregister_dma_memory(m_codec_idx, &m_vpu_enc_buffer);
        }

        if(m_p_src_frame_buffer != NULL) {
            p_vpu_if->vdi_unregister_dma_memory(m_codec_idx, &m_vpu_src_frame_buffer);
        }

	    p_vpu_if->VPU_EncClose(m_EncHandle);
        m_EncHandle = NULL;

        
    }

    for(i = 0; i < MAX_FRAMEBUFFER_COUNT; i++) {
        if(m_p_frame_buffer[i] != NULL) {
            mmp_buffer_mgr::get_instance()->free_buffer(m_p_frame_buffer[i]);
            m_p_frame_buffer[i] = NULL;
        }
    }

    if(m_p_src_frame_buffer != NULL) {
        mmp_buffer_mgr::get_instance()->free_buffer(m_p_src_frame_buffer);
        m_p_src_frame_buffer = NULL;
    }

    if(m_p_enc_buffer != NULL) {
       mmp_buffer_mgr::get_instance()->free_buffer(m_p_enc_buffer);
       m_p_enc_buffer = NULL;
    }
        
    return MMP_SUCCESS;
}

MMP_RESULT CMmpEncoderVpuIF::EncodeDSI(class mmp_vpu_if *p_vpu_if, class mmp_buffer_videoframe* p_buf_vf) {

    MMP_RESULT mmpResult = MMP_SUCCESS;
    TiledMapConfig mapCfg;
    RetCode ret;
    MMP_S32 i;
    
    //srcFrameWidth = ((encOP.picWidth+15)&~15);
	//srcFrameStride = srcFrameWidth;
	//srcFrameFormat = FORMAT_420;
	//framebufFormat = FORMAT_420;
	//srcFrameHeight = ((encOP.picHeight+15)&~15);
    m_framebufWidth = MMP_BYTE_ALIGN(m_encOP.picWidth, 32); 
	m_framebufHeight = MMP_BYTE_ALIGN(m_encOP.picHeight, 16);
    m_framebufStride = MMP_BYTE_ALIGN(m_framebufWidth, 32);

    MMPDEBUGMSG(m_DEBUG_VPUEnc, (TEXT("[CMmpDecoderVpuIF::EncodeDSI] m_framebufWidth=%d m_framebufHeight=%d  m_framebufStride=%d "), m_framebufWidth, m_framebufHeight, m_framebufStride ));

    /* Enc Seq Init */
    if(mmpResult == MMP_SUCCESS) {
        ret = p_vpu_if->VPU_EncGetInitialInfo(m_EncHandle, &m_enc_init_info);
	    if( ret != RETCODE_SUCCESS )
	    {
		    MMPDEBUGMSG(1, (TEXT("[CMmpDecoderVpuIF::EncodeDSI] FAIL: p_vpu_if->VPU_EncGetInitialInfo")));
		    mmpResult = MMP_FAILURE;
	    }
        else {
	        p_vpu_if->VPU_ClearInterrupt(m_codec_idx);
    	    m_regFrameBufCount = m_enc_init_info.minFrameBufferCount;
        }
    }

    /* Register YUV FrameBuffer */
    if(mmpResult == MMP_SUCCESS) {
        // MaverickCache configure
        //encConfig.frameCacheBypass   = 0;
	    //encConfig.frameCacheBurst    = 0;
	    //encConfig.frameCacheMerge    = 3;
	    //encConfig.frameCacheWayShape = 15;			
        MaverickCache2Config(
		    &m_encCacheConfig, 
		    0, //encoder
		    m_encOP.cbcrInterleave, // cb cr interleave
		    0,//m_encConfig.frameCacheBypass,
		    0,//m_encConfig.frameCacheBurst,
		    3,//m_encConfig.frameCacheMerge,
		    m_mapType,
		    15//encConfig.frameCacheWayShape
            );
	    p_vpu_if->VPU_EncGiveCommand(m_EncHandle, SET_CACHE_CONFIG, &m_encCacheConfig);

        FrameBuffer user_frame[MAX_FRAMEBUFFER_COUNT];//*pUserFrame = NULL;
        class mmp_buffer_addr buf_addr;
        
        for(i = 0; i < m_regFrameBufCount; i++) {
            m_p_frame_buffer[i] = mmp_buffer_mgr::get_instance()->alloc_dma_buffer(m_framebufStride*m_framebufHeight*3/2);
            if(m_p_frame_buffer[i] == NULL) {
                MMPDEBUGMSG(1, (TEXT("[CMmpDecoderVpuIF::EncodeDSI] FAIL: alloc_dma_buffer (FRAME BUF) (%d/%d) sz=%d"), i, m_regFrameBufCount, m_framebufStride*m_framebufHeight*3/2 ));
                break;
            }
            else {

                buf_addr = m_p_frame_buffer[i]->get_buf_addr();

                memset(&user_frame[i], 0x00, sizeof(FrameBuffer));
                user_frame[i].bufY = buf_addr.m_phy_addr;
                if(m_is_cbcr_interleave ==0) {
                    user_frame[i].bufCb = buf_addr.m_phy_addr + m_framebufStride*m_framebufHeight;
                    user_frame[i].bufCr = buf_addr.m_phy_addr + m_framebufStride*m_framebufHeight + m_framebufStride*m_framebufHeight/4;
                }
                else {
                    user_frame[i].bufCb = buf_addr.m_phy_addr + m_framebufStride*m_framebufHeight;
                    user_frame[i].bufCr = user_frame[i].bufCb;
                }
                user_frame[i].mapType = m_mapType;
                user_frame[i].stride = m_framebufStride;
                user_frame[i].height = m_framebufHeight;
                user_frame[i].myIndex = i;
                user_frame[i].cbcrInterleave = m_is_cbcr_interleave;
                
            }
        }
        
        if(i == m_regFrameBufCount) {
            ret = p_vpu_if->VPU_EncRegisterFrameBuffer(m_EncHandle, user_frame, m_regFrameBufCount, m_framebufStride, m_framebufHeight, m_mapType);
	        if( ret != RETCODE_SUCCESS )
	        {
		        MMPDEBUGMSG(1, (TEXT("[CMmpDecoderVpuIF::EncodeDSI] FAIL: p_vpu_if->VPU_EncRegisterFrameBuffer")));
		        mmpResult = MMP_FAILURE;
	        }
            else {
	            p_vpu_if->VPU_EncGiveCommand(m_EncHandle, GET_TILEDMAP_CONFIG, &mapCfg);
            }
        }
        else {
            mmpResult = MMP_FAILURE;
        }
        
    }

    /* Allocate DMA Buffer */
    if(mmpResult == MMP_SUCCESS) {
        m_p_src_frame_buffer = mmp_buffer_mgr::get_instance()->alloc_dma_buffer(m_framebufStride*m_framebufHeight*3/2);
        if(m_p_src_frame_buffer == NULL) {
            MMPDEBUGMSG(1, (TEXT("[CMmpDecoderVpuIF::EncodeDSI] FAIL: alloc_dma_buffer (SRC FRAME BUF)")));
            mmpResult = MMP_FAILURE;
        }
        else {
            class mmp_buffer_addr buf_addr;

            buf_addr = this->m_p_src_frame_buffer->get_buf_addr();
            m_vpu_src_frame_buffer.base = buf_addr.m_vir_addr;
            m_vpu_src_frame_buffer.phys_addr = buf_addr.m_phy_addr;
            m_vpu_src_frame_buffer.size = buf_addr.m_size;
            m_vpu_src_frame_buffer.virt_addr = buf_addr.m_vir_addr;

            p_vpu_if->vdi_register_dma_memory(m_codec_idx, &m_vpu_src_frame_buffer);
        }
    }

    EncHeaderParam encHeaderParam;
    MMP_U8* p_dsi = m_DSI;
    MMP_S32 dsi_size = 0;

    memset(&encHeaderParam, 0x00, sizeof(encHeaderParam));
    
    switch(m_encOP.bitstreamFormat) {
    
        case STD_MPEG4:

            encHeaderParam.buf = m_vpu_enc_buffer.phys_addr; //m_vbStream.phys_addr;
		    encHeaderParam.headerType = VOS_HEADER;
            encHeaderParam.size = m_vpu_enc_buffer.size;//m_vbStream.size;
            ret = p_vpu_if->VPU_EncGiveCommand(m_EncHandle, ENC_PUT_VIDEO_HEADER, &encHeaderParam);
		    if (ret != RETCODE_SUCCESS)
		    {
			    //VLOG(ERR, "p_vpu_if->VPU_EncGiveCommand ( ENC_PUT_VIDEO_HEADER ) for VOS_HEADER failed Error code is 0x%x \n", ret);			
			    mmpResult = MMP_FAILURE; //goto ERR_ENC_OPEN;
		    }
            else {
                vdi_read_memory(m_codec_idx, encHeaderParam.buf, p_dsi, encHeaderParam.size, m_encOP.streamEndian);
                p_dsi+=encHeaderParam.size;
                dsi_size+=encHeaderParam.size;
			}

            encHeaderParam.headerType = VOL_HEADER;
            encHeaderParam.size = m_vpu_enc_buffer.size; //m_vbStream.size;
		    ret = p_vpu_if->VPU_EncGiveCommand(m_EncHandle, ENC_PUT_VIDEO_HEADER, &encHeaderParam); 
		    if (ret != RETCODE_SUCCESS)  {
			    //VLOG(ERR, "p_vpu_if->VPU_EncGiveCommand ( ENC_PUT_VIDEO_HEADER ) for VOL_HEADER failed Error code is 0x%x \n", ret);			
			    mmpResult = MMP_FAILURE; //goto ERR_ENC_OPEN;
		    }
            else {
                vdi_read_memory(m_codec_idx, encHeaderParam.buf, p_dsi, encHeaderParam.size, m_encOP.streamEndian);
                p_dsi+=encHeaderParam.size;
                dsi_size+=encHeaderParam.size;
            }
            break;

        case STD_AVC:
            encHeaderParam.headerType = SPS_RBSP;
            encHeaderParam.buf  = m_vpu_enc_buffer.phys_addr;//m_vbStream.phys_addr;
            encHeaderParam.size = m_vpu_enc_buffer.size;//m_vbStream.size;
            ret = p_vpu_if->VPU_EncGiveCommand(m_EncHandle, ENC_PUT_VIDEO_HEADER, &encHeaderParam); 
            if (ret != RETCODE_SUCCESS){
                mmpResult = MMP_FAILURE; //goto ERR_ENC_OPEN;
            }
            else {
                vdi_read_memory(m_codec_idx, encHeaderParam.buf, p_dsi, encHeaderParam.size, m_encOP.streamEndian);
                p_dsi+=encHeaderParam.size;
                dsi_size+=encHeaderParam.size;
            }
		
            encHeaderParam.headerType = PPS_RBSP;
            encHeaderParam.buf		= m_vpu_enc_buffer.phys_addr;//m_vbStream.phys_addr;
            encHeaderParam.pBuf	   = (BYTE *)m_vpu_enc_buffer.virt_addr;//m_vbStream.virt_addr;
            encHeaderParam.size	   = m_vpu_enc_buffer.size;//m_vbStream.size;
		    ret = p_vpu_if->VPU_EncGiveCommand(m_EncHandle, ENC_PUT_VIDEO_HEADER, &encHeaderParam);
            if (ret != RETCODE_SUCCESS){
                mmpResult = MMP_FAILURE; //goto ERR_ENC_OPEN;
            }
            else {
                vdi_read_memory(m_codec_idx, encHeaderParam.buf, p_dsi, encHeaderParam.size, m_encOP.streamEndian);
                p_dsi+=encHeaderParam.size;
                dsi_size+=encHeaderParam.size;
            }

            break;
    
    }

    if(mmpResult == MMP_SUCCESS) {
        m_DSISize = dsi_size;    
    }

    return mmpResult;
}


MMP_RESULT CMmpEncoderVpuIF::EncodeAu(class mmp_vpu_if *p_vpu_if, class mmp_buffer_videoframe* p_buf_vf, class mmp_buffer_videostream* p_buf_vs) {

    MMP_RESULT mmpResult = MMP_SUCCESS;
    EncParam		encParam;
    FrameBuffer *pFB;
    int int_reason, timeout_count;
    RetCode ret;
    EncOutputInfo	outputInfo;
    MMP_U8* pBuffer;
    MMP_U32 nBufSize, nBufMaxSize;
    enum mmp_buffer_media::FLAG nFlag;

    memset(&encParam, 0x00, sizeof(encParam));
    memset(&outputInfo, 0x00, sizeof(outputInfo));

    m_FrameBuffer_src.bufY = p_buf_vf->get_buf_phy_addr_y();
    m_FrameBuffer_src.bufCb = p_buf_vf->get_buf_phy_addr_u();
    m_FrameBuffer_src.bufCr = p_buf_vf->get_buf_phy_addr_v();
    m_FrameBuffer_src.mapType = m_mapType;
    m_FrameBuffer_src.stride = p_buf_vf->get_stride_luma();
    m_FrameBuffer_src.height = p_buf_vf->get_pic_height();//p_buf_vf->get_buf_height(MMP_YUV420_PLAINE_INDEX_Y);
    m_FrameBuffer_src.myIndex = MAX_FRAMEBUFFER_COUNT;
    m_FrameBuffer_src.sourceLBurstEn = 0;
    m_FrameBuffer_src.cbcrInterleave = m_is_cbcr_interleave;
        
    MMPDEBUGMSG(m_DEBUG_VPUEnc, (TEXT("\n------------ VPU Enc Start ---------------------------- fourcc(%c%c%c%c  %c%c%c%c) Pic(%dx%d) inFrmCnt(%d) crop(%d %d) "), 
                 MMPGETFOURCCARG(m_fourcc_in),
                 MMPGETFOURCCARG(m_fourcc_out),
                 p_buf_vf->get_pic_width(), p_buf_vf->get_pic_height(), m_regFrameBufCount ,
                 m_encOP.EncStdParam.avcParam.frameCroppingFlag,
                 m_encOP.EncStdParam.avcParam.frameCropBottom
                 ));
        
    pFB = &m_FrameBuffer_src;

    /* Enc Start */
    if(mmpResult == MMP_SUCCESS) {

        encParam.forceIPicture = 0;
	    encParam.skipPicture   = 0;
	    encParam.quantParam	   = 10;//encConfig.picQpY;

        encParam.sourceFrame = pFB;
        encParam.picStreamBufferAddr = m_vpu_enc_buffer.phys_addr;//m_vbStream.phys_addr;	// can set the newly allocated buffer.
        encParam.picStreamBufferSize = m_vpu_enc_buffer.size;//m_vbStream.size;

        while(1) {
            MMPDEBUGMSG(m_DEBUG_VPUEnc, (TEXT("[CMmpEncoderVpuIF::EncodeAu] ln=%d  VPU_EncStartOneFrame"),   __LINE__ ));
            ret = p_vpu_if->VPU_EncStartOneFrame(m_EncHandle, &encParam);
		    if(ret != RETCODE_FRAME_NOT_COMPLETE) {
                break;
            }
            else {
                MMPDEBUGMSG(m_DEBUG_VPUEnc, (TEXT("[CMmpEncoderVpuIF::EncodeAu] ln=%d  FAIL: VPU_EncStartOneFrame (ret=RETCODE_FRAME_NOT_COMPLETE)"),   __LINE__ ));
            }
        }
        
	    if( ret != RETCODE_SUCCESS )  {
            MMPDEBUGMSG(1, (TEXT("[CMmpDecoderVpuIF::EncodeAu] FAIL: p_vpu_if->VPU_EncStartOneFrame")));
		    mmpResult = MMP_FAILURE;
	    }
    }
    
    /* Wait Interrupt */
    timeout_count = 0;
	while(mmpResult == MMP_SUCCESS) 
	{
        MMPDEBUGMSG(m_DEBUG_VPUEnc, (TEXT("[CMmpEncoderVpuIF::EncodeAu] ln=%d  VPU_WaitInterrupt"),   __LINE__ ));

		int_reason = p_vpu_if->VPU_WaitInterrupt(m_codec_idx, VPU_WAIT_TIME_OUT);
		if (int_reason == (int)-1)	{
			
            if( (timeout_count*VPU_WAIT_TIME_OUT) > VPU_ENC_TIMEOUT) {
				//VLOG(ERR, "Error : encoder timeout happened\n");
				p_vpu_if->VPU_SWReset(m_codec_idx, SW_RESET_SAFETY, m_EncHandle);
                mmpResult = MMP_FAILURE;
				break;
			}
			int_reason = 0;
			timeout_count++;
		}
		
        MMPDEBUGMSG(m_DEBUG_VPUEnc, (TEXT("[CMmpEncoderVpuIF::EncodeAu] ln=%d int_reason=0x%x "),   __LINE__ , int_reason));

		if (int_reason & (1<<INT_BIT_BIT_BUF_FULL))	{
            break;
		}					

        //MMPDEBUGMSG(m_DEBUG_VPUEnc, (TEXT("[CMmpEncoderVpuIF::EncodeAu] ln=%d  "),   __LINE__ ));

		if(int_reason != 0)	{
			p_vpu_if->VPU_ClearInterrupt(m_codec_idx);
			if (int_reason & (1<<INT_BIT_PIC_RUN)) 
				break;
		}

        //MMPDEBUGMSG(m_DEBUG_VPUEnc, (TEXT("[CMmpEncoderVpuIF::EncodeAu] ln=%d  "),   __LINE__ ));
	}
    
    MMPDEBUGMSG(m_DEBUG_VPUEnc, (TEXT("[CMmpEncoderVpuIF::EncodeAu] ln=%d  Intr-OK"),   __LINE__ ));
    

    /* Process Result */
    if(mmpResult == MMP_SUCCESS) {
    

        MMPDEBUGMSG(m_DEBUG_VPUEnc, (TEXT("[CMmpEncoderVpuIF::EncodeAu] ln=%d  VPU_EncGetOutputInfo"),   __LINE__ ));

        ret = p_vpu_if->VPU_EncGetOutputInfo(m_EncHandle, &outputInfo);
        if(ret != RETCODE_SUCCESS) {
            MMPDEBUGMSG(m_DEBUG_VPUEnc, (TEXT("[CMmpEncoderVpuIF::EncodeAu] ln=%d  FAIL: VPU_EncGetOutputInfo "),   __LINE__ ));
            mmpResult = MMP_FAILURE;
        }
        else {
                        
            pBuffer = (MMP_U8*)p_buf_vs->get_buf_vir_addr();
            nBufMaxSize = p_buf_vs->get_buf_size();
            nBufSize = outputInfo.bitstreamSize;
            if(outputInfo.picType == PIC_TYPE_I)  nFlag = mmp_buffer_media::FLAG_VIDEO_KEYFRAME;
            else nFlag = mmp_buffer_media::FLAG_NULL;

            memcpy(pBuffer, (void*)m_vpu_enc_buffer.virt_addr, nBufSize);
            m_encoed_frame_count++;

            MMPDEBUGMSG(m_DEBUG_VPUEnc, (TEXT("[CMmpEncoderVpuIF::EncodeAu] ln=%d FrmCnt=%d nBufSz=%d  (%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x ) "),   
                                __LINE__ , m_encoed_frame_count,  nBufSize, 
                                pBuffer[0], pBuffer[1], pBuffer[2], pBuffer[3], 
                                pBuffer[4], pBuffer[5], pBuffer[6], pBuffer[7], 
                                pBuffer[8], pBuffer[9], pBuffer[10], pBuffer[11], 
                                pBuffer[12], pBuffer[13], pBuffer[14], pBuffer[15] 
                                ));

            p_buf_vs->set_stream_size(nBufSize);
            p_buf_vs->or_flag(nFlag);
        }
    }

    
    MMPDEBUGMSG(m_DEBUG_VPUEnc, (TEXT("---------------- VPU Enc End ------- res=%d nBufSize=%d FrmCnt=%d \n\n"), 
                    mmpResult, nBufSize, m_encoed_frame_count ));
    

    return mmpResult;
}
    
void CMmpEncoderVpuIF::print_reg() {

    MMP_CHAR *buffer = new MMP_CHAR[1024*10];
    MMP_CHAR szbuf[64];
    
    buffer[0] = '\0';
    strcat(buffer, "\n--------------------\n");
    
    sprintf(szbuf, "m_mapType = %d \n", m_mapType); strcat(buffer, szbuf);
    sprintf(szbuf, "m_encOP.bitstreamFormat = %d \n", m_encOP.bitstreamFormat); strcat(buffer, szbuf);    
    sprintf(szbuf, "m_encOP.linear2TiledEnable = %d \n", m_encOP.linear2TiledEnable); strcat(buffer, szbuf);    
    sprintf(szbuf, "m_encOP.picWidth = %d \n", m_encOP.picWidth); strcat(buffer, szbuf);    
    sprintf(szbuf, "m_encOP.picHeight = %d \n", m_encOP.picHeight); strcat(buffer, szbuf);    
    sprintf(szbuf, "m_encOP.frameRateInfo = %d \n", m_encOP.frameRateInfo); strcat(buffer, szbuf);    
    sprintf(szbuf, "m_encOP.MESearchRange = %d \n", m_encOP.MESearchRange); strcat(buffer, szbuf);    
    sprintf(szbuf, "m_encOP.rcEnable = %d \n", m_encOP.rcEnable); strcat(buffer, szbuf);    
    sprintf(szbuf, "m_encOP.gopSize = %d \n", m_encOP.gopSize); strcat(buffer, szbuf);    
    sprintf(szbuf, "m_encOP.frameSkipDisable = %d \n", m_encOP.frameSkipDisable); strcat(buffer, szbuf);    
    sprintf(szbuf, "m_encOP.initialDelay = %d \n", m_encOP.initialDelay); strcat(buffer, szbuf);    
    sprintf(szbuf, "m_encOP.bitRate  = %d \n", m_encOP.bitRate ); strcat(buffer, szbuf);    
    sprintf(szbuf, "m_encOP.vbvBufferSize  = %d \n", m_encOP.vbvBufferSize); strcat(buffer, szbuf);    
    sprintf(szbuf, "m_encOP.meBlkMode  = %d \n", m_encOP.meBlkMode); strcat(buffer, szbuf);    
    sprintf(szbuf, "m_encOP.sliceMode.sliceMode  = %d \n", m_encOP.sliceMode.sliceMode); strcat(buffer, szbuf);    
    sprintf(szbuf, "m_encOP.sliceMode.sliceSizeMode  = %d \n", m_encOP.sliceMode.sliceSizeMode); strcat(buffer, szbuf);    
    sprintf(szbuf, "m_encOP.sliceMode.sliceSize  = %d \n", m_encOP.sliceMode.sliceSize); strcat(buffer, szbuf);    
    sprintf(szbuf, "m_encOP.intraRefresh  = %d \n", m_encOP.intraRefresh); strcat(buffer, szbuf);    
    sprintf(szbuf, "m_encOP.rcIntraQp  = %d \n", m_encOP.rcIntraQp); strcat(buffer, szbuf);    
    sprintf(szbuf, "m_encOP.userQpMax  = %d \n", m_encOP.userQpMax); strcat(buffer, szbuf);    
    sprintf(szbuf, "m_encOP.userGamma  = %d \n", m_encOP.userGamma); strcat(buffer, szbuf);    
    sprintf(szbuf, "m_encOP.rcIntervalMode  = %d \n", m_encOP.rcIntervalMode); strcat(buffer, szbuf);    
    sprintf(szbuf, "m_encOP.mbInterval  = %d \n", m_encOP.mbInterval); strcat(buffer, szbuf);    
    sprintf(szbuf, "m_encOP.MEUseZeroPmv  = %d \n", m_encOP.MEUseZeroPmv); strcat(buffer, szbuf);    
    sprintf(szbuf, "m_encOP.intraCostWeight  = %d \n", m_encOP.intraCostWeight); strcat(buffer, szbuf);    
    sprintf(szbuf, "m_encOP.ringBufferEnable  = %d \n", m_encOP.ringBufferEnable); strcat(buffer, szbuf);    
    sprintf(szbuf, "m_encOP.bitstreamBuffer  = %d \n", m_encOP.bitstreamBuffer); strcat(buffer, szbuf);    
    sprintf(szbuf, "m_encOP.bitstreamBufferSize  = %d \n", m_encOP.bitstreamBufferSize); strcat(buffer, szbuf);    
    sprintf(szbuf, "m_encOP.cbcrInterleave  = %d \n", m_encOP.cbcrInterleave); strcat(buffer, szbuf);    
    sprintf(szbuf, "m_encOP.frameEndian  = %d \n", m_encOP.frameEndian); strcat(buffer, szbuf);    
    sprintf(szbuf, "m_encOP.streamEndian  = %d \n", m_encOP.streamEndian); strcat(buffer, szbuf);    
    sprintf(szbuf, "m_encOP.bwbEnable  = %d \n", m_encOP.bwbEnable); strcat(buffer, szbuf);    
    sprintf(szbuf, "m_encOP.lineBufIntEn  = %d \n", m_encOP.lineBufIntEn); strcat(buffer, szbuf);    
    sprintf(szbuf, "m_encOP.coreIdx  = %d \n", m_encOP.coreIdx); strcat(buffer, szbuf);    
        
    strcat(buffer, "\n--------------------\n");

    MMPDEBUGMSG(MMPZONE_MONITOR, (TEXT("%s"), buffer ));

    delete [] buffer;
}

void CMmpEncoderVpuIF::make_encOP_Common() {
    
    
    m_mapType = LINEAR_FRAME_MAP; //Map Type 0(Linear) / 1(Frame-V) / 2(Frame-H) / 3(Field-V) / 4(Mix-V) / 5(Frame-MB) / 6(Filed-MB) / 16(Tile2Linear)
        
    m_encOP.bitstreamFormat = STD_AVC; //0(H.264) / 1(VC1) / 2(MPEG2) / 3(MPEG4) / 4(H263) / 5(DIVX3) / 6(RV) / 7(AVS) / 11(VP8)
    m_encOP.linear2TiledEnable = (m_mapType>>4)&0x1;

    m_encOP.picWidth = m_pic_width;
    m_encOP.picHeight = m_pic_height;
    m_encOP.frameRateInfo = 30;//this->m_create_config.nFrameRate; //30;
    m_encOP.MESearchRange = 3;	
    
#if 1 /* bitrate control enable */
    m_encOP.rcEnable = 1;
    m_encOP.gopSize = 20;          //this->m_create_config.nIDRPeriod; //30;					// only first picture is I
    //m_encOP.gopSize = 10;          //this->m_create_config.nIDRPeriod; //30;					// only first picture is I
    m_encOP.frameSkipDisable = 1;  // for compare with C-model ( C-model = only 1 )
    m_encOP.initialDelay = 0;
    m_encOP.bitRate = 10000;        /* unit: kbps */
    //m_encOP.bitRate = 10000;      /* unit: kbps   27fps */
    //m_encOP.bitRate =   4000;     /* unit: kbps */
    //m_encOP.bitRate = 1000;       /* unit: kbps */
#else
    m_encOP.rcEnable = 0;
    m_encOP.idrInterval = 30;
    m_encOP.gopSize = 10;//this->m_create_config.nIDRPeriod; //30;					// only first picture is I
    m_encOP.bitRate = 0;
#endif
    
#if 0
    if (pEncInfo->openParam.rcEnable) { // rate control enabled
		/* coda960 ENCODER */
        VpuWriteReg(pCodecInst->coreIdx, CMD_ENC_SEQ_GOP_NUM, pEncInfo->openParam.gopSize);

		data = (pEncInfo->openParam.frameSkipDisable) << 31 |
			pEncInfo->openParam.initialDelay << 16 |
			pEncInfo->openParam.bitRate<<1 | 1;
		VpuWriteReg(pCodecInst->coreIdx, CMD_ENC_SEQ_RC_PARA, data);
	}
	else {
        if (pEncInfo->openParam.bitstreamFormat == STD_AVC)
            VpuWriteReg(pCodecInst->coreIdx, CMD_ENC_SEQ_GOP_NUM,  (pEncInfo->openParam.idrInterval << 21) | pEncInfo->openParam.gopSize);
        else
		  VpuWriteReg(pCodecInst->coreIdx, CMD_ENC_SEQ_GOP_NUM, pEncInfo->openParam.gopSize);
		VpuWriteReg(pCodecInst->coreIdx, CMD_ENC_SEQ_RC_PARA, 0);
	}
#endif
    
    m_encOP.vbvBufferSize = 0;			// 0 = ignore
    m_encOP.meBlkMode = 0;		// for compare with C-model ( C-model = only 0 )
    
    m_encOP.sliceMode.sliceMode = 0;		// slice mode (0 : one slice, 1 : multiple slice)
    m_encOP.sliceMode.sliceSizeMode = 0;    // slice size mode (0 : slice bit number, 1 : slice mb number)
    m_encOP.sliceMode.sliceSize = 0;        // slice size number (bit count or mb number)

    m_encOP.intraRefresh = 0;
    m_encOP.rcIntraQp = -1;				// disable == -1
    m_encOP.userQpMax		= -1;
    m_encOP.userGamma		= -1; //(Uint32)(0.75*32768);		//  (0*32768 < gamma < 1*32768)
    m_encOP.rcIntervalMode = 0;						// 0:normal, 1:frame_level, 2:slice_level, 3: user defined Mb_level
    m_encOP.mbInterval	= 0;
    m_encOP.MEUseZeroPmv = 0;		
    m_encOP.intraCostWeight = 900;
    
    m_encOP.bitstreamBuffer = m_vpu_enc_buffer.phys_addr;//m_vbStream.phys_addr;
    m_encOP.bitstreamBufferSize = m_vpu_enc_buffer.size;//m_vbStream.size;
	m_encOP.ringBufferEnable =  0;//encConfig.ringBufferEnable;
	//m_encOP.cbcrInterleave = CBCR_INTERLEAVE;
	m_encOP.cbcrInterleave = m_is_cbcr_interleave;
	if (m_mapType == TILED_FRAME_MB_RASTER_MAP ||
		m_mapType == TILED_FIELD_MB_RASTER_MAP) {
		m_encOP.cbcrInterleave = 1;
	}
	m_encOP.frameEndian = VPU_FRAME_ENDIAN;
	m_encOP.streamEndian = VPU_STREAM_ENDIAN;
	m_encOP.bwbEnable = VPU_ENABLE_BWB;
	m_encOP.lineBufIntEn =  1; //encConfig.lineBufIntEn;
	m_encOP.coreIdx	  = m_codec_idx;
    
}

void CMmpEncoderVpuIF::make_encOP_H263() {

    this->make_encOP_Common();

    m_encOP.bitstreamFormat = STD_H263; //0(H.264) / 1(VC1) / 2(MPEG2) / 3(MPEG4) / 4(H263) / 5(DIVX3) / 6(RV) / 7(AVS) / 11(VP8)
    
    m_encOP.EncStdParam.h263Param.h263AnnexIEnable = 0;
    m_encOP.EncStdParam.h263Param.h263AnnexJEnable = 0;
    m_encOP.EncStdParam.h263Param.h263AnnexKEnable = 0;
    m_encOP.EncStdParam.h263Param.h263AnnexTEnable = 0;		
}

void CMmpEncoderVpuIF::make_encOP_H264() {
    
    this->make_encOP_Common();

    m_encOP.bitstreamFormat = STD_AVC; //0(H.264) / 1(VC1) / 2(MPEG2) / 3(MPEG4) / 4(H263) / 5(DIVX3) / 6(RV) / 7(AVS) / 11(VP8)
    
    m_encOP.EncStdParam.avcParam.constrainedIntraPredFlag = 0;
    m_encOP.EncStdParam.avcParam.disableDeblk = 0; //// disable_deblk (0 : enable, 1 : disable, 2 : disable at slice boundary)

    m_encOP.EncStdParam.avcParam.deblkFilterOffsetAlpha = 6;
    m_encOP.EncStdParam.avcParam.deblkFilterOffsetBeta = 0;
    m_encOP.EncStdParam.avcParam.chromaQpOffset = 10;
    m_encOP.EncStdParam.avcParam.audEnable = 0;
    m_encOP.EncStdParam.avcParam.frameCroppingFlag = 0;
    m_encOP.EncStdParam.avcParam.frameCropLeft = 0;
    m_encOP.EncStdParam.avcParam.frameCropRight = 0;
    m_encOP.EncStdParam.avcParam.frameCropTop = 0;
    m_encOP.EncStdParam.avcParam.frameCropBottom = 0;
    m_encOP.EncStdParam.avcParam.level = 0;

    if(m_pic_height == 1080) {
         m_encOP.EncStdParam.avcParam.frameCroppingFlag = 1;
         m_encOP.EncStdParam.avcParam.frameCropBottom = 8;
    }
	
}

void CMmpEncoderVpuIF::make_encOP_MPEG4() {

    this->make_encOP_Common();

    m_encOP.bitstreamFormat = STD_MPEG4; //0(H.264) / 1(VC1) / 2(MPEG2) / 3(MPEG4) / 4(H263) / 5(DIVX3) / 6(RV) / 7(AVS) / 11(VP8)
    m_encOP.MEUseZeroPmv = 1;			
    
    m_encOP.EncStdParam.mp4Param.mp4DataPartitionEnable = 0;
    m_encOP.EncStdParam.mp4Param.mp4ReversibleVlcEnable = 0;
    m_encOP.EncStdParam.mp4Param.mp4IntraDcVlcThr = 0;
    m_encOP.EncStdParam.mp4Param.mp4HecEnable	= 0;
    m_encOP.EncStdParam.mp4Param.mp4Verid = 2;		
}

