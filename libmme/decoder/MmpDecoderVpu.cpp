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

#include "MmpDecoderVpu.hpp"
#include "MmpUtil.hpp"
#include "vpuhelper.h"
#include "MmpH264Tool.hpp"
#include "mmp_buffer_mgr.hpp"

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

#define ENC_SRC_BUF_NUM			2
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
//CMmpDecoderVpu Member Functions

CMmpDecoderVpu::CMmpDecoderVpu(struct MmpDecoderCreateConfig *pCreateConfig) :

m_create_config(*pCreateConfig)

,m_codec_idx(0)
,m_version(0)
,m_revision(0)
,m_productId(0)

,m_DecHandle(NULL)
,m_regFrameBufCount(0)

,m_p_dsi_stream(NULL)
,m_dsi_stream_size(0)
{
    int i;

    memset(&m_decOP, 0x00, sizeof(m_decOP));
    memset(&m_vbStream, 0x00, sizeof(m_vbStream));
    memset(&m_dec_init_info, 0x00, sizeof(m_dec_init_info));

    for(i = 0; i < MAX_FRAMEBUFFER_COUNT; i++) {
        memset(&m_vbFrame[i], 0x00, sizeof(vpu_buffer_t));
    }
    
    for(i = 0; i < MAX_FRAMEBUFFER_COUNT; i++) {
        m_p_decoded_buffer[i] = NULL;
    }
}

CMmpDecoderVpu::~CMmpDecoderVpu()
{
    
}

MMP_RESULT CMmpDecoderVpu::Open()
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

        MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpDecoderVpu::Open] FAIL :  VPU_Init \n")));
        mmpResult = MMP_FAILURE;
    }

    /* alloc dma buffer */
    if(mmpResult == MMP_SUCCESS) {
        
        m_vbStream.size	 = STREAM_BUF_SIZE;
        iret = vdi_allocate_dma_memory(m_codec_idx, &m_vbStream);
	    if(iret < 0)  {
            m_vbStream.base = 0;
		    MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpDecoderVpu::Open] FAIL :  vdi_allocate_dma_memory (iret=%d) \n"), iret));
            mmpResult = MMP_FAILURE;
	    }
    }

    /* Decoder Open */
    if(mmpResult == MMP_SUCCESS) {

        switch(this->m_create_config.nFormat) {
    
            /* Video */
            case MMP_FOURCC_VIDEO_H263: this->make_decOP_H263(); break;
            case MMP_FOURCC_VIDEO_H264: this->make_decOP_H264(); break;
            case MMP_FOURCC_VIDEO_MPEG4: this->make_decOP_MPEG4(); break;
            case MMP_FOURCC_VIDEO_MPEG2: this->make_decOP_MPEG2(); break;
            
            case MMP_FOURCC_VIDEO_WMV3:
            case MMP_FOURCC_VIDEO_VC1: this->make_decOP_VC1(); break;

            case MMP_FOURCC_VIDEO_MSMPEG4V3: this->make_decOP_MSMpeg4V3(); break;
            
            case MMP_FOURCC_VIDEO_RV30: this->make_decOP_RV30(); break;
            case MMP_FOURCC_VIDEO_RV40: this->make_decOP_RV40(); break;
            case MMP_FOURCC_VIDEO_VP80: this->make_decOP_VP80(); break;

            default:  
                mmpResult = MMP_FAILURE;
                break;
        }

        vpu_ret = VPU_DecOpen(&m_DecHandle, &m_decOP);
	    if( vpu_ret != RETCODE_SUCCESS ) {
		    MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpDecoderVpu::Open] FAIL :  VPU_DecOpen (vpu_ret=%d) \n"), vpu_ret));
            mmpResult = MMP_FAILURE;
	    }
        else {
           // ret = VPU_DecGiveCommand(m_DecHandle, GET_DRAM_CONFIG, &dramCfg);
	       // if( ret != RETCODE_SUCCESS ) {
		   //     VLOG(ERR, "VPU_DecGiveCommand[GET_DRAM_CONFIG] failed Error code is 0x%x \n", ret );
		   //     goto ERR_DEC_OPEN;
	       // }

            if (m_decOP.bitstreamMode == BS_MODE_PIC_END) {
                VPU_DecSetRdPtr(m_DecHandle, m_decOP.bitstreamBuffer, 1);	
            }

            SecAxiUse		secAxiUse = {0};

            secAxiUse.useBitEnable  = USE_BIT_INTERNAL_BUF;
	        secAxiUse.useIpEnable   = USE_IP_INTERNAL_BUF;
	        secAxiUse.useDbkYEnable = USE_DBKY_INTERNAL_BUF;
	        secAxiUse.useDbkCEnable = USE_DBKC_INTERNAL_BUF;
	        secAxiUse.useBtpEnable  = USE_BTP_INTERNAL_BUF;
	        secAxiUse.useOvlEnable  = USE_OVL_INTERNAL_BUF;

	        VPU_DecGiveCommand(m_DecHandle, SET_SEC_AXI, &secAxiUse);


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
	        VPU_DecGiveCommand(m_DecHandle, SET_CACHE_CONFIG, &decCacheConfig);

        }
	}


    return mmpResult;
}


MMP_RESULT CMmpDecoderVpu::Close()
{

    MMPDEBUGMSG(1, (TEXT("[CMmpDecoderVpu::Close] ln=%d "), __LINE__ ));
    if(m_DecHandle != NULL) {

       MMPDEBUGMSG(1, (TEXT("[CMmpDecoderVpu::Close] ln=%d "), __LINE__ ));

       VPU_DecUpdateBitstreamBuffer(m_DecHandle, STREAM_END_SIZE);
       MMPDEBUGMSG(1, (TEXT("[CMmpDecoderVpu::Close] ln=%d "), __LINE__ ));

       MMPDEBUGMSG(1, (TEXT("[CMmpDecoderVpu::Close] ln=%d "), __LINE__ ));
	   VPU_DecClose(m_DecHandle);
       m_DecHandle = NULL;

       MMPDEBUGMSG(1, (TEXT("[CMmpDecoderVpu::Close] ln=%d "), __LINE__ ));

        if(m_vbStream.base != 0) {
            vdi_free_dma_memory(m_codec_idx, &m_vbStream);
            m_vbStream.base = 0;
        }

        MMPDEBUGMSG(1, (TEXT("[CMmpDecoderVpu::Close] ln=%d "), __LINE__ ));
        VPU_DeInit(m_codec_idx);
    }

#if 1
    MMPDEBUGMSG(1, (TEXT("[CMmpDecoderVpu::Close] ln=%d m_p_dsi_stream=0x%x "), __LINE__ , m_p_dsi_stream));

    if(m_p_dsi_stream != NULL) {
        delete [] m_p_dsi_stream;
        m_p_dsi_stream = NULL;
    }
    
#endif

    MMPDEBUGMSG(1, (TEXT("[CMmpDecoderVpu::Close] ln=%d "), __LINE__ ));

    return MMP_SUCCESS;
}

MMP_RESULT CMmpDecoderVpu::DecodeDSI_CheckStream_Mpeg4(MMP_U8* pStream, MMP_U32 nStreamSize) {

    /*
    MMP_RESULT mmpResult = MMP_SUCCESS;
    CMmpBitExtractor be(pStream);

    Decode_NextStartCodePrefix(&be, nStreamSize);
    */
    return MMP_SUCCESS;
}

MMP_RESULT CMmpDecoderVpu::DecodeDSI(MMP_U8* pStream, MMP_U32 nStreamSize) {

    MMP_RESULT mmpResult = MMP_SUCCESS;
    int size;
    RetCode vpu_ret;
    MMP_U8* p_dsi_stream = NULL;
    MMP_S32 dsi_stream_size, header_size;
    AVCodecContext *avc;
    AVCodecContext *cc= NULL;
    AVStream *st;
    MMP_U8* pbHeader;
    int frameRate = 0;
    
    MMPDEBUGMSG(1, (TEXT("[CMmpDecoderVpu::DecodeDSI] ln=%d sz=%d (%02x %02x %02x %02x, %02x %02x %02x %02x, %02x %02x %02x %02x, %02x %02x %02x %02x ) "), __LINE__, nStreamSize,
                   pStream[0], pStream[1], pStream[2], pStream[3], 
                   pStream[4], pStream[5], pStream[6], pStream[7], 
                   pStream[8], pStream[9], pStream[10], pStream[11], 
                   pStream[12], pStream[13], pStream[14], pStream[15] 
          ));

    p_dsi_stream = pStream;
    dsi_stream_size = nStreamSize;
    switch(this->m_create_config.nFormat) {

        case MMP_FOURCC_VIDEO_H264:
            if(CMmpH264Parser::Remake_VideoDSI_AVC1((unsigned char*)pStream, nStreamSize, (int*)&dsi_stream_size,
                NULL, NULL, NULL, NULL) == MMP_SUCCESS) {
           
                p_dsi_stream = pStream;

                MMPDEBUGMSG(1, (TEXT("[CMmpDecoderVpu::DecodeDSI-AVC1] ln=%d sz=%d (%02x %02x %02x %02x, %02x %02x %02x %02x, %02x %02x %02x %02x, %02x %02x %02x %02x ) "), __LINE__, nStreamSize,p_dsi_stream, 
                       pStream[0], pStream[1], pStream[2], pStream[3], 
                       pStream[4], pStream[5], pStream[6], pStream[7], 
                       pStream[8], pStream[9], pStream[10], pStream[11], 
                       pStream[12], pStream[13], pStream[14], pStream[15] 
                ));
            }
            break;

        case MMP_FOURCC_VIDEO_MSMPEG4V3:
            cc = (AVCodecContext *)pStream;
            st = (AVStream *)&pStream[sizeof(AVCodecContext)];
      
            m_p_dsi_stream = new MMP_U8[128];
            dsi_stream_size = 0;
            p_dsi_stream = m_p_dsi_stream;
            pbHeader = m_p_dsi_stream;

            PUT_LE32(pbHeader, MKTAG('C', 'N', 'M', 'V')); //signature 'CNMV'
            PUT_LE16(pbHeader, 0x00);                      //version
            PUT_LE16(pbHeader, 0x20);                      //length of header in bytes
            PUT_LE32(pbHeader, MKTAG('D', 'I', 'V', '3')); //codec FourCC
            PUT_LE16(pbHeader, cc->width);                //width
            PUT_LE16(pbHeader, cc->height);               //height
            PUT_LE32(pbHeader, st->r_frame_rate.num);      //frame rate
            PUT_LE32(pbHeader, st->r_frame_rate.den);      //time scale(?)
            PUT_LE32(pbHeader, st->nb_index_entries);      //number of frames in file
            PUT_LE32(pbHeader, 0); //unused
            dsi_stream_size += 32;		
            
            //PUT_BUFFER(pbHeader, pbMetaData, nMetaData);
            //size += nMetaData;
#if (MMP_OS == MMP_OS_WIN32)
            memcpy(&p_dsi_stream[dsi_stream_size], cc, sizeof(AVCodecContext) );
            dsi_stream_size += sizeof(AVCodecContext);
#endif
            
            break;

        case MMP_FOURCC_VIDEO_VC1:
        case MMP_FOURCC_VIDEO_WMV3:

            cc = (AVCodecContext *)pStream;
            st = (AVStream *)&pStream[sizeof(AVCodecContext)];
            avc = st->codec;
            cc->extradata = &pStream[sizeof(AVCodecContext)+sizeof(AVStream)];
      
            if (st->avg_frame_rate.den && st->avg_frame_rate.num)
                frameRate = (int)((double)st->avg_frame_rate.num/(double)st->avg_frame_rate.den);

            if (!frameRate && st->r_frame_rate.den && st->r_frame_rate.num)
                frameRate = (int)((double)st->r_frame_rate.num/(double)st->r_frame_rate.den);

#if (MMP_OS == MMP_OS_WIN32)
            m_p_dsi_stream = new MMP_U8[128 + cc->extradata_size*2 + sizeof(AVCodecContext)];
#else
            m_p_dsi_stream = new MMP_U8[128 + cc->extradata_size];
#endif
            dsi_stream_size = 0;
            p_dsi_stream = m_p_dsi_stream;
            pbHeader = m_p_dsi_stream;

#if (MMP_OS == MMP_OS_WIN32)
            memcpy(&p_dsi_stream[dsi_stream_size], cc, sizeof(AVCodecContext) );
            dsi_stream_size += sizeof(AVCodecContext);
            pbHeader += sizeof(AVCodecContext);

            memcpy(&p_dsi_stream[dsi_stream_size], cc->extradata, cc->extradata_size);
            dsi_stream_size += cc->extradata_size;
            pbHeader += cc->extradata_size;
#endif

            header_size = 0;
            PUT_LE32(pbHeader, ((0xC5 << 24)|0));
            header_size += 4; //version
            PUT_LE32(pbHeader, cc->extradata_size);
            header_size += 4;
            if(cc->extradata_size>0) {
                PUT_BUFFER(pbHeader, cc->extradata, cc->extradata_size);
                header_size += cc->extradata_size;
            }
            PUT_LE32(pbHeader, avc->height);
            header_size += 4;
            PUT_LE32(pbHeader, avc->width);
            header_size += 4;
            PUT_LE32(pbHeader, 12);
            header_size += 4;
            PUT_LE32(pbHeader, 2 << 29 | 1 << 28 | 0x80 << 24 | 1 << 0);
            header_size += 4; // STRUCT_B_FRIST (LEVEL:3|CBR:1:RESERVE:4:HRD_BUFFER|24)
            PUT_LE32(pbHeader, avc->bit_rate);
            header_size += 4; // hrd_rate
            PUT_LE32(pbHeader, frameRate);            
            header_size += 4; // frameRate

            dsi_stream_size+=header_size;
            break;

        case MMP_FOURCC_VIDEO_RV30:
        case MMP_FOURCC_VIDEO_RV40:

            cc = (AVCodecContext *)pStream;
            st = (AVStream *)&pStream[sizeof(AVCodecContext)];
            avc = st->codec;
            cc->extradata = &pStream[sizeof(AVCodecContext)+sizeof(AVStream)];
      
            if (st->avg_frame_rate.den && st->avg_frame_rate.num)
                frameRate = (int)((double)st->avg_frame_rate.num/(double)st->avg_frame_rate.den);

            if (!frameRate && st->r_frame_rate.den && st->r_frame_rate.num)
                frameRate = (int)((double)st->r_frame_rate.num/(double)st->r_frame_rate.den);

#if (MMP_OS == MMP_OS_WIN32)
            m_p_dsi_stream = new MMP_U8[128 + cc->extradata_size*2 + sizeof(AVCodecContext)];
#else
            m_p_dsi_stream = new MMP_U8[128 + cc->extradata_size];
#endif
            dsi_stream_size = 0;
            p_dsi_stream = m_p_dsi_stream;
            pbHeader = m_p_dsi_stream;

#if (MMP_OS == MMP_OS_WIN32)
            memcpy(&p_dsi_stream[dsi_stream_size], cc, sizeof(AVCodecContext) );
            dsi_stream_size += sizeof(AVCodecContext);
            pbHeader += sizeof(AVCodecContext);

            memcpy(&p_dsi_stream[dsi_stream_size], cc->extradata, cc->extradata_size);
            dsi_stream_size += cc->extradata_size;
            pbHeader += cc->extradata_size;
#endif

            header_size = 26 + cc->extradata_size;
            PUT_BE32(pbHeader, header_size); //Length
            PUT_LE32(pbHeader, MKTAG('V', 'I', 'D', 'O')); //MOFTag
            PUT_LE32(pbHeader, this->m_create_config.nFormat); //SubMOFTagl
            PUT_BE16(pbHeader, avc->width);
            PUT_BE16(pbHeader, avc->height);
            PUT_BE16(pbHeader, 0x0c); //BitCount;
            PUT_BE16(pbHeader, 0x00); //PadWidth;
            PUT_BE16(pbHeader, 0x00); //PadHeight;

            //PUT_LE32(pbHeader, frameRate);
		    PUT_BE32(pbHeader, frameRate<<16);
            PUT_BUFFER(pbHeader, cc->extradata, cc->extradata_size); //OpaqueDatata
            
            //size += st_size; //add for startcode pattern.
            dsi_stream_size+=header_size;
            break;

        case MMP_FOURCC_VIDEO_VP80:

            cc = (AVCodecContext *)pStream;
            st = (AVStream *)&pStream[sizeof(AVCodecContext)];
            avc = st->codec;
            cc->extradata = &pStream[sizeof(AVCodecContext)+sizeof(AVStream)];

#if (MMP_OS == MMP_OS_WIN32)
            m_p_dsi_stream = new MMP_U8[128 + cc->extradata_size*2 + sizeof(AVCodecContext)];
#else
            m_p_dsi_stream = new MMP_U8[128 + cc->extradata_size];
#endif
            dsi_stream_size = 0;
            p_dsi_stream = m_p_dsi_stream;
            pbHeader = m_p_dsi_stream;
            
            PUT_LE32(pbHeader, MKTAG('D', 'K', 'I', 'F')); //signature 'DKIF'
            PUT_LE16(pbHeader, 0x00);                      //version
            PUT_LE16(pbHeader, 0x20);                      //length of header in bytes
            PUT_LE32(pbHeader, MKTAG('V', 'P', '8', '0')); //codec FourCC
            PUT_LE16(pbHeader, avc->width);                //width
            PUT_LE16(pbHeader, avc->height);               //height
            PUT_LE32(pbHeader, st->r_frame_rate.num);      //frame rate
            PUT_LE32(pbHeader, st->r_frame_rate.den);      //time scale(?)
            PUT_LE32(pbHeader, st->nb_index_entries);      //number of frames in file
            PUT_LE32(pbHeader, 0); //unused
            header_size = 32;

            dsi_stream_size += header_size;

#if (MMP_OS == MMP_OS_WIN32)
            memcpy(&p_dsi_stream[dsi_stream_size], cc, sizeof(AVCodecContext) );
            dsi_stream_size += sizeof(AVCodecContext);
            pbHeader += sizeof(AVCodecContext);

            if(cc->extradata_size > 0) {
                memcpy(&p_dsi_stream[dsi_stream_size], cc->extradata, cc->extradata_size);
                dsi_stream_size += cc->extradata_size;
                pbHeader += cc->extradata_size;
            }
#endif
            break;

        default:

            p_dsi_stream = pStream;
            dsi_stream_size = nStreamSize;
    }

    MMPDEBUGMSG(0, (TEXT("[CMmpDecoderVpu::DecodeDSI] ln=%d sz=%d (%02x %02x %02x %02x, %02x %02x %02x %02x, %02x %02x %02x %02x, %02x %02x %02x %02x ) "), __LINE__, dsi_stream_size,
                   p_dsi_stream[0], p_dsi_stream[1], p_dsi_stream[2], p_dsi_stream[3], 
                   p_dsi_stream[4], p_dsi_stream[5], p_dsi_stream[6], p_dsi_stream[7], 
                   p_dsi_stream[8], p_dsi_stream[9], p_dsi_stream[10], p_dsi_stream[11], 
                   p_dsi_stream[12], p_dsi_stream[13], p_dsi_stream[14], p_dsi_stream[15] 
          ));

    /* Input DSI Stream */
    if(mmpResult == MMP_SUCCESS) {
        size = WriteBsBufFromBufHelper(m_codec_idx, m_DecHandle, &m_vbStream, p_dsi_stream, dsi_stream_size, m_decOP.streamEndian);
	    if (size <0) {
            MMPDEBUGMSG(1, (TEXT("[CMmpDecoderVpu::DecodeDSI] ln=%d FAIL: WriteBsBufFromBufHelper "), __LINE__));
		    //VLOG(ERR, "WriteBsBufFromBufHelper failed Error code is 0x%x \n", size );
		    //goto ERR_DEC_OPEN;
            mmpResult = MMP_FAILURE;
	    }
    }

    /* RUN Seq Init */
    if(mmpResult == MMP_SUCCESS) {

        if(m_decOP.bitstreamMode == BS_MODE_PIC_END)
	    {
		    vpu_ret = VPU_DecGetInitialInfo(m_DecHandle, &m_dec_init_info);
		    if(vpu_ret != RETCODE_SUCCESS) {

                MMPDEBUGMSG(1, (TEXT("[CMmpDecoderVpu::DecodeDSI] FAIL: VPU_DecGetInitialInfo ln=%d "), __LINE__));
    #ifdef SUPPORT_MEM_PROTECT
			    //if (ret == RETCODE_MEMORY_ACCESS_VIOLATION)
			    //	PrintMemoryAccessViolationReason(coreIdx, NULL); 
    #endif
			    //VLOG(ERR, "VPU_DecGetInitialInfo failed Error code is 0x%x \n", ret);
			    //goto ERR_DEC_OPEN;

                mmpResult = MMP_FAILURE;
		    }
		    VPU_ClearInterrupt(m_codec_idx);
	    }

    }

    /* Framebuffer Register */
    if(mmpResult == MMP_SUCCESS) {
    
        FrameBuffer *pUserFrame = NULL;
        int framebufStride = MMP_BYTE_ALIGN(m_dec_init_info.picWidth, 16);
        int framebufHeight = MMP_BYTE_ALIGN(m_dec_init_info.picHeight, 16);

        m_regFrameBufCount = m_dec_init_info.minFrameBufferCount + EXTRA_FRAME_BUFFER_NUM;

        vpu_ret = VPU_DecRegisterFrameBuffer(m_DecHandle, pUserFrame, m_regFrameBufCount, framebufStride, framebufHeight, m_mapType);
        if(vpu_ret != RETCODE_SUCCESS)  {


            MMPDEBUGMSG(1, (TEXT("[CMmpDecoderVpu::DecodeDSI] FAIL: VPU_DecRegisterFrameBuffer ")));
            mmpResult = MMP_FAILURE;
        }
    
    }

    return mmpResult;
}

MMP_RESULT CMmpDecoderVpu::DecodeAu_StreamRemake_AVC1(CMmpMediaSample* pMediaSample) {

    int remain_size, stream_size;
    unsigned int* ps;
    unsigned char* pau;

    pau = pMediaSample->pAu;
    remain_size = pMediaSample->uiAuSize;
    while(remain_size > 0) {

        ps = (unsigned int*)pau;
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
    
    return MMP_SUCCESS;
}

MMP_RESULT CMmpDecoderVpu::DecodeAu_PinEnd(CMmpMediaSample* pMediaSample, CMmpMediaSampleDecodeResult* pDecResult) {

    MMP_RESULT mmpResult = MMP_SUCCESS;
    RetCode vpu_ret;
    DecParam		decParam	= {0};
    int int_reason, size, i;
    MMP_U32 start_tick, t1, t2, t3 ,t4;
    MMP_U8* pStream, *pHeader;
    MMP_S32 nStreamSize;
    MMP_U8* pRemakeStream = NULL;

    start_tick = CMmpUtil::GetTickCount();

    decParam.iframeSearchEnable = 0;
    decParam.skipframeMode = 0;
    decParam.DecStdParam.mp2PicFlush = 0;

    pStream = pMediaSample->pAu;
    nStreamSize = pMediaSample->uiAuSize;

    MMPDEBUGMSG(0, (TEXT("[CMmpDecoderVpu::DecodeAu_PinEnd] ln=%d sz=%d (%02x %02x %02x %02x, %02x %02x %02x %02x, %02x %02x %02x %02x, %02x %02x %02x %02x ) "), __LINE__, pMediaSample->uiAuSize,
                   pMediaSample->pAu[0], pMediaSample->pAu[1], pMediaSample->pAu[2], pMediaSample->pAu[3], 
                   pMediaSample->pAu[4], pMediaSample->pAu[5], pMediaSample->pAu[6], pMediaSample->pAu[7], 
                   pMediaSample->pAu[8], pMediaSample->pAu[9], pMediaSample->pAu[10], pMediaSample->pAu[11], 
                   pMediaSample->pAu[12], pMediaSample->pAu[13], pMediaSample->pAu[14], pMediaSample->pAu[15] 
          ));


    switch(this->m_create_config.nFormat) {

        case MMP_FOURCC_VIDEO_H264: 
            this->DecodeAu_StreamRemake_AVC1(pMediaSample); 
            break;

        case MMP_FOURCC_VIDEO_MSMPEG4V3: 
        case MMP_FOURCC_VIDEO_VP80: 

            nStreamSize = pMediaSample->uiAuSize +12;
            pRemakeStream = new MMP_U8[nStreamSize];
            pStream = pRemakeStream;
            pHeader = pStream;
            
            PUT_LE32(pHeader, pMediaSample->uiAuSize);
            PUT_LE32(pHeader,0);
            PUT_LE32(pHeader,0);
            //size += 12;
            memcpy(pHeader, pMediaSample->pAu, pMediaSample->uiAuSize);
            break;

        case MMP_FOURCC_VIDEO_VC1:
        case MMP_FOURCC_VIDEO_WMV3:

            nStreamSize = pMediaSample->uiAuSize +8;
            pRemakeStream = new MMP_U8[nStreamSize];
            pStream = pRemakeStream;
            pHeader = pStream;
            
            //write size&keyframe
            if((pMediaSample->uiFlag&MMP_MEDIASAMPMLE_FLAG_VIDEO_KEYFRAME) != 0) {
                PUT_LE32(pHeader, pMediaSample->uiAuSize | 0x80000000 );
            }
            else {
                PUT_LE32(pHeader, pMediaSample->uiAuSize | 0x00000000 );
            }

            //Write TimeStamp
            if(AV_NOPTS_VALUE == pMediaSample->uiTimeStamp)
            {
                PUT_LE32(pHeader, 0);
            }
            else
            {
                PUT_LE32(pHeader, pMediaSample->uiTimeStamp/1000); // milli_sec
            }

            memcpy(pHeader, pMediaSample->pAu, pMediaSample->uiAuSize);
            break;


        case MMP_FOURCC_VIDEO_RV30:
        case MMP_FOURCC_VIDEO_RV40:

            {
                char cSlice;
                int nSlice, offset;
                unsigned int val;
                int header_size;
            
                cSlice = pMediaSample->pAu[0] + 1;
                nSlice = pMediaSample->uiAuSize - 1 - (cSlice * 8);
                header_size = 20 + (cSlice*8);
                nStreamSize = header_size + pMediaSample->uiAuSize;

                pRemakeStream = new MMP_U8[nStreamSize];
                pStream = pRemakeStream;
                pHeader = pStream;
                
                PUT_BE32(pHeader, nSlice);
                if(AV_NOPTS_VALUE == pMediaSample->uiTimeStamp)
                {
                    PUT_LE32(pHeader, 0);
                }
                else
                {
                    PUT_LE32(pHeader, (int)(pMediaSample->uiTimeStamp/1000)); // milli_sec
                }
                PUT_BE16(pHeader, pMediaSample->uiSampleNumber);
                PUT_BE16(pHeader, 0x02); //Flags
                PUT_BE32(pHeader, 0x00); //LastPacket
                PUT_BE32(pHeader, cSlice); //NumSegments
                offset = 1;
                for (i = 0; i < (int)cSlice; i++)
                {
                    val = (pMediaSample->pAu[offset+3] << 24) | (pMediaSample->pAu[offset+2] << 16) | (pMediaSample->pAu[offset+1] << 8) | pMediaSample->pAu[offset];
                    PUT_BE32(pHeader, val); //isValid
                    offset += 4;
                    val = (pMediaSample->pAu[offset+3] << 24) | (pMediaSample->pAu[offset+2] << 16) | (pMediaSample->pAu[offset+1] << 8) | pMediaSample->pAu[offset];
                    PUT_BE32(pHeader, val); //Offset
                    offset += 4;
                }

                //int cSlice = chunkData[0] + 1;
				//int nSlice =  chunkSize - 1 - (cSlice * 8);
				//chunkData += (1+(cSlice*8));
                memcpy(&pStream[header_size], &pMediaSample->pAu[1+(cSlice*8)], pMediaSample->uiAuSize-(1+(cSlice*8)) );
				nStreamSize = nSlice + header_size;//chunkSize = nSlice;
                
#if (MMP_OS == MMP_OS_WIN32)
                memcpy(&pStream[header_size], pMediaSample->pAu, pMediaSample->uiAuSize);
                nStreamSize = header_size + pMediaSample->uiAuSize;
#endif

                MMPDEBUGMSG(1, (TEXT("[CMmpDecoderVpu::DecodeAu_PinEnd] ln=%d RV40 nStreamSize=%d nSlice=%d "), __LINE__, nStreamSize, nSlice));
            }
            break;


        default:
            pStream = pMediaSample->pAu;
            nStreamSize = pMediaSample->uiAuSize;
    }
    
    size = WriteBsBufFromBufHelper(m_codec_idx, m_DecHandle, &m_vbStream, pStream, nStreamSize, m_decOP.streamEndian);
	if (size <0)
	{
		//VLOG(ERR, "WriteBsBufFromBufHelper failed Error code is 0x%x \n", size );
		//goto ERR_DEC_OPEN;
        mmpResult = MMP_FAILURE;
        MMPDEBUGMSG(1, (TEXT("[CMmpDecoderVpu::DecodeAu_PinEnd] ln=%d FAIL: WriteBsBufFromBufHelper"), __LINE__));
	}
    t1 = CMmpUtil::GetTickCount();
    
	// Start decoding a frame.
	vpu_ret = VPU_DecStartOneFrame(m_DecHandle, &decParam);
	if (vpu_ret != RETCODE_SUCCESS) 
	{
		//VLOG(ERR,  "VPU_DecStartOneFrame failed Error code is 0x%x \n", ret);
		//goto ERR_DEC_OPEN;
        mmpResult = MMP_FAILURE;
        MMPDEBUGMSG(1, (TEXT("[CMmpDecoderVpu::DecodeAu_PinEnd] ln=%d FAIL: VPU_DecStartOneFrame"), __LINE__));
	}

    t2 = CMmpUtil::GetTickCount();

    while(mmpResult == MMP_SUCCESS) {
    
        int_reason = VPU_WaitInterrupt(m_codec_idx, VPU_DEC_TIMEOUT);
        if (int_reason == (Uint32)-1) // timeout
		{
            MMPDEBUGMSG(1, (TEXT("[CMmpDecoderVpu::DecodeAu_PinEnd] ln=%d  FAIL: VPU_WaitInterrupt   TimeOut "), __LINE__));
			VPU_SWReset(m_codec_idx, SW_RESET_SAFETY, m_DecHandle);				
			mmpResult = MMP_FAILURE;
            break;
		}		

        //MMPDEBUGMSG(1, (TEXT("[CMmpDecoderVpu::DecodeAu_PinEnd] ln=%d int_reason=%d"), __LINE__, int_reason));
        if (int_reason)
    		VPU_ClearInterrupt(m_codec_idx);

    
        if (int_reason & (1<<INT_BIT_PIC_RUN)) 
				break;		
    }
    t3 = CMmpUtil::GetTickCount();

    if(mmpResult == MMP_SUCCESS) {

        memset(&m_output_info, 0x00, sizeof(m_output_info));
        vpu_ret = VPU_DecGetOutputInfo(m_DecHandle, &m_output_info);

        if(vpu_ret == RETCODE_SUCCESS) {
        
            
            if(m_output_info.indexFrameDisplay >= 0) {
            
                //int luma_size, chroma_size;
                FrameBuffer frameBuf;
                VPU_DecGetFrameBuffer(m_DecHandle, m_output_info.indexFrameDisplay, &frameBuf);

                t4 = CMmpUtil::GetTickCount();

                MMPDEBUGMSG(0, (TEXT("[CMmpDecoderVpu::DecodeAu_PinEnd] ln=%d dur=(%d %d %d %d) m_output_info.indexFrameDisplay=%d   bufYCbCr(0x%08x 0x%08x 0x%08x) "), __LINE__, 
                                                            (t1-start_tick),(t2-start_tick),(t3-start_tick),(t4-start_tick), 
                                                            m_output_info.indexFrameDisplay,
                                                            frameBuf.bufY, frameBuf.bufCb, frameBuf.bufCr
                                                            ));
                

                pDecResult->uiDecodedSize = (m_output_info.dispPicWidth*m_output_info.dispPicHeight*3)/2;
                pDecResult->bImage = MMP_TRUE;
                pDecResult->uiTimeStamp = pMediaSample->uiTimeStamp;

#if 0//(MMP_OS == MMP_OS_WIN32)
                luma_size = MMP_BYTE_ALIGN(m_dec_init_info.picWidth, 16) * MMP_BYTE_ALIGN(m_dec_init_info.picHeight, 16);
                chroma_size = luma_size/4;
                vdi_read_memory(m_codec_idx, frameBuf.bufY, (unsigned char*)pDecResult->uiDecodedBufferLogAddr[MMP_DECODED_BUF_Y],  luma_size, m_decOP.frameEndian);
                vdi_read_memory(m_codec_idx, frameBuf.bufCb, (unsigned char*)pDecResult->uiDecodedBufferLogAddr[MMP_DECODED_BUF_U],  chroma_size, m_decOP.frameEndian);
                vdi_read_memory(m_codec_idx, frameBuf.bufCr, (unsigned char*)pDecResult->uiDecodedBufferLogAddr[MMP_DECODED_BUF_V],  chroma_size, m_decOP.frameEndian);
#else
                unsigned int key=0xAAAA9829;
				unsigned int value, addr;

                memcpy((unsigned char*)pDecResult->uiDecodedBufferLogAddr[MMP_DECODED_BUF_Y], &frameBuf, sizeof(frameBuf));

                memcpy((unsigned char*)pDecResult->uiDecodedBufferLogAddr[MMP_DECODED_BUF_U], &key, sizeof(unsigned int));

				addr = pDecResult->uiDecodedBufferLogAddr[MMP_DECODED_BUF_V];
				value = key; 
				memcpy((unsigned char*)addr, &value, sizeof(unsigned int));
				
				addr += sizeof(unsigned int);
				value = (unsigned int)this; 
				memcpy((unsigned char*)addr, &value, sizeof(unsigned int));
				
				addr += sizeof(unsigned int);
				value = (unsigned int)CMmpDecoderVpu::vdi_memcpy_stub; 
				memcpy((unsigned char*)addr, &value, sizeof(unsigned int));
#endif

                
                VPU_DecClrDispFlag(m_DecHandle, m_output_info.indexFrameDisplay);
            }
            else {
                MMPDEBUGMSG(1, (TEXT("[CMmpDecoderVpu::DecodeAu_PinEnd] ln=%d m_output_info.indexFrameDisplay=%d "), __LINE__, m_output_info.indexFrameDisplay));
            }
            
            
        }
        else {
            mmpResult = MMP_FAILURE;
            MMPDEBUGMSG(1, (TEXT("[CMmpDecoderVpu::DecodeAu_PinEnd] ln=%d FAIL: VPU_DecGetOutputInfo "), __LINE__));
        }
		
    }

    if(pRemakeStream != NULL) {
        delete [] pRemakeStream;
    }
    return mmpResult;
}

void CMmpDecoderVpu::vdi_memcpy_stub(void* param, void* dest_vaddr, void* src_paddr, int size) {
	CMmpDecoderVpu* pMmpDecoderVpu=(CMmpDecoderVpu*)param;
	vdi_read_memory(pMmpDecoderVpu->m_codec_idx, (PhysicalAddress)src_paddr, (unsigned char*)dest_vaddr, size, pMmpDecoderVpu->m_decOP.frameEndian);
}

void CMmpDecoderVpu::make_decOP_Common() {
    
    
    m_mapType = 0; //Map Type 0(Linear) / 1(Frame-V) / 2(Frame-H) / 3(Field-V) / 4(Mix-V) / 5(Frame-MB) / 6(Filed-MB) / 16(Tile2Linear)
        
    m_decOP.bitstreamFormat = (CodStd)STD_AVC_DEC; //0(H.264) / 1(VC1) / 2(MPEG2) / 3(MPEG4) / 4(H263) / 5(DIVX3) / 6(RV) / 7(AVS) / 11(VP8)
    m_decOP.avcExtension = 0;  // AVC extension 0(No) / 1(MVC) 
    m_decOP.coreIdx = m_codec_idx; 
    m_decOP.bitstreamBuffer = m_vbStream.phys_addr;
	m_decOP.bitstreamBufferSize = m_vbStream.size;
	m_decOP.mp4DeblkEnable = 0;
	m_decOP.mp4Class = 0; //MPEG4 CLASS 0(MPEG4) / 1(DIVX 5.0 or higher) / 2(XVID) / 5(DIVX 4.0) / 8(DIVX/XVID Auto Detect)/ 256(Sorenson spark) :
	m_decOP.tiled2LinearEnable = (m_mapType>>4)&0x1;
    m_decOP.bitstreamMode = BS_MODE_PIC_END; //Bitstream Mode(0: Interrupt mode, 1: Rollback mode, 2: PicEnd mode) 
    m_decOP.cbcrInterleave = CBCR_INTERLEAVE;
   	m_decOP.bwbEnable	  = VPU_ENABLE_BWB;
    m_decOP.frameEndian	= VPU_FRAME_ENDIAN;
	m_decOP.streamEndian   = VPU_STREAM_ENDIAN;
}

void CMmpDecoderVpu::make_decOP_H263() {

    this->make_decOP_Common();

    m_decOP.bitstreamFormat = (CodStd)STD_H263_DEC; //0(H.264) / 1(VC1) / 2(MPEG2) / 3(MPEG4) / 4(H263) / 5(DIVX3) / 6(RV) / 7(AVS) / 11(VP8)
    m_decOP.mp4DeblkEnable = 0;
	m_decOP.mp4Class = 0; //MPEG4 CLASS 0(MPEG4) / 1(DIVX 5.0 or higher) / 2(XVID) / 5(DIVX 4.0) / 8(DIVX/XVID Auto Detect)/ 256(Sorenson spark) :
	
}

void CMmpDecoderVpu::make_decOP_H264() {
    
    this->make_decOP_Common();

    m_decOP.bitstreamFormat = (CodStd)STD_AVC_DEC; //0(H.264) / 1(VC1) / 2(MPEG2) / 3(MPEG4) / 4(H263) / 5(DIVX3) / 6(RV) / 7(AVS) / 11(VP8)
    m_decOP.avcExtension = 0;  // AVC extension 0(No) / 1(MVC) 
    
	
}

void CMmpDecoderVpu::make_decOP_MPEG4() {

    this->make_decOP_Common();

    m_decOP.bitstreamFormat = (CodStd)STD_MP4_DEC; //0(H.264) / 1(VC1) / 2(MPEG2) / 3(MPEG4) / 4(H263) / 5(DIVX3) / 6(RV) / 7(AVS) / 11(VP8)
    m_decOP.mp4DeblkEnable = 1;
	m_decOP.mp4Class = 0; //MPEG4 CLASS 0(MPEG4) / 1(DIVX 5.0 or higher) / 2(XVID) / 5(DIVX 4.0) / 8(DIVX/XVID Auto Detect)/ 256(Sorenson spark) :
	
}

void CMmpDecoderVpu::make_decOP_MPEG2() {

    this->make_decOP_Common();

    m_decOP.bitstreamFormat = (CodStd)STD_MP2_DEC; //0(H.264) / 1(VC1) / 2(MPEG2) / 3(MPEG4) / 4(H263) / 5(DIVX3) / 6(RV) / 7(AVS) / 11(VP8)
    m_decOP.mp4DeblkEnable = 0;
	m_decOP.mp4Class = 0; //MPEG4 CLASS 0(MPEG4) / 1(DIVX 5.0 or higher) / 2(XVID) / 5(DIVX 4.0) / 8(DIVX/XVID Auto Detect)/ 256(Sorenson spark) :
	
}

void CMmpDecoderVpu::make_decOP_VC1() {

    this->make_decOP_Common();

    m_decOP.bitstreamFormat = STD_VC1; //0(H.264) / 1(VC1) / 2(MPEG2) / 3(MPEG4) / 4(H263) / 5(DIVX3) / 6(RV) / 7(AVS) / 11(VP8)
    m_decOP.mp4DeblkEnable = 0;
	m_decOP.mp4Class = 0; //MPEG4 CLASS 0(MPEG4) / 1(DIVX 5.0 or higher) / 2(XVID) / 5(DIVX 4.0) / 8(DIVX/XVID Auto Detect)/ 256(Sorenson spark) :
	
}

void CMmpDecoderVpu::make_decOP_MSMpeg4V3() {

    this->make_decOP_Common();

    m_decOP.bitstreamFormat = (CodStd)STD_DIV3; //0(H.264) / 1(VC1) / 2(MPEG2) / 3(MPEG4) / 4(H263) / 5(DIVX3) / 6(RV) / 7(AVS) / 11(VP8)
    m_decOP.mp4DeblkEnable = 0;
	m_decOP.mp4Class = 0; //MPEG4 CLASS 0(MPEG4) / 1(DIVX 5.0 or higher) / 2(XVID) / 5(DIVX 4.0) / 8(DIVX/XVID Auto Detect)/ 256(Sorenson spark) :
	
}

void CMmpDecoderVpu::make_decOP_RV30() {

    this->make_decOP_RV40();
}

void CMmpDecoderVpu::make_decOP_RV40() {

    this->make_decOP_Common();

    m_decOP.bitstreamFormat = STD_RV; //0(H.264) / 1(VC1) / 2(MPEG2) / 3(MPEG4) / 4(H263) / 5(DIVX3) / 6(RV) / 7(AVS) / 11(VP8)
    m_decOP.mp4DeblkEnable = 0;
	m_decOP.mp4Class = 0; //MPEG4 CLASS 0(MPEG4) / 1(DIVX 5.0 or higher) / 2(XVID) / 5(DIVX 4.0) / 8(DIVX/XVID Auto Detect)/ 256(Sorenson spark) :
	
}

void CMmpDecoderVpu::make_decOP_VP80() {

    this->make_decOP_Common();

    m_decOP.bitstreamFormat = STD_VP8; //0(H.264) / 1(VC1) / 2(MPEG2) / 3(MPEG4) / 4(H263) / 5(DIVX3) / 6(RV) / 7(AVS) / 11(VP8)
    m_decOP.mp4DeblkEnable = 0;
	m_decOP.mp4Class = 0; //MPEG4 CLASS 0(MPEG4) / 1(DIVX 5.0 or higher) / 2(XVID) / 5(DIVX 4.0) / 8(DIVX/XVID Auto Detect)/ 256(Sorenson spark) :
	
}

void CMmpDecoderVpu::make_user_frame() {

#if 0
    FrameBufferAllocInfo fbAllocInfo;
    int framebufStride, framebufHeight;
    DRAMConfig dramCfg = {0};
    FrameBuffer  fbUser[MAX_REG_FRAME]={0,};

    framebufStride = MMP_BYTE_ALIGN(m_dec_init_info.picWidth, 16);
    framebufHeight = MMP_BYTE_ALIGN(m_dec_init_info.picHeight, 16);

    fbAllocInfo.format          = FORMAT_420;
	fbAllocInfo.cbcrInterleave  = m_decOP.cbcrInterleave;
	fbAllocInfo.mapType         = m_mapType;
	fbAllocInfo.stride          = framebufStride;
	fbAllocInfo.height          = framebufHeight;
	fbAllocInfo.num             = m_regFrameBufCount;
	
	fbAllocInfo.endian          = m_decOP.frameEndian;
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
