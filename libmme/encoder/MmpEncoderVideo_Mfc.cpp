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

#include "MmpEncoderVideo_Mfc.hpp"
#include "MmpUtil.hpp"


extern "C"  {
#include "color_space_convertor.h"
}

/////////////////////////////////////////////////////////////
//CMmpEncoderVideo_Mfc Member Functions

CMmpEncoderVideo_Mfc::CMmpEncoderVideo_Mfc(struct MmpEncoderCreateConfig *pCreateConfig) : CMmpEncoderVideo(pCreateConfig, MMP_FALSE), CMmpEncoderMfc(pCreateConfig)
,m_nEncodedStreamCount(0)
,m_nInputFrameCount(0)
{
    
}

CMmpEncoderVideo_Mfc::~CMmpEncoderVideo_Mfc()
{

}

MMP_RESULT CMmpEncoderVideo_Mfc::Open()
{
    MMP_RESULT mmpResult;
    
    mmpResult=CMmpEncoderVideo::Open();
    if(mmpResult!=MMP_SUCCESS)
    {
        return mmpResult;
    }

    mmpResult=CMmpEncoderMfc::Open();
    if(mmpResult!=MMP_SUCCESS)
    {
        return mmpResult;
    }

    sprintf((char*)m_szCodecName, "%c%c%c%c", MMPGETFOURCC(m_nFormat,0), MMPGETFOURCC(m_nFormat,1), MMPGETFOURCC(m_nFormat,2), MMPGETFOURCC(m_nFormat,3));

    MMPDEBUGMSG(MMPZONE_MONITOR, (TEXT("[CMmpEncoderVideo_Mfc::Open] Success nForamt=(0x%08x %s) \n\r"), 
                  m_nFormat, m_szCodecName ));

    
    return MMP_SUCCESS;
}


MMP_RESULT CMmpEncoderVideo_Mfc::Close()
{
    MMP_RESULT mmpResult;
    
    mmpResult=CMmpEncoderMfc::Close();
    if(mmpResult!=MMP_SUCCESS)
    {
        MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpEncoderVideo_Mfc::Close] CMmpEncoderFfmpeg::Close() \n\r")));
        return mmpResult;
    }

    mmpResult=CMmpEncoderVideo::Close();
    if(mmpResult!=MMP_SUCCESS)
    {
        MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpEncoderVideo_Mfc::Close] CMmpEncoderVideo::Close() \n\r")));
        return mmpResult;
    }
    
    MMPDEBUGMSG(MMPZONE_MONITOR, (TEXT("[CMmpEncoderVideo_Mfc::Close] Success nForamt=(0x%08x %s) \n\r"), 
                  m_nFormat, m_szCodecName ));


    return MMP_SUCCESS;
}

#if 0
MMP_RESULT CMmpEncoderVideo_Mfc::EncodeAu(CMmpMediaSampleEncode* pMediaSample, CMmpMediaSampleEncodeResult* pEncResult) {

    SSBSIP_MFC_ERROR_CODE mfc_ret;
    SSBSIP_MFC_ENC_OUTPUT_INFO mfc_outputInfo;
    SSBSIP_MFC_ENC_INPUT_INFO mfc_inputinfo;
    SSBSIP_MFC_ENC_INPUT_INFO *p_mfc_inputinfo = &mfc_inputinfo;
    
    MMP_RESULT mmpResult = MMP_FAILURE;
    MMP_U32 i;
    MMP_U32 enc_start_tick, enc_end_tick;

    MMP_U8* pTemp;
    MMP_U8* pEncBuffer;
    MMP_U32 nEncBufSize, nEncBufMaxSize, nEncFlag;
    MMP_U32 buf_width, buf_height;

    enc_start_tick = CMmpUtil::GetTickCount();

    pEncResult->uiEncodedStreamSize[0] = 0;
    pEncResult->uiEncodedStreamSize[1] = 0;
    pEncResult->uiTimeStamp = pMediaSample->uiTimeStamp;
    pEncResult->uiFlag = 0;
    
    /*
    if(m_pAVCodecContext == NULL) {
    
        mmpResult = CMmpEncoderMfc::EncodeDSI(pMediaSample, pEncResult);
        if(mmpResult != MMP_SUCCESS) {
            return mmpResult;
        }
    }
    */

#ifndef WIN32
    if(m_nEncodedStreamCount == 0) {

        mfc_ret = SsbSipMfcEncGetOutBuf(m_hMFCHandle, &mfc_outputInfo);
        if(mfc_ret != MFC_RET_OK)
        {
            MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpEncoderVideo_Mfc::EncodeAu] GetConfig Failed !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! ")));
            //return MMP_FAILURE;
        }
        else {
            this->EncodedFrameQueue_AddFrame((MMP_U8*)mfc_outputInfo.StrmVirAddr, mfc_outputInfo.headerSize, MMP_ENCODED_FLAG_VIDEO_CONFIGDATA);

            pTemp = (MMP_U8*)mfc_outputInfo.StrmVirAddr;
            MMPDEBUGMSG(0, (TEXT("[CMmpEncoderVideo_Mfc::EncodeAu] ln=%d Sz=(%d %d) (%02x %02x %02x %02x %02x %02x %02x %02x )"), __LINE__, 
                    mfc_outputInfo.headerSize, mfc_outputInfo.dataSize,
                   (unsigned int)pTemp[0],(unsigned int)pTemp[1],(unsigned int)pTemp[2],(unsigned int)pTemp[3],
                   (unsigned int)pTemp[4],(unsigned int)pTemp[5],(unsigned int)pTemp[6],(unsigned int)pTemp[7]
                 ));
        }
    }
#endif

    memcpy(p_mfc_inputinfo, &this->m_mfc_input_info[0], sizeof(SSBSIP_MFC_ENC_INPUT_INFO) );
    
    buf_width = MMP_BYTE_ALIGN(m_bih_in.biWidth, 16);
    buf_height = MMP_BYTE_ALIGN(m_bih_in.biHeight, 16);

    
    MMPDEBUGMSG(0, (TEXT("[CMmpEncoderVideo_Mfc::EncodeAu] MediaSample(Phy:0x%08x 0x%08x ) MfcIn(Phy: 0x%08x 0x%08x Vir: 0x%08x 0x%08x Sz:%d %d ) "),
        pMediaSample->uiBufferPhyAddr[MMP_DECODED_BUF_Y], pMediaSample->uiBufferPhyAddr[MMP_DECODED_BUF_U],
        p_mfc_inputinfo->YPhyAddr, p_mfc_inputinfo->CPhyAddr,
        p_mfc_inputinfo->YVirAddr, p_mfc_inputinfo->CVirAddr,
        p_mfc_inputinfo->YSize, p_mfc_inputinfo->CSize
        ));

             

    if( (pMediaSample->uiBufferPhyAddr[MMP_DECODED_BUF_Y] == 0) 
        && (pMediaSample->uiBufferPhyAddr[MMP_DECODED_BUF_U] == 0) ) {
    
            p_mfc_inputinfo->YSize = buf_width*buf_height;
            p_mfc_inputinfo->CSize = p_mfc_inputinfo->YSize/2;
            p_mfc_inputinfo->y_cookie = 0;
            p_mfc_inputinfo->c_cookie = 0;

            memcpy(p_mfc_inputinfo->YVirAddr, (void*)pMediaSample->uiBufferLogAddr[MMP_DECODED_BUF_Y], p_mfc_inputinfo->YSize);
            if(pMediaSample->pixelformat == MMP_PIXELFORMAT_YUV420_PLANAR) {
                csc_linear_to_tiled_uv((unsigned char*)p_mfc_inputinfo->CVirAddr,
                                       (unsigned char*)pMediaSample->uiBufferLogAddr[MMP_DECODED_BUF_U],
                                       (unsigned char*)pMediaSample->uiBufferLogAddr[MMP_DECODED_BUF_V],
                                       buf_width,  /* uv width */
                                       buf_height/2  /* uv height */
                                       );

            }
            else {
                memcpy(p_mfc_inputinfo->CVirAddr, (void*)pMediaSample->uiBufferLogAddr[MMP_DECODED_BUF_U], p_mfc_inputinfo->CSize);
            }

    }
    else {
        p_mfc_inputinfo->YPhyAddr = (void*)pMediaSample->uiBufferPhyAddr[MMP_DECODED_BUF_Y]; //pVideoEnc->MFCEncInputBuffer[pVideoEnc->indexInputBuffer].YPhyAddr;
        p_mfc_inputinfo->CPhyAddr = (void*)pMediaSample->uiBufferPhyAddr[MMP_DECODED_BUF_U]; //pVideoEnc->MFCEncInputBuffer[pVideoEnc->indexInputBuffer].CPhyAddr;
        p_mfc_inputinfo->YVirAddr = (void*)pMediaSample->uiBufferLogAddr[MMP_DECODED_BUF_Y]; //pVideoEnc->MFCEncInputBuffer[pVideoEnc->indexInputBuffer].YVirAddr;
        p_mfc_inputinfo->CVirAddr = (void*)pMediaSample->uiBufferLogAddr[MMP_DECODED_BUF_U]; //pVideoEnc->MFCEncInputBuffer[pVideoEnc->indexInputBuffer].CVirAddr;

        p_mfc_inputinfo->YSize = buf_width*buf_height;
        p_mfc_inputinfo->CSize = p_mfc_inputinfo->YSize/2;
        
        p_mfc_inputinfo->y_cookie = 0;
        p_mfc_inputinfo->c_cookie = 0;
    }
        
    mfc_ret = SsbSipMfcEncSetInBuf(m_hMFCHandle, p_mfc_inputinfo);
    if(mfc_ret == MFC_RET_OK) {

        mfc_ret = SsbSipMfcEncExe(m_hMFCHandle);
        if(mfc_ret == MFC_RET_OK) /* Success */ {
            
            mfc_ret = SsbSipMfcEncGetOutBuf(m_hMFCHandle, &mfc_outputInfo);
            if(mfc_ret == MFC_RET_OK) /* Success */ {
            
                pEncBuffer = (MMP_U8*)pEncResult->uiEncodedBufferLogAddr[MMP_ENCODED_BUF_STREAM];
                nEncBufMaxSize = pEncResult->uiEncodedBufferMaxSize[MMP_ENCODED_BUF_STREAM];
                nEncBufSize = mfc_outputInfo.dataSize;

                pTemp = (MMP_U8*)mfc_outputInfo.StrmVirAddr;
                MMPDEBUGMSG(0, (TEXT("[CMmpEncoderVideo_Mfc::EncodeAu] ln=%d Sz=%d Type=%d (%02x %02x %02x %02x %02x %02x %02x %02x )"), __LINE__, 
                    mfc_outputInfo.dataSize,
                    mfc_outputInfo.frameType,
                   (unsigned int)pTemp[0],(unsigned int)pTemp[1],(unsigned int)pTemp[2],(unsigned int)pTemp[3],
                   (unsigned int)pTemp[4],(unsigned int)pTemp[5],(unsigned int)pTemp[6],(unsigned int)pTemp[7]
                 ));

                if(m_nEncodedStreamCount == 0) {

#ifdef WIN32
                    this->EncodedFrameQueue_AddFrameWithConfig_Mpeg4((MMP_U8*)mfc_outputInfo.StrmVirAddr, nEncBufSize, (mfc_outputInfo.frameType == MFC_FRAME_TYPE_I_FRAME)?MMP_ENCODED_FLAG_VIDEO_KEYFRAME:0);
#else
                    this->EncodedFrameQueue_AddFrame((MMP_U8*)mfc_outputInfo.StrmVirAddr, nEncBufSize, (mfc_outputInfo.frameType == MFC_FRAME_TYPE_I_FRAME)?MMP_ENCODED_FLAG_VIDEO_KEYFRAME:0);
#endif
                    if(this->EncodedFrameQueue_IsEmpty() != MMP_TRUE) {

                        this->EncodedFrameQueue_GetFrame(pEncBuffer, nEncBufMaxSize, &nEncBufSize, &nEncFlag);

                        pEncResult->uiEncodedStreamSize[MMP_ENCODED_BUF_STREAM] = nEncBufSize;
                        pEncResult->uiFlag |= nEncFlag;

                        m_nEncodedStreamCount++;
                    }
                }
                else {
                
                    memcpy(pEncBuffer, (void*)mfc_outputInfo.StrmVirAddr, nEncBufSize);
                    pEncResult->uiEncodedStreamSize[MMP_ENCODED_BUF_STREAM] = nEncBufSize;

                    if (mfc_outputInfo.frameType == MFC_FRAME_TYPE_I_FRAME) {
                        pEncResult->uiFlag |= MMP_ENCODED_FLAG_VIDEO_KEYFRAME;
                    }

                    m_nEncodedStreamCount++;
                }
            }
            
            mmpResult = MMP_SUCCESS; 
        }
    }

    enc_end_tick = CMmpUtil::GetTickCount();

    pEncResult->uiEncodedDuration = enc_end_tick - enc_start_tick;

    CMmpEncoderVideo::EncodeMonitor(pMediaSample, pEncResult);

    m_nInputFrameCount++;
    return mmpResult;
}
#endif


/*
 * @file    color_space_convertor.h
 * @brief   SEC_OMX specific define. It support MFC 5.x tiled.
 *   NV12T(tiled) layout:
 *   Each element is not pixel. It is 64x32 pixel block.
 *   uv pixel block is interleaved as u v u v u v ...
 *   y1    y2    y7    y8    y9    y10   y15   y16
 *   y3    y4    y5    y6    y11   y12   y13   y14
 *   y17   y18   y23   y24   y25   y26   y31   y32
 *   y19   y20   y21   y22   y27   y28   y29   y30
 *   uv1   uv2   uv7   uv8   uv9   uv10  uv15  uv16
 *   uv3   uv4   uv5   uv6   uv11  uv12  uv13  uv14
 *   YUV420Planar(linear) layout:
 *   Each element is not pixel. It is 64x32 pixel block.
 *   y1    y2    y3    y4    y5    y6    y7    y8
 *   y9    y10   y11   y12   y13   y14   y15   y16
 *   y17   y18   y19   y20   y21   y22   y23   y24
 *   y25   y26   y27   y28   y29   y30   y31   y32
 *   u1    u2    u3    u4    u5    u6    u7    u8
 *   v1    v2    v3    v4    v5    v6    v7    v8
 *   YUV420Semiplanar(linear) layout:
 *   Each element is not pixel. It is 64x32 pixel block.
 *   uv pixel block is interleaved as u v u v u v ...
 *   y1    y2    y3    y4    y5    y6    y7    y8
 *   y9    y10   y11   y12   y13   y14   y15   y16
 *   y17   y18   y19   y20   y21   y22   y23   y24
 *   y25   y26   y27   y28   y29   y30   y31   y32
 *   uv1   uv2   uv3   uv4   uv5   uv6   uv7   uv8
 *   uv9   uv10  uv11  uv12  uv13  uv14  uv15  uv16
 * @author  ShinWon Lee (shinwon.lee@samsung.com)
 * @version 1.0
 * @history
 *   2011.07.01 : Create
 *   2011.12.01 : Added csc functions
 */


MMP_RESULT CMmpEncoderVideo_Mfc::EncodeAu(class mmp_buffer_videoframe* p_buf_vf, class mmp_buffer_videostream* p_buf_vs) {

    SSBSIP_MFC_ERROR_CODE mfc_ret;
    SSBSIP_MFC_ENC_OUTPUT_INFO mfc_outputInfo;
    SSBSIP_MFC_ENC_INPUT_INFO mfc_inputinfo;
    SSBSIP_MFC_ENC_INPUT_INFO *p_mfc_inputinfo = &mfc_inputinfo;
    
    MMP_RESULT mmpResult = MMP_FAILURE;
    MMP_U32 i;
    MMP_U32 enc_start_tick, enc_end_tick;

    MMP_U8* pTemp;
    MMP_U8* pEncBuffer;
    MMP_U32 nEncBufSize, nEncBufMaxSize, nEncFlag;
    MMP_U32 buf_width, buf_height;

    enc_start_tick = CMmpUtil::GetTickCount();

    p_buf_vs->set_stream_size(0);
    p_buf_vs->set_stream_offset(0);
    p_buf_vs->set_flag(0);
    p_buf_vs->set_pts(p_buf_vf->get_pts());
    p_buf_vs->set_dsi_size(0);


    if(m_nEncodedStreamCount == 0) {

        mfc_ret = SsbSipMfcEncGetOutBuf(m_hMFCHandle, &mfc_outputInfo);
        if(mfc_ret != MFC_RET_OK)
        {
            MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpEncoderVideo_Mfc::EncodeAu] GetConfig Failed !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! ")));
            //return MMP_FAILURE;
        }
        else {
            
            p_buf_vs->alloc_dsi_buffer(mfc_outputInfo.headerSize);
            memcpy((void*)p_buf_vs->get_dsi_buffer(), (void*)mfc_outputInfo.StrmVirAddr, mfc_outputInfo.headerSize);

            pTemp = (MMP_U8*)mfc_outputInfo.StrmVirAddr;
            MMPDEBUGMSG(0, (TEXT("[CMmpEncoderVideo_Mfc::EncodeAu] ln=%d Sz=(%d %d) (%02x %02x %02x %02x %02x %02x %02x %02x )"), __LINE__, 
                    mfc_outputInfo.headerSize, mfc_outputInfo.dataSize,
                   (unsigned int)pTemp[0],(unsigned int)pTemp[1],(unsigned int)pTemp[2],(unsigned int)pTemp[3],
                   (unsigned int)pTemp[4],(unsigned int)pTemp[5],(unsigned int)pTemp[6],(unsigned int)pTemp[7]
                 ));
        }
    }

    memcpy(p_mfc_inputinfo, &this->m_mfc_input_info[0], sizeof(SSBSIP_MFC_ENC_INPUT_INFO) );
    
    buf_width = p_buf_vf->get_buf_stride(0);
    buf_height = p_buf_vf->get_buf_height(0);

    p_mfc_inputinfo->YSize = buf_width*buf_height;
    p_mfc_inputinfo->CSize = p_mfc_inputinfo->YSize/2;
    p_mfc_inputinfo->y_cookie = 0;
    p_mfc_inputinfo->c_cookie = 0;
    
    MMPDEBUGMSG(0, (TEXT("[CMmpEncoderVideo_Mfc::EncodeAu] MfcIn(Phy: 0x%08x 0x%08x Vir: 0x%08x 0x%08x Sz:%d %d ) "),
        p_mfc_inputinfo->YPhyAddr, p_mfc_inputinfo->CPhyAddr,
        p_mfc_inputinfo->YVirAddr, p_mfc_inputinfo->CVirAddr,
        p_mfc_inputinfo->YSize, p_mfc_inputinfo->CSize
        ));

    //if( (pMediaSample->uiBufferPhyAddr[MMP_DECODED_BUF_Y] == 0) 
     //   && (pMediaSample->uiBufferPhyAddr[MMP_DECODED_BUF_U] == 0) ) {
    
            
#if 1
    //MFC Input Format : NV12_TILE
          //MFC Input Format : Tiled 
            csc_linear_to_tiled_y((unsigned char*)p_mfc_inputinfo->YVirAddr, (unsigned char*)p_buf_vf->get_buf_vir_addr_y(), buf_width, buf_height);
            //memcpy(p_mfc_inputinfo->YVirAddr, (void*)p_buf_vf->get_buf_vir_addr_y(), p_mfc_inputinfo->YSize);
            //if(pMediaSample->pixelformat == MMP_PIXELFORMAT_YUV420_PLANAR) {
                csc_linear_to_tiled_uv((unsigned char*)p_mfc_inputinfo->CVirAddr,
                                       (unsigned char*)p_buf_vf->get_buf_vir_addr_u(),
                                       (unsigned char*)p_buf_vf->get_buf_vir_addr_v(),
                                       buf_width,  /* uv width */
                                       buf_height/2  /* uv height */
                                       );
#else
            //MFC Input Format : YUV420M 
            memcpy(p_mfc_inputinfo->YVirAddr, (void*)p_buf_vf->get_buf_vir_addr_y(), p_mfc_inputinfo->YSize);
            memcpy(p_mfc_inputinfo->CVirAddr, (void*)p_buf_vf->get_buf_vir_addr_u(), p_mfc_inputinfo->YSize/4);
            memcpy(p_mfc_inputinfo->CVirAddr + (p_mfc_inputinfo->YSize/4), (void*)p_buf_vf->get_buf_vir_addr_v(), p_mfc_inputinfo->YSize/4);

#endif

            //}
            //else {
              //  memcpy(p_mfc_inputinfo->CVirAddr, (void*)pMediaSample->uiBufferLogAddr[MMP_DECODED_BUF_U], p_mfc_inputinfo->CSize);
            //}

    //}
    //else {
    //    p_mfc_inputinfo->YPhyAddr = (void*)pMediaSample->uiBufferPhyAddr[MMP_DECODED_BUF_Y]; //pVideoEnc->MFCEncInputBuffer[pVideoEnc->indexInputBuffer].YPhyAddr;
    //    p_mfc_inputinfo->CPhyAddr = (void*)pMediaSample->uiBufferPhyAddr[MMP_DECODED_BUF_U]; //pVideoEnc->MFCEncInputBuffer[pVideoEnc->indexInputBuffer].CPhyAddr;
    //    p_mfc_inputinfo->YVirAddr = (void*)pMediaSample->uiBufferLogAddr[MMP_DECODED_BUF_Y]; //pVideoEnc->MFCEncInputBuffer[pVideoEnc->indexInputBuffer].YVirAddr;
    //    p_mfc_inputinfo->CVirAddr = (void*)pMediaSample->uiBufferLogAddr[MMP_DECODED_BUF_U]; //pVideoEnc->MFCEncInputBuffer[pVideoEnc->indexInputBuffer].CVirAddr;

    //    p_mfc_inputinfo->YSize = buf_width*buf_height;
    //    p_mfc_inputinfo->CSize = p_mfc_inputinfo->YSize/2;
        
    //    p_mfc_inputinfo->y_cookie = 0;
    //    p_mfc_inputinfo->c_cookie = 0;
   // }
        
    mfc_ret = SsbSipMfcEncSetInBuf(m_hMFCHandle, p_mfc_inputinfo);
    if(mfc_ret == MFC_RET_OK) {

        mfc_ret = SsbSipMfcEncExe(m_hMFCHandle);
        if(mfc_ret == MFC_RET_OK) /* Success */ {
            
            mfc_ret = SsbSipMfcEncGetOutBuf(m_hMFCHandle, &mfc_outputInfo);
            if(mfc_ret == MFC_RET_OK) /* Success */ {
            
                pEncBuffer = (MMP_U8*)p_buf_vs->get_buf_vir_addr();
                nEncBufMaxSize = p_buf_vs->get_buf_size();
                nEncBufSize = mfc_outputInfo.dataSize;

                pTemp = (MMP_U8*)mfc_outputInfo.StrmVirAddr;
                MMPDEBUGMSG(0, (TEXT("[CMmpEncoderVideo_Mfc::EncodeAu] ln=%d Sz=%d Type=%d (%02x %02x %02x %02x %02x %02x %02x %02x )"), __LINE__, 
                    mfc_outputInfo.dataSize,
                    mfc_outputInfo.frameType,
                   (unsigned int)pTemp[0],(unsigned int)pTemp[1],(unsigned int)pTemp[2],(unsigned int)pTemp[3],
                   (unsigned int)pTemp[4],(unsigned int)pTemp[5],(unsigned int)pTemp[6],(unsigned int)pTemp[7]
                 ));

                
                memcpy(pEncBuffer, (void*)mfc_outputInfo.StrmVirAddr, nEncBufSize);
                p_buf_vs->set_stream_size(nEncBufSize);
                p_buf_vs->set_stream_offset(0);
                                
                if (mfc_outputInfo.frameType == MFC_FRAME_TYPE_I_FRAME) {
                    p_buf_vs->or_flag(mmp_buffer_media::FLAG_VIDEO_KEYFRAME);
                }
                m_nEncodedStreamCount++;
                
            }
            
            mmpResult = MMP_SUCCESS; 
        }
    }

    enc_end_tick = CMmpUtil::GetTickCount();

    //pEncResult->uiEncodedDuration = enc_end_tick - enc_start_tick;

    //CMmpEncoderVideo::EncodeMonitor(pMediaSample, pEncResult);

    m_nInputFrameCount++;
    return mmpResult;
}

#if (MMP_OS == MMP_OS_WIN32) 

extern "C" void csc_linear_to_tiled_uv(
    unsigned char *uv_dst,
    unsigned char *u_src,
    unsigned char *v_src,
    unsigned int uv_width, 
    unsigned int uv_height) {

    unsigned char* u_dst, *v_dst;
    
    int chroma_size = (uv_width/2)*uv_height;
    
    u_dst = uv_dst;
    v_dst = u_dst + chroma_size;

    memcpy((void*)u_dst, (void*)u_src, chroma_size);
    memcpy((void*)v_dst, (void*)v_src, chroma_size);
}

#endif

