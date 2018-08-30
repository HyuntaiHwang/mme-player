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

#include "MmpEncoderVpu.hpp"
#include "MmpUtil.hpp"
#include "vpuhelper.h"
#include "MmpH264Tool.hpp"

extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavutil/mem.h"
#include "libavresample/audio_convert.h"
}

#define PUT_BYTE(_p, _b) \
    *_p++ = (unsigned char)_b; 

#define PUT_BUFFER(_p, _buf, _len) \
    memcpy(_p, _buf, _len); \
    _p += _len;

#define PUT_LE32(_p, _var) \
    *_p++ = (unsigned char)((_var)>>0);  \
    *_p++ = (unsigned char)((_var)>>8);  \
    *_p++ = (unsigned char)((_var)>>16); \
    *_p++ = (unsigned char)((_var)>>24); 

#define PUT_BE32(_p, _var) \
    *_p++ = (unsigned char)((_var)>>24);  \
    *_p++ = (unsigned char)((_var)>>16);  \
    *_p++ = (unsigned char)((_var)>>8); \
    *_p++ = (unsigned char)((_var)>>0); 


#define PUT_LE16(_p, _var) \
    *_p++ = (unsigned char)((_var)>>0);  \
    *_p++ = (unsigned char)((_var)>>8);  


#define PUT_BE16(_p, _var) \
    *_p++ = (unsigned char)((_var)>>8);  \
    *_p++ = (unsigned char)((_var)>>0);  


//#define ENC_SOURCE_FRAME_DISPLAY

#define VPU_ENC_TIMEOUT       1000
#define VPU_DEC_TIMEOUT       1000
#define VPU_WAIT_TIME_OUT	100		//should be less than normal decoding time to give a chance to fill stream. if this value happens some problem. we should fix VPU_WaitInterrupt function
#define PARALLEL_VPU_WAIT_TIME_OUT 0 	//the value of timeout is 0 means we just check interrupt flag. do not wait any time to give a chance of an interrupt of the next core.


#if PARALLEL_VPU_WAIT_TIME_OUT > 0 
#undef VPU_DEC_TIMEOUT
#define VPU_DEC_TIMEOUT       1000
#endif


#define MAX_CHUNK_HEADER_SIZE 1024
#define MAX_DYNAMIC_BUFCOUNT	3
#define NUM_FRAME_BUF			19
#define MAX_ROT_BUF_NUM			2
#define EXTRA_FRAME_BUFFER_NUM	1

#define STREAM_BUF_SIZE		 0x300000  // max bitstream size

//#define STREAM_FILL_SIZE    (512 * 16)  //  4 * 1024 | 512 | 512+256( wrap around test )
#define STREAM_FILL_SIZE    0x2000  //  4 * 1024 | 512 | 512+256( wrap around test )

#define STREAM_END_SIZE			0
#define STREAM_END_SET_FLAG		0
#define STREAM_END_CLEAR_FLAG	-1
#define STREAM_READ_SIZE    (512 * 16)

//#define SUPPORT_SW_MIXER


#define FORCE_SET_VSYNC_FLAG
//#define TEST_USER_FRAME_BUFFER
#ifdef TEST_USER_FRAME_BUFFER
//#define TEST_MULTIPLE_CALL_REGISTER_FRAME_BUFFER
#endif



/////////////////////////////////////////////////////////////
//CMmpEncoderVpu Member Functions

CMmpEncoderVpu::CMmpEncoderVpu(struct MmpEncoderCreateConfig *pCreateConfig) :

m_create_config(*pCreateConfig)

,m_codec_idx(0)
,m_version(0)
,m_revision(0)
,m_productId(0)

,m_EncHandle(NULL)
,m_regFrameBufCount(0)

,m_mapType(LINEAR_FRAME_MAP) //Map Type 0(Linear) / 1(Frame-V) / 2(Frame-H) / 3(Field-V) / 4(Mix-V) / 5(Frame-MB) / 6(Filed-MB) / 16(Tile2Linear)
,m_srcFrameIdx(0)
{
    memset(&m_encOP, 0x00, sizeof(m_encOP));
    memset(&m_vbStream, 0x00, sizeof(m_vbStream));
    memset(&m_enc_init_info, 0x00, sizeof(m_enc_init_info));

}

CMmpEncoderVpu::~CMmpEncoderVpu()
{
    
}

MMP_RESULT CMmpEncoderVpu::Open()
{
    RetCode vpu_ret;
    int iret;
    MMP_RESULT mmpResult = MMP_SUCCESS;

    
    vpu_ret = VPU_Init(m_codec_idx);
    if( (vpu_ret == RETCODE_SUCCESS) || (vpu_ret == RETCODE_CALLED_BEFORE) ) {
        
        VPU_GetVersionInfo(m_codec_idx, &m_version, &m_revision, &m_productId);	

        MMPDEBUGMSG(MMPZONE_INFO, (TEXT("VPU coreNum : [%d]\n"), m_codec_idx));
        MMPDEBUGMSG(MMPZONE_INFO, (TEXT("Firmware Version => projectId : %x | version : %04d.%04d.%08d | revision : r%d\n"), 
                    (Uint32)(m_version>>16), (Uint32)((m_version>>(12))&0x0f), (Uint32)((m_version>>(8))&0x0f), (Uint32)((m_version)&0xff), m_revision));
        MMPDEBUGMSG(MMPZONE_INFO, (TEXT("Hardware Version => %04x\n"), m_productId));
        MMPDEBUGMSG(MMPZONE_INFO, (TEXT("API Version => %04x\n\n"), API_VERSION));
    }
    else {

        MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpEncoderVpu::Open] FAIL :  VPU_Init \n")));
        mmpResult = MMP_FAILURE;
    }

    /* alloc dma buffer */
    if(mmpResult == MMP_SUCCESS) {
        
        m_vbStream.size	 = STREAM_BUF_SIZE;
        iret = vdi_allocate_dma_memory(m_codec_idx, &m_vbStream);
	    if(iret < 0)  {
            m_vbStream.base = 0;
		    MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpEncoderVpu::Open] FAIL :  vdi_allocate_dma_memory (iret=%d) \n"), iret));
            mmpResult = MMP_FAILURE;
	    }
    }

    /* Decoder Open */
    if(mmpResult == MMP_SUCCESS) {

        switch(this->m_create_config.nFormat) {
    
            /* Video */
            case MMP_FOURCC_VIDEO_H263: this->make_encOP_H263(); break;
            case MMP_FOURCC_VIDEO_H264: this->make_encOP_H264(); break;
            case MMP_FOURCC_VIDEO_MPEG4: this->make_encOP_MPEG4(); break;
            default:  
                mmpResult = MMP_FAILURE;
                break;
        }

        vpu_ret = VPU_EncOpen(&m_EncHandle, &m_encOP);
	    if( vpu_ret != RETCODE_SUCCESS ) {
		    MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpEncoderVpu::Open] FAIL :  VPU_DecOpen (vpu_ret=%d) \n"), vpu_ret));
            mmpResult = MMP_FAILURE;
	    }
        else {
           // ret = VPU_DecGiveCommand(m_EncHandle, GET_DRAM_CONFIG, &dramCfg);
	       // if( ret != RETCODE_SUCCESS ) {
		   //     VLOG(ERR, "VPU_DecGiveCommand[GET_DRAM_CONFIG] failed Error code is 0x%x \n", ret );
		   //     goto ERR_DEC_OPEN;
	       // }

            
            SecAxiUse		secAxiUse = {0};

            secAxiUse.useBitEnable = USE_BIT_INTERNAL_BUF;
	        secAxiUse.useIpEnable = USE_IP_INTERNAL_BUF;
	        secAxiUse.useDbkYEnable = USE_DBKY_INTERNAL_BUF;
	        secAxiUse.useDbkCEnable = USE_DBKC_INTERNAL_BUF;
	        secAxiUse.useBtpEnable = USE_BTP_INTERNAL_BUF;
	        secAxiUse.useOvlEnable = USE_OVL_INTERNAL_BUF;
	        VPU_EncGiveCommand(m_EncHandle, SET_SEC_AXI, &secAxiUse);
        }
	}


    return mmpResult;
}


MMP_RESULT CMmpEncoderVpu::Close()
{
    MMPDEBUGMSG(1, (TEXT("[CMmpEncoderVpu::Close] ln=%d "), __LINE__ ));
    if(m_EncHandle != NULL) {

       MMPDEBUGMSG(1, (TEXT("[CMmpEncoderVpu::Close] ln=%d "), __LINE__ ));
	   VPU_EncClose(m_EncHandle);
       m_EncHandle = NULL;

        if(m_vbStream.base != 0) {
            vdi_free_dma_memory(m_codec_idx, &m_vbStream);
            m_vbStream.base = 0;
        }

        MMPDEBUGMSG(1, (TEXT("[CMmpEncoderVpu::Close] ln=%d "), __LINE__ ));
        VPU_DeInit(m_codec_idx);
    }

    return MMP_SUCCESS;
}

MMP_RESULT CMmpEncoderVpu::EncodeDSI() {

    MMP_RESULT mmpResult = MMP_SUCCESS;
    TiledMapConfig mapCfg;
    FrameBufferAllocInfo fbAllocInfo;
    RetCode ret;
    

    //srcFrameWidth = ((encOP.picWidth+15)&~15);
	//srcFrameStride = srcFrameWidth;
	//srcFrameFormat = FORMAT_420;
	//framebufFormat = FORMAT_420;
	//srcFrameHeight = ((encOP.picHeight+15)&~15);
    m_framebufWidth = MMP_BYTE_ALIGN(m_encOP.picWidth, 16); //(encConfig.rotAngle==90||encConfig.rotAngle ==270)?srcFrameHeight:srcFrameWidth;
	m_framebufHeight = MMP_BYTE_ALIGN(m_encOP.picHeight, 16);//(encConfig.rotAngle==90||encConfig.rotAngle ==270)?srcFrameWidth:srcFrameHeight;
	m_framebufStride = m_framebufWidth;


    /* Enc Seq Init */
    if(mmpResult == MMP_SUCCESS) {
        ret = VPU_EncGetInitialInfo(m_EncHandle, &m_enc_init_info);
	    if( ret != RETCODE_SUCCESS )
	    {
		    VLOG(ERR, "VPU_EncGetInitialInfo failed Error code is 0x%x \n", ret );
		    mmpResult = MMP_FAILURE;
	    }
        else {
	        VPU_ClearInterrupt(m_codec_idx);
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
	    VPU_EncGiveCommand(m_EncHandle, SET_CACHE_CONFIG, &m_encCacheConfig);

        ret = VPU_EncRegisterFrameBuffer(m_EncHandle, NULL, m_regFrameBufCount, m_framebufStride, m_framebufHeight, m_mapType);
	    if( ret != RETCODE_SUCCESS )
	    {
		    //VLOG(ERR, "VPU_EncRegisterFrameBuffer failed Error code is 0x%x \n", ret );
		    mmpResult = MMP_SUCCESS;
	    }
        else {
	        VPU_EncGiveCommand(m_EncHandle, GET_TILEDMAP_CONFIG, &mapCfg);
        }
    }

    /* Allocate Frame Buffer */
    if(mmpResult == MMP_SUCCESS) {

        fbAllocInfo.format = FORMAT_420; //srcFrameFormat;
	    fbAllocInfo.cbcrInterleave = m_encOP.cbcrInterleave;
	    if(m_encOP.linear2TiledEnable)
		    fbAllocInfo.mapType = LINEAR_FRAME_MAP;
	    else
		    fbAllocInfo.mapType = m_mapType;

	    fbAllocInfo.stride = m_framebufStride;
	    fbAllocInfo.height = m_framebufHeight;
	    fbAllocInfo.num = ENC_SRC_BUF_NUM;
	    fbAllocInfo.endian = m_encOP.frameEndian;
	    fbAllocInfo.type = FB_TYPE_PPU;
	    ret = VPU_EncAllocateFrameBuffer(m_EncHandle, fbAllocInfo, m_fbSrc);
	    if( ret != RETCODE_SUCCESS )
	    {
		    //VLOG(ERR, "VPU_EncAllocateFrameBuffer fail to allocate source frame buffer is 0x%x \n", ret );
		    mmpResult = MMP_FAILURE;
	    }
    }

    EncHeaderParam encHeaderParam = { 0 };
    MMP_U8* p_dsi = m_DSI;
    MMP_S32 dsi_size = 0;
    
    switch(m_encOP.bitstreamFormat) {
    
        case STD_MPEG4:

            encHeaderParam.buf = m_vbStream.phys_addr;
		    encHeaderParam.headerType = VOS_HEADER;
		    encHeaderParam.size = m_vbStream.size;
            ret = VPU_EncGiveCommand(m_EncHandle, ENC_PUT_VIDEO_HEADER, &encHeaderParam);
		    if (ret != RETCODE_SUCCESS)
		    {
			    //VLOG(ERR, "VPU_EncGiveCommand ( ENC_PUT_VIDEO_HEADER ) for VOS_HEADER failed Error code is 0x%x \n", ret);			
			    mmpResult = MMP_FAILURE; //goto ERR_ENC_OPEN;
		    }
            else {
                vdi_read_memory(m_codec_idx, encHeaderParam.buf, p_dsi, encHeaderParam.size, m_encOP.streamEndian);
                p_dsi+=encHeaderParam.size;
                dsi_size+=encHeaderParam.size;
			}

            encHeaderParam.headerType = VOL_HEADER;
		    encHeaderParam.size = m_vbStream.size;
		    ret = VPU_EncGiveCommand(m_EncHandle, ENC_PUT_VIDEO_HEADER, &encHeaderParam); 
		    if (ret != RETCODE_SUCCESS)
		    {
			    //VLOG(ERR, "VPU_EncGiveCommand ( ENC_PUT_VIDEO_HEADER ) for VOL_HEADER failed Error code is 0x%x \n", ret);			
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
		    encHeaderParam.buf  = m_vbStream.phys_addr;
		    encHeaderParam.size = m_vbStream.size;
            ret = VPU_EncGiveCommand(m_EncHandle, ENC_PUT_VIDEO_HEADER, &encHeaderParam); 
            if (ret != RETCODE_SUCCESS){
                mmpResult = MMP_FAILURE; //goto ERR_ENC_OPEN;
            }
            else {
                vdi_read_memory(m_codec_idx, encHeaderParam.buf, p_dsi, encHeaderParam.size, m_encOP.streamEndian);
                p_dsi+=encHeaderParam.size;
                dsi_size+=encHeaderParam.size;
            }
		
            encHeaderParam.headerType = PPS_RBSP;
		    encHeaderParam.buf		= m_vbStream.phys_addr;
		    encHeaderParam.pBuf	   = (BYTE *)m_vbStream.virt_addr;
		    encHeaderParam.size	   = m_vbStream.size;
		    ret = VPU_EncGiveCommand(m_EncHandle, ENC_PUT_VIDEO_HEADER, &encHeaderParam);
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

MMP_RESULT CMmpEncoderVpu::EncodeAuEx1(CMmpMediaSampleEncode* pMediaSample, CMmpMediaSampleEncodeResult* pEncResult) {

    MMP_RESULT mmpResult = MMP_SUCCESS;
    EncParam		encParam	= { 0 };
    FrameBuffer *pFB;
    int luma_size, chroma_size;
    int int_reason, timeout_count;
    RetCode ret;
    EncOutputInfo	outputInfo	= { 0 };

    MMP_U8* pBuffer;
    MMP_U32 nBufSize, nBufMaxSize, nFlag;


    luma_size = m_framebufStride*m_framebufHeight;
    chroma_size = luma_size/4;
    pFB = &m_fbSrc[m_srcFrameIdx];
    
    vdi_write_memory(m_codec_idx, pFB->bufY, (Uint8 *)pMediaSample->uiBufferLogAddr[MMP_DECODED_BUF_Y], luma_size, m_encOP.frameEndian);
    vdi_write_memory(m_codec_idx, pFB->bufCb, (Uint8 *)pMediaSample->uiBufferLogAddr[MMP_DECODED_BUF_U], chroma_size, m_encOP.frameEndian);
    vdi_write_memory(m_codec_idx, pFB->bufCr, (Uint8 *)pMediaSample->uiBufferLogAddr[MMP_DECODED_BUF_V], chroma_size, m_encOP.frameEndian);
    
    encParam.forceIPicture = 0;
	encParam.skipPicture   = 0;
	encParam.quantParam	   = 10;//encConfig.picQpY;

    encParam.sourceFrame = pFB;
    encParam.picStreamBufferAddr = m_vbStream.phys_addr;	// can set the newly allocated buffer.
	encParam.picStreamBufferSize = m_vbStream.size;

    ret = VPU_EncStartOneFrame(m_EncHandle, &encParam);
	if( ret != RETCODE_SUCCESS )
	{
		//VLOG(ERR, "VPU_EncStartOneFrame failed Error code is 0x%x \n", ret );
		//goto ERR_ENC_OPEN;
        mmpResult = MMP_FAILURE;
	}

    timeout_count = 0;
	while(mmpResult == MMP_SUCCESS) 
	{
		int_reason = VPU_WaitInterrupt(m_codec_idx, VPU_WAIT_TIME_OUT);
		if (int_reason == (int)-1)	{
			
            if( (timeout_count*VPU_WAIT_TIME_OUT) > VPU_ENC_TIMEOUT) {
				//VLOG(ERR, "Error : encoder timeout happened\n");
				VPU_SWReset(m_codec_idx, SW_RESET_SAFETY, m_EncHandle);
                mmpResult = MMP_FAILURE;
				break;
			}
			int_reason = 0;
			timeout_count++;
		}
		
		if(1) //encOP.ringBufferEnable == 0 && encOP.lineBufIntEn) 
		{
			if (int_reason & (1<<INT_BIT_BIT_BUF_FULL))		{

                //vdi_read_memory(m_codec_idx, m_encOP.bitstreamBuffer, buf, size, endian);
				//    if (!ReadBsResetBufHelper(coreIdx, fpBitstrm, encOP.bitstreamBuffer, encOP.bitstreamBufferSize, encOP.streamEndian))
				//	goto ERR_ENC_OPEN;
                break;
			}					
		}


		if (int_reason)
		{
			VPU_ClearInterrupt(m_codec_idx);
			if (int_reason & (1<<INT_BIT_PIC_RUN)) 
				break;
		}
	}

    if(mmpResult == MMP_SUCCESS) {
    
        ret = VPU_EncGetOutputInfo(m_EncHandle, &outputInfo);
        if(ret != RETCODE_SUCCESS) {
            mmpResult = MMP_FAILURE;
        }
        else {
            pBuffer = (MMP_U8*)pEncResult->uiEncodedBufferLogAddr[MMP_ENCODED_BUF_STREAM];
            nBufMaxSize = pEncResult->uiEncodedBufferMaxSize[MMP_ENCODED_BUF_STREAM];
            nBufSize = outputInfo.bitstreamSize;
            if(outputInfo.picType == PIC_TYPE_I)  nFlag = MMP_MEDIASAMPMLE_FLAG_VIDEO_KEYFRAME;
            else nFlag = 0;

            vdi_read_memory(m_codec_idx, outputInfo.bitstreamBuffer, pBuffer, outputInfo.bitstreamSize, m_encOP.streamEndian);

            pEncResult->uiEncodedStreamSize[MMP_ENCODED_BUF_STREAM] = nBufSize;
            pEncResult->uiFlag = nFlag;
        }

    }

    m_srcFrameIdx = (m_srcFrameIdx+1)%m_regFrameBufCount;

    return mmpResult;
}

void CMmpEncoderVpu::make_encOP_Common() {
    
    
    m_mapType = LINEAR_FRAME_MAP; //Map Type 0(Linear) / 1(Frame-V) / 2(Frame-H) / 3(Field-V) / 4(Mix-V) / 5(Frame-MB) / 6(Filed-MB) / 16(Tile2Linear)
        
    m_encOP.bitstreamFormat = STD_AVC; //0(H.264) / 1(VC1) / 2(MPEG2) / 3(MPEG4) / 4(H263) / 5(DIVX3) / 6(RV) / 7(AVS) / 11(VP8)
    m_encOP.linear2TiledEnable = (m_mapType>>4)&0x1;

    m_encOP.picWidth = this->m_create_config.nPicWidth;
    m_encOP.picHeight = this->m_create_config.nPicHeight;
    m_encOP.frameRateInfo = this->m_create_config.nFrameRate; //30;
    m_encOP.MESearchRange = 3;	
    m_encOP.bitRate = this->m_create_config.nBitRate/1024;
    m_encOP.initialDelay = 0;
    m_encOP.vbvBufferSize = 0;			// 0 = ignore
    m_encOP.meBlkMode = 0;		// for compare with C-model ( C-model = only 0 )
    m_encOP.frameSkipDisable = 1;			// for compare with C-model ( C-model = only 1 )
    m_encOP.gopSize = this->m_create_config.nIDRPeriod; //30;					// only first picture is I
    m_encOP.sliceMode.sliceMode = 1;		// 1 slice per picture
    m_encOP.sliceMode.sliceSizeMode = 1;
    m_encOP.sliceMode.sliceSize = 115;
    m_encOP.intraRefresh = 0;
    m_encOP.rcIntraQp = -1;				// disable == -1
    m_encOP.userQpMax		= 0;
    m_encOP.userGamma		= (Uint32)(0.75*32768);		//  (0*32768 < gamma < 1*32768)
    m_encOP.rcIntervalMode= 1;						// 0:normal, 1:frame_level, 2:slice_level, 3: user defined Mb_level
    m_encOP.mbInterval	= 0;
    //pEncConfig->picQpY = 23;
    //if (bitFormat == STD_MPEG4)
    //    m_encOP.MEUseZeroPmv = 1;			
    //else
    m_encOP.MEUseZeroPmv = 0;		
    m_encOP.intraCostWeight = 400;


    m_encOP.bitstreamBuffer = m_vbStream.phys_addr;
	m_encOP.bitstreamBufferSize = m_vbStream.size;
	m_encOP.ringBufferEnable =  0;//encConfig.ringBufferEnable;
	m_encOP.cbcrInterleave = CBCR_INTERLEAVE;
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

void CMmpEncoderVpu::make_encOP_H263() {

    this->make_encOP_Common();

    m_encOP.bitstreamFormat = STD_H263; //0(H.264) / 1(VC1) / 2(MPEG2) / 3(MPEG4) / 4(H263) / 5(DIVX3) / 6(RV) / 7(AVS) / 11(VP8)
    
    m_encOP.EncStdParam.h263Param.h263AnnexIEnable = 0;
    m_encOP.EncStdParam.h263Param.h263AnnexJEnable = 0;
    m_encOP.EncStdParam.h263Param.h263AnnexKEnable = 0;
    m_encOP.EncStdParam.h263Param.h263AnnexTEnable = 0;		
}

void CMmpEncoderVpu::make_encOP_H264() {
    
    this->make_encOP_Common();

    m_encOP.bitstreamFormat = STD_AVC; //0(H.264) / 1(VC1) / 2(MPEG2) / 3(MPEG4) / 4(H263) / 5(DIVX3) / 6(RV) / 7(AVS) / 11(VP8)
    
    m_encOP.EncStdParam.avcParam.constrainedIntraPredFlag = 0;
    m_encOP.EncStdParam.avcParam.disableDeblk = 1;
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

	
}

void CMmpEncoderVpu::make_encOP_MPEG4() {

    this->make_encOP_Common();

    m_encOP.bitstreamFormat = STD_MPEG4; //0(H.264) / 1(VC1) / 2(MPEG2) / 3(MPEG4) / 4(H263) / 5(DIVX3) / 6(RV) / 7(AVS) / 11(VP8)
    m_encOP.MEUseZeroPmv = 1;			
    
    m_encOP.EncStdParam.mp4Param.mp4DataPartitionEnable = 0;
    m_encOP.EncStdParam.mp4Param.mp4ReversibleVlcEnable = 0;
    m_encOP.EncStdParam.mp4Param.mp4IntraDcVlcThr = 0;
    m_encOP.EncStdParam.mp4Param.mp4HecEnable	= 0;
    m_encOP.EncStdParam.mp4Param.mp4Verid = 2;		
}

void CMmpEncoderVpu::make_user_frame() {

#if 0
    FrameBufferAllocInfo fbAllocInfo;
    int framebufStride, framebufHeight;
    DRAMConfig dramCfg = {0};
    FrameBuffer  fbUser[MAX_REG_FRAME]={0,};

    framebufStride = MMP_BYTE_ALIGN(m_dec_init_info.picWidth, 16);
    framebufHeight = MMP_BYTE_ALIGN(m_dec_init_info.picHeight, 16);

    fbAllocInfo.format          = FORMAT_420;
	fbAllocInfo.cbcrInterleave  = m_encOP.cbcrInterleave;
	fbAllocInfo.mapType         = m_mapType;
	fbAllocInfo.stride          = framebufStride;
	fbAllocInfo.height          = framebufHeight;
	fbAllocInfo.num             = m_regFrameBufCount;
	
	fbAllocInfo.endian          = m_encOP.frameEndian;
	fbAllocInfo.type            = FB_TYPE_CODEC;

    m_framebufSize = VPU_GetFrameBufSize(framebufStride, framebufHeight, fbAllocInfo.mapType, framebufFormat, &dramCfg);

    for (i=0; i<fbAllocInfo.num; i++)
	{
		vbFrame[i].size = framebufSize;
		if (vdi_allocate_dma_memory(coreIdx, &vbFrame[i]) < 0)
		{
			VLOG(ERR, "fail to allocate frame buffer\n" );
			goto ERR_DEC_OPEN;
		}
		fbUser[i].bufY = vbFrame[i].phys_addr;
		fbUser[i].bufCb = -1;
		fbUser[i].bufCr = -1;
	}
    ret = VPU_DecAllocateFrameBuffer(handle, fbAllocInfo, fbUser);
    pUserFrame = (FrameBuffer *)fbUser;				
#endif
}
