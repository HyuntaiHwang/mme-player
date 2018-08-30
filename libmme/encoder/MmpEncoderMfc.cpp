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

#include "MmpEncoderMfc.hpp"
#include "MmpUtil.hpp"

/////////////////////////////////////////////////////////////
//CMmpEncoderMfc Member Functions

CMmpEncoderMfc::CMmpEncoderMfc(struct MmpEncoderCreateConfig *pCreateConfig) :
m_CreateConfig(*pCreateConfig)
,m_hMFCHandle(NULL)
{
    switch(pCreateConfig->nFormat) {
    
        /* Video */
        case MMP_FOURCC_VIDEO_H263: m_MFCCodecType=H263_ENC; break;
        case MMP_FOURCC_VIDEO_H264: m_MFCCodecType=H264_ENC; break;
        case MMP_FOURCC_VIDEO_MPEG4: m_MFCCodecType=MPEG4_ENC; break;
        default:  m_MFCCodecType = UNKNOWN_TYPE;
    }

    m_MFCImgResol.width = 0;
    m_MFCImgResol.height = 0;
    m_MFCImgResol.buf_width = 0;
    m_MFCImgResol.buf_height = 0;
}

CMmpEncoderMfc::~CMmpEncoderMfc()
{

}

MMP_RESULT CMmpEncoderMfc::Open()
{
    MMP_U32 i;
    MMP_RESULT mmpResult = MMP_SUCCESS;
    SSBSIP_MFC_ERROR_CODE mfc_ret;


    if(mmpResult == MMP_SUCCESS) {
        /* MFC(Multi Format Codec) decoder and CMM(Codec Memory Management) driver open */
        m_hMFCHandle = SsbSipMfcEncOpen();
        if(m_hMFCHandle == NULL) {
            mmpResult = MMP_FAILURE;
            MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpEncoderMfc::Open] FAIL: SsbSipMfcDecOpen \n\r")));
        }
    }

    MMPDEBUGMSG(0, (TEXT("[CMmpEncoderMfc::Open] W:%d H:%d nIDRPeriod:%d BitRate:%d Fps:%d \n\r"),
                                this->m_CreateConfig.nPicWidth,
                                this->m_CreateConfig.nPicHeight,
                                this->m_CreateConfig.nIDRPeriod,
                                this->m_CreateConfig.nBitRate,
                                this->m_CreateConfig.nFrameRate
                                ));

    if(mmpResult == MMP_SUCCESS) {
    
        if(m_MFCCodecType == MPEG4_ENC) {
            this->Set_Mpeg4Enc_Param(&m_Mpeg4param); //Set_Mpeg4Enc_Param(&(pMpeg4Enc->hMFCMpeg4Handle.mpeg4MFCParam), pSECComponent);
            mfc_ret = SsbSipMfcEncInit(m_hMFCHandle, &m_Mpeg4param);
        }
        else if(m_MFCCodecType == H263_ENC) {
            this->Set_H263Enc_Param(&m_H263Param); //Set_Mpeg4Enc_Param(&(pMpeg4Enc->hMFCMpeg4Handle.mpeg4MFCParam), pSECComponent);
            mfc_ret = SsbSipMfcEncInit(m_hMFCHandle, &m_H263Param);
        }
        else {
            this->Set_H264Enc_Param(&m_H264param); 
            mfc_ret = SsbSipMfcEncInit(m_hMFCHandle, &m_H264param);
        }
        
        if(mfc_ret != MFC_RET_OK) {
            mmpResult = MMP_FAILURE;
            MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpEncoderMfc::Open] FAIL: SsbSipMfcEncInit \n\r")));
        }
    }

#if 1
    /* Allocate encoder's input buffer */
    if(mmpResult == MMP_SUCCESS) {
    
        for(i = 0; i < MMP_ENCODER_MFC_INPUT_BUF_COUNT; i++) {

            mfc_ret = SsbSipMfcEncGetInBuf(m_hMFCHandle, &m_mfc_input_info[i]);
            if(mfc_ret != MFC_RET_OK) {
                mmpResult = MMP_FAILURE;
                MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpEncoderMfc::Open] FAIL: SsbSipMfcEncGetInBuf (%d) \n\r"), i));
                break;
            }
            else {
                //pVideoEnc->MFCEncInputBuffer[0].YPhyAddr = pMpeg4Enc->hMFCMpeg4Handle.inputInfo.YPhyAddr;
                //pVideoEnc->MFCEncInputBuffer[0].CPhyAddr = pMpeg4Enc->hMFCMpeg4Handle.inputInfo.CPhyAddr;
                //pVideoEnc->MFCEncInputBuffer[0].YVirAddr = pMpeg4Enc->hMFCMpeg4Handle.inputInfo.YVirAddr;
                //pVideoEnc->MFCEncInputBuffer[0].CVirAddr = pMpeg4Enc->hMFCMpeg4Handle.inputInfo.CVirAddr;
                ////pVideoEnc->MFCEncInputBuffer[0].YBufferSize = pMpeg4Enc->hMFCMpeg4Handle.inputInfo.YSize;
                //pVideoEnc->MFCEncInputBuffer[0].CBufferSize = pMpeg4Enc->hMFCMpeg4Handle.inputInfo.CSize;
                //pVideoEnc->MFCEncInputBuffer[0].YDataSize = 0;
                //pVideoEnc->MFCEncInputBuffer[0].CDataSize = 0;
                //SEC_OSAL_Log(SEC_LOG_TRACE, "pMpeg4Enc->hMFCMpeg4Handle.inputInfo.YVirAddr : 0x%x", pMpeg4Enc->hMFCMpeg4Handle.inputInfo.YVirAddr);
                //SEC_OSAL_Log(SEC_LOG_TRACE, "pMpeg4Enc->hMFCMpeg4Handle.inputInfo.CVirAddr : 0x%x", pMpeg4Enc->hMFCMpeg4Handle.inputInfo.CVirAddr);
            }
        }
    }
#endif

    return mmpResult;
}


MMP_RESULT CMmpEncoderMfc::Close()
{
    
    if(m_hMFCHandle != NULL) {
        SsbSipMfcEncClose(m_hMFCHandle);
        m_hMFCHandle = NULL;
    }

    return MMP_SUCCESS;
}


void CMmpEncoderMfc::Set_Mpeg4Enc_Param(SSBSIP_MFC_ENC_MPEG4_PARAM *pMpeg4Param)//, SEC_OMX_BASECOMPONENT *pSECComponent)
{
#if 0 //libOMX.SEC.M4V.Encoder Default Data

    I/        ( 2619): SourceWidth             : 640
I/        ( 2619): SourceHeight            : 480
I/        ( 2619): IDRPeriod               : 15
I/        ( 2619): SliceMode               : 0
I/        ( 2619): RandomIntraMBRefresh    : 0
I/        ( 2619): EnableFRMRateControl    : 0
I/        ( 2619): Bitrate                 : 192000

I/        ( 2619): FrameQp                 : 20
I/        ( 2619): FrameQp_P               : 20

I/        ( 2619): QSCodeMax               : 30
I/        ( 2619): QSCodeMin               : 10
I/        ( 2619): CBRPeriodRf             : 100

I/        ( 2619): PadControlOn            : 0
I/        ( 2619): LumaPadVal              : 0
I/        ( 2619): CbPadVal                : 0
I/        ( 2619): CrPadVal                : 0

I/        ( 2619): FrameMap                : 0

I/        ( 2619): ProfileIDC              : 0
I/        ( 2619): LevelIDC                : 4

I/        ( 2619): FrameQp_B               : 20
I/        ( 2619): TimeIncreamentRes       : 15
I/        ( 2619): VopTimeIncreament       : 1
I/        ( 2619): SliceArgument           : 0
I/        ( 2619): NumberBFrames           : 0
I/        ( 2619): DisableQpelME           : 1

#endif

    //SEC_OMX_BASEPORT    *pSECInputPort = NULL;
    //SEC_OMX_BASEPORT    *pSECOutputPort = NULL;
    //SEC_OMX_VIDEOENC_COMPONENT *pVideoEnc = NULL;
    //SEC_MPEG4ENC_HANDLE *pMpeg4Enc = NULL;

    //pVideoEnc = (SEC_OMX_VIDEOENC_COMPONENT *)pSECComponent->hComponentHandle;
    //pMpeg4Enc = (SEC_MPEG4ENC_HANDLE *)((SEC_OMX_VIDEOENC_COMPONENT *)pSECComponent->hComponentHandle)->hCodecHandle;
    //pSECInputPort = &pSECComponent->pSECPort[INPUT_PORT_INDEX];
    //pSECOutputPort = &pSECComponent->pSECPort[OUTPUT_PORT_INDEX];

    pMpeg4Param->codecType            = MPEG4_ENC;
    pMpeg4Param->SourceWidth          = this->m_CreateConfig.nPicWidth; //pSECOutputPort->portDefinition.format.video.nFrameWidth;
    pMpeg4Param->SourceHeight         = this->m_CreateConfig.nPicHeight; //pSECOutputPort->portDefinition.format.video.nFrameHeight;
    pMpeg4Param->IDRPeriod            = this->m_CreateConfig.nIDRPeriod;//pMpeg4Enc->mpeg4Component[OUTPUT_PORT_INDEX].nPFrames + 1;
    pMpeg4Param->SliceMode            = 0;
//#ifdef USE_SLICE_OUTPUT_MODE
//    pMpeg4Param->OutputMode           = FRAME;
//#endif
    pMpeg4Param->RandomIntraMBRefresh = 0;
    pMpeg4Param->Bitrate              = this->m_CreateConfig.nBitRate; //pSECOutputPort->portDefinition.format.video.nBitrate;
    pMpeg4Param->QSCodeMax            = 30;
    pMpeg4Param->QSCodeMin            = 10;
    pMpeg4Param->PadControlOn         = 0;    /* 0: Use boundary pixel, 1: Use the below setting value */
    pMpeg4Param->LumaPadVal           = 0;
    pMpeg4Param->CbPadVal             = 0;
    pMpeg4Param->CrPadVal             = 0;

#if 1
    switch(this->m_CreateConfig.enc_info.mpeg4.eProfile) {
        case MMP_VIDEO_MPEG4ProfileSimple           ://= 0x01,        
        case MMP_VIDEO_MPEG4ProfileSimpleScalable   ://= 0x02,    
        case MMP_VIDEO_MPEG4ProfileCore             ://= 0x04,              
        case MMP_VIDEO_MPEG4ProfileMain             ://= 0x08,             
        case MMP_VIDEO_MPEG4ProfileNbit             ://= 0x10,              
        case MMP_VIDEO_MPEG4ProfileScalableTexture  ://= 0x20,   
        case MMP_VIDEO_MPEG4ProfileSimpleFace       ://= 0x40,        
        case MMP_VIDEO_MPEG4ProfileSimpleFBA        ://= 0x80,         
        case MMP_VIDEO_MPEG4ProfileBasicAnimated    ://= 0x100,     
        case MMP_VIDEO_MPEG4ProfileHybrid           ://= 0x200,            
        case MMP_VIDEO_MPEG4ProfileAdvancedRealTime ://= 0x400,  
        case MMP_VIDEO_MPEG4ProfileCoreScalable     ://= 0x800,     
            pMpeg4Param->ProfileIDC           = 0; //Simple:0  Advanced Profile:1
            break;

        case MMP_VIDEO_MPEG4ProfileAdvancedCoding   ://= 0x1000,    
        case MMP_VIDEO_MPEG4ProfileAdvancedCore     ://= 0x2000,      
        case MMP_VIDEO_MPEG4ProfileAdvancedScalable ://= 0x4000,
        case MMP_VIDEO_MPEG4ProfileAdvancedSimple   ://= 0x8000,
            pMpeg4Param->ProfileIDC           = 1; //Simple:0  Advanced Profile:1
            break;
                   
        default:
            pMpeg4Param->ProfileIDC           = 0; //Simple:0  Advanced Profile:1 //SOMXMpeg4ProfileToMFCProfile(pMpeg4Enc->mpeg4Component[OUTPUT_PORT_INDEX].eProfile);
    }
    
    switch(this->m_CreateConfig.enc_info.mpeg4.eLevel) {
        case MMP_VIDEO_MPEG4Level0  : //= 0x01,   /**< Level 0 */   
        case MMP_VIDEO_MPEG4Level0b : //= 0x02,   /**< Level 0b */   
        case MMP_VIDEO_MPEG4Level1  : //= 0x04,   /**< Level 1 */ 
        case MMP_VIDEO_MPEG4Level2  : //= 0x08,   /**< Level 2 */ 
        case MMP_VIDEO_MPEG4Level3  : //= 0x10,   /**< Level 3 */ 
        case MMP_VIDEO_MPEG4Level4  : //= 0x20,   /**< Level 4 */  
        case MMP_VIDEO_MPEG4Level4a : //= 0x40,   /**< Level 4a */  
        case MMP_VIDEO_MPEG4Level5  : //= 0x80,   /**< Level 5 */  
        default:
            pMpeg4Param->LevelIDC             = 0; //OMXMpeg4LevelToMFCLevel(pMpeg4Enc->mpeg4Component[OUTPUT_PORT_INDEX].eLevel);
            break;
    }
#else

    pMpeg4Param->ProfileIDC           = 0; //Simple:0  Advanced Profile:1 //SOMXMpeg4ProfileToMFCProfile(pMpeg4Enc->mpeg4Component[OUTPUT_PORT_INDEX].eProfile);
    pMpeg4Param->LevelIDC             = 0; //OMXMpeg4LevelToMFCLevel(pMpeg4Enc->mpeg4Component[OUTPUT_PORT_INDEX].eLevel);
#endif

    pMpeg4Param->TimeIncreamentRes    = this->m_CreateConfig.nFrameRate; //(pSECInputPort->portDefinition.format.video.xFramerate) >> 16;
    pMpeg4Param->VopTimeIncreament    = 1;
    pMpeg4Param->SliceArgument        = 0;    /* MB number or byte number */
    pMpeg4Param->NumberBFrames        = 0;    /* 0(not used) ~ 2 */
    pMpeg4Param->DisableQpelME        = 1;

    pMpeg4Param->FrameQp              = 20;//pVideoEnc->quantization.nQpI;
    pMpeg4Param->FrameQp_P            = 20;//pVideoEnc->quantization.nQpP;
    pMpeg4Param->FrameQp_B            = 20;//pVideoEnc->quantization.nQpB;

    //SEC_OSAL_Log(SEC_LOG_TRACE, "pVideoEnc->eControlRate[OUTPUT_PORT_INDEX]: 0x%x", pVideoEnc->eControlRate[OUTPUT_PORT_INDEX]);
    //switch (pVideoEnc->eControlRate[OUTPUT_PORT_INDEX]) {
    //case OMX_Video_ControlRateVariable:
    //    SEC_OSAL_Log(SEC_LOG_TRACE, "Video Encode VBR");
    //    pMpeg4Param->EnableFRMRateControl = 0;        // 0: Disable, 1: Frame level RC
    //    pMpeg4Param->CBRPeriodRf          = 100;
    //    break;
    //case OMX_Video_ControlRateConstant:
    //    SEC_OSAL_Log(SEC_LOG_TRACE, "Video Encode CBR");
        pMpeg4Param->EnableFRMRateControl = 1;        // 0: Disable, 1: Frame level RC
        pMpeg4Param->CBRPeriodRf          = 10;
    //    break;
    //case OMX_Video_ControlRateDisable:
    //default: //Android default
    //    SEC_OSAL_Log(SEC_LOG_TRACE, "Video Encode VBR");
    //    pMpeg4Param->EnableFRMRateControl = 0;
    //    pMpeg4Param->CBRPeriodRf          = 100;
    //    break;
    //}

    //switch ((SEC_OMX_COLOR_FORMATTYPE)pSECInputPort->portDefinition.format.video.eColorFormat) {
    //case OMX_SEC_COLOR_FormatNV12LPhysicalAddress:
    //case OMX_SEC_COLOR_FormatNV12LVirtualAddress:
    //case OMX_COLOR_FormatYUV420SemiPlanar:
    //case OMX_COLOR_FormatYUV420Planar:
//#ifdef METADATABUFFERTYPE
  //  case OMX_COLOR_FormatAndroidOpaque:
//#endif
       // pMpeg4Param->FrameMap = NV12_LINEAR;
  //      break;
  //  case OMX_SEC_COLOR_FormatNV12TPhysicalAddress:
  //  case OMX_SEC_COLOR_FormatNV12Tiled:
  //      pMpeg4Param->FrameMap = NV12_TILE;
  //      break;
  //  case OMX_SEC_COLOR_FormatNV21LPhysicalAddress:
  //  case OMX_SEC_COLOR_FormatNV21Linear:
  //      pMpeg4Param->FrameMap = NV21_LINEAR;
  //      break;
  //  default:
        pMpeg4Param->FrameMap = NV12_TILE;
  //      break;
  //  }

   // Mpeg4PrintParams(pMpeg4Param);
}

void CMmpEncoderMfc::Set_H264Enc_Param(SSBSIP_MFC_ENC_H264_PARAM *pH264Arg)
{
    //SEC_OMX_BASEPORT          *pSECInputPort = NULL;
    //SEC_OMX_BASEPORT          *pSECOutputPort = NULL;
    //SEC_OMX_VIDEOENC_COMPONENT *pVideoEnc = NULL;
    //SEC_H264ENC_HANDLE        *pH264Enc = NULL;

    //pVideoEnc = (SEC_OMX_VIDEOENC_COMPONENT *)pSECComponent->hComponentHandle;
    //pH264Enc = (SEC_H264ENC_HANDLE *)((SEC_OMX_VIDEOENC_COMPONENT *)pSECComponent->hComponentHandle)->hCodecHandle;
    //pSECInputPort = &pSECComponent->pSECPort[INPUT_PORT_INDEX];
    //pSECOutputPort = &pSECComponent->pSECPort[OUTPUT_PORT_INDEX];

    pH264Arg->codecType    = H264_ENC;
    pH264Arg->SourceWidth  = this->m_CreateConfig.nPicWidth; //pSECOutputPort->portDefinition.format.video.nFrameWidth;
    pH264Arg->SourceHeight = this->m_CreateConfig.nPicHeight; //pSECOutputPort->portDefinition.format.video.nFrameHeight;
    pH264Arg->IDRPeriod    = this->m_CreateConfig.nIDRPeriod; //pH264Enc->AVCComponent[OUTPUT_PORT_INDEX].nPFrames + 1;
    pH264Arg->SliceMode    = 0;
#ifdef USE_SLICE_OUTPUT_MODE
    pH264Arg->OutputMode   = FRAME;
#endif
    pH264Arg->RandomIntraMBRefresh = 0;
    pH264Arg->Bitrate      = this->m_CreateConfig.nBitRate; //pSECOutputPort->portDefinition.format.video.nBitrate;
    pH264Arg->QSCodeMax    = 51;
    pH264Arg->QSCodeMin    = 10;
    pH264Arg->PadControlOn = 0;             // 0: disable, 1: enable
    pH264Arg->LumaPadVal   = 0;
    pH264Arg->CbPadVal     = 0;
    pH264Arg->CrPadVal     = 0;

#if 1
    switch(this->m_CreateConfig.enc_info.h264.eProfile) {
    
        case MMP_VIDEO_AVCProfileBaseline : //= 0x01,   /**< Baseline profile */
            pH264Arg->ProfileIDC   = 2;
            break;
        case MMP_VIDEO_AVCProfileMain     : //= 0x02,   /**< Main profile */
            pH264Arg->ProfileIDC   = 0;
            break;
        case MMP_VIDEO_AVCProfileExtended : //= 0x04,   /**< Extended profile */
        case MMP_VIDEO_AVCProfileHigh     : //= 0x08,   /**< High profile */
        case MMP_VIDEO_AVCProfileHigh10   : //= 0x10,   /**< High 10 profile */
        case MMP_VIDEO_AVCProfileHigh422  : //= 0x20,   /**< High 4:2:2 profile */
        case MMP_VIDEO_AVCProfileHigh444  : //= 0x40,   /**< High 4:4:4 profile */
            pH264Arg->ProfileIDC   = 1;
            break;
        default:
            pH264Arg->ProfileIDC   = 0; //0:Main 1:High 2:BaseLine OMXAVCProfileToProfileIDC(pH264Enc->AVCComponent[OUTPUT_PORT_INDEX].eProfile); //0;  //(OMX_VIDEO_AVCProfileMain)
            break;
    }

    switch(this->m_CreateConfig.enc_info.h264.eLevel) {
    
        case MMP_VIDEO_AVCLevel1  : // = 0x01,     /**< Level 1 */
             pH264Arg->LevelIDC  = 10;
             break;
        case MMP_VIDEO_AVCLevel1b : // = 0x02,     /**< Level 1b */
             pH264Arg->LevelIDC  = 9;
             break;
        case MMP_VIDEO_AVCLevel11 : // = 0x04,     /**< Level 1.1 */
             pH264Arg->LevelIDC  = 11;
             break;
        case MMP_VIDEO_AVCLevel12 : // = 0x08,     /**< Level 1.2 */
             pH264Arg->LevelIDC  = 12;
             break;
        case MMP_VIDEO_AVCLevel13 : // = 0x10,     /**< Level 1.3 */
             pH264Arg->LevelIDC  = 13;
             break;
        case MMP_VIDEO_AVCLevel2  : // = 0x20,     /**< Level 2 */
             pH264Arg->LevelIDC  = 20;
             break;
        case MMP_VIDEO_AVCLevel21 : // = 0x40,     /**< Level 2.1 */
             pH264Arg->LevelIDC  = 21;
             break;
        case MMP_VIDEO_AVCLevel22 : // = 0x80,     /**< Level 2.2 */
             pH264Arg->LevelIDC  = 22;
             break;
        case MMP_VIDEO_AVCLevel3  : // = 0x100,    /**< Level 3 */
             pH264Arg->LevelIDC  = 30;
             break;
        case MMP_VIDEO_AVCLevel31 : // = 0x200,    /**< Level 3.1 */
             pH264Arg->LevelIDC  = 31;
             break;
        case MMP_VIDEO_AVCLevel32 : // = 0x400,    /**< Level 3.2 */
             pH264Arg->LevelIDC  = 32;
             break;
        case MMP_VIDEO_AVCLevel4  : // = 0x800,    /**< Level 4 */
             pH264Arg->LevelIDC  = 40;
             break;
        case MMP_VIDEO_AVCLevel41 : // = 0x1000,   /**< Level 4.1 */
             pH264Arg->LevelIDC  = 41;
             break;
        case MMP_VIDEO_AVCLevel42 : // = 0x2000,   /**< Level 4.2 */
             pH264Arg->LevelIDC  = 42;
             break;
        case MMP_VIDEO_AVCLevel5  : // = 0x4000,   /**< Level 5 */
             pH264Arg->LevelIDC  = 50;
             break;
        case MMP_VIDEO_AVCLevel51 : // = 0x8000,   /**< Level 5.1 */
             pH264Arg->LevelIDC  = 51;
             break;
    
        default:
            pH264Arg->LevelIDC     = 40; //10(1.0)  9(1.0b)  11(1.1)  12(1.2)  13(1.3)  20(2.0) 21(2.1) 22(2.2) 30(3.0) 31(3.1) 32(3.2) 40(4.0)    MXAVCLevelToLevelIDC(pH264Enc->AVCComponent[OUTPUT_PORT_INDEX].eLevel);       //40; //(OMX_VIDEO_AVCLevel4)
            break;
    }
#else

    pH264Arg->ProfileIDC   = 0; //0:Main 
    pH264Arg->LevelIDC     = 40;

#endif

    pH264Arg->FrameRate    = this->m_CreateConfig.nFrameRate;//(pSECInputPort->portDefinition.format.video.xFramerate) >> 16;
    pH264Arg->SliceArgument = 0;          // Slice mb/byte size number
    pH264Arg->NumberBFrames = 0;            // 0 ~ 2
    pH264Arg->NumberReferenceFrames = 1;
    pH264Arg->NumberRefForPframes   = 1;
    pH264Arg->LoopFilterDisable     = 1;    // 1: Loop Filter Disable, 0: Filter Enable
    pH264Arg->LoopFilterAlphaC0Offset = 0;
    pH264Arg->LoopFilterBetaOffset    = 0;
    pH264Arg->SymbolMode       = 0;         // 0: CAVLC, 1: CABAC
    pH264Arg->PictureInterlace = 0;
    pH264Arg->Transform8x8Mode = 0;         // 0: 4x4, 1: allow 8x8
    pH264Arg->DarkDisable     = 1;
    pH264Arg->SmoothDisable   = 1;
    pH264Arg->StaticDisable   = 1;
    pH264Arg->ActivityDisable = 1;

    pH264Arg->FrameQp      = 20;//pVideoEnc->quantization.nQpI;
    pH264Arg->FrameQp_P    = 20;//pVideoEnc->quantization.nQpP;
    pH264Arg->FrameQp_B    = 20;//pVideoEnc->quantization.nQpB;

    //SEC_OSAL_Log(SEC_LOG_TRACE, "pVideoEnc->eControlRate[OUTPUT_PORT_INDEX]: 0x%x", pVideoEnc->eControlRate[OUTPUT_PORT_INDEX]);
    //switch (pVideoEnc->eControlRate[OUTPUT_PORT_INDEX]) {
    //case OMX_Video_ControlRateVariable:
    //    SEC_OSAL_Log(SEC_LOG_TRACE, "Video Encode VBR");
    //    pH264Arg->EnableFRMRateControl = 0;        // 0: Disable, 1: Frame level RC
    //    pH264Arg->EnableMBRateControl  = 0;        // 0: Disable, 1:MB level RC
     //   pH264Arg->CBRPeriodRf  = 100;
     //   break;
    //case OMX_Video_ControlRateConstant:
    //    SEC_OSAL_Log(SEC_LOG_TRACE, "Video Encode CBR");
        pH264Arg->EnableFRMRateControl = 1;        // 0: Disable, 1: Frame level RC
        pH264Arg->EnableMBRateControl  = 1;         // 0: Disable, 1:MB level RC
        pH264Arg->CBRPeriodRf  = 10;
    //    break;
    //case OMX_Video_ControlRateDisable:
    //default: //Android default
    //    SEC_OSAL_Log(SEC_LOG_TRACE, "Video Encode VBR");
        pH264Arg->EnableFRMRateControl = 0;
        pH264Arg->EnableMBRateControl  = 0;
        pH264Arg->CBRPeriodRf  = 100;
    //    break;
    //}

    //switch ((SEC_OMX_COLOR_FORMATTYPE)pSECInputPort->portDefinition.format.video.eColorFormat) {
    //case OMX_SEC_COLOR_FormatNV12LPhysicalAddress:
    //case OMX_SEC_COLOR_FormatNV12LVirtualAddress:
    //case OMX_COLOR_FormatYUV420SemiPlanar:
    //case OMX_COLOR_FormatYUV420Planar:
//#ifdef USE_METADATABUFFERTYPE
  //  case OMX_COLOR_FormatAndroidOpaque:
//#endif
        //pH264Arg->FrameMap = NV12_LINEAR;
  //      break;
    //case OMX_SEC_COLOR_FormatNV12TPhysicalAddress:
    //case OMX_SEC_COLOR_FormatNV12Tiled:
        pH264Arg->FrameMap = NV12_TILE;
  //      break;
  //  case OMX_SEC_COLOR_FormatNV21LPhysicalAddress:
  //  case OMX_SEC_COLOR_FormatNV21Linear:
  //      pH264Arg->FrameMap = NV21_LINEAR;
  //      break;
  //  default:
  //      pH264Arg->FrameMap = NV12_TILE;
  //      break;
  //  }

 //   H264PrintParams(pH264Arg);
}


void CMmpEncoderMfc::Set_H263Enc_Param(SSBSIP_MFC_ENC_H263_PARAM *pH263Param)
{
    //SEC_OMX_BASEPORT    *pSECInputPort = NULL;
    //SEC_OMX_BASEPORT    *pSECOutputPort = NULL;
    //SEC_OMX_VIDEOENC_COMPONENT *pVideoEnc = NULL;
    //SEC_MPEG4ENC_HANDLE *pMpeg4Enc = NULL;

    //pVideoEnc = (SEC_OMX_VIDEOENC_COMPONENT *)pSECComponent->hComponentHandle;
    //pMpeg4Enc = (SEC_MPEG4ENC_HANDLE *)((SEC_OMX_VIDEOENC_COMPONENT *)pSECComponent->hComponentHandle)->hCodecHandle;
    //pSECInputPort = &pSECComponent->pSECPort[INPUT_PORT_INDEX];
    //pSECOutputPort = &pSECComponent->pSECPort[OUTPUT_PORT_INDEX];

    pH263Param->codecType            = H263_ENC;
    pH263Param->SourceWidth          = this->m_CreateConfig.nPicWidth; //pSECOutputPort->portDefinition.format.video.nFrameWidth;
    pH263Param->SourceHeight         = this->m_CreateConfig.nPicHeight; //pSECOutputPort->portDefinition.format.video.nFrameHeight;
    pH263Param->IDRPeriod            = this->m_CreateConfig.nIDRPeriod; //pMpeg4Enc->h263Component[OUTPUT_PORT_INDEX].nPFrames + 1;
    pH263Param->SliceMode            = 0;
    pH263Param->RandomIntraMBRefresh = 0;
    pH263Param->Bitrate              = this->m_CreateConfig.nBitRate; //pSECOutputPort->portDefinition.format.video.nBitrate;
    pH263Param->QSCodeMax            = 30;
    pH263Param->QSCodeMin            = 10;
    pH263Param->PadControlOn         = 0;    /* 0: Use boundary pixel, 1: Use the below setting value */
    pH263Param->LumaPadVal           = 0;
    pH263Param->CbPadVal             = 0;
    pH263Param->CrPadVal             = 0;

    pH263Param->FrameRate            = this->m_CreateConfig.nFrameRate; //(pSECInputPort->portDefinition.format.video.xFramerate) >> 16;

    pH263Param->FrameQp              = 20;//pVideoEnc->quantization.nQpI;
    pH263Param->FrameQp_P            = 20;//pVideoEnc->quantization.nQpP;

    //SEC_OSAL_Log(SEC_LOG_TRACE, "pVideoEnc->eControlRate[OUTPUT_PORT_INDEX]: 0x%x", pVideoEnc->eControlRate[OUTPUT_PORT_INDEX]);
    //switch (pVideoEnc->eControlRate[OUTPUT_PORT_INDEX]) {
    //case OMX_Video_ControlRateVariable:
    //    SEC_OSAL_Log(SEC_LOG_TRACE, "Video Encode VBR");
    //    pH263Param->EnableFRMRateControl = 0;        // 0: Disable, 1: Frame level RC
    //    pH263Param->CBRPeriodRf          = 100;
    //    break;
    //case OMX_Video_ControlRateConstant:
    //    SEC_OSAL_Log(SEC_LOG_TRACE, "Video Encode CBR");
        pH263Param->EnableFRMRateControl = 1;        // 0: Disable, 1: Frame level RC
        pH263Param->CBRPeriodRf          = 10;
    //    break;
    //case OMX_Video_ControlRateDisable:
    //default: //Android default
    //    SEC_OSAL_Log(SEC_LOG_TRACE, "Video Encode VBR");
    //    pH263Param->EnableFRMRateControl = 0;
    //    pH263Param->CBRPeriodRf          = 100;
    //    break;
   // }

    //switch ((SEC_OMX_COLOR_FORMATTYPE)pSECInputPort->portDefinition.format.video.eColorFormat) {
    //case OMX_SEC_COLOR_FormatNV12LPhysicalAddress:
    //case OMX_SEC_COLOR_FormatNV12LVirtualAddress:
    //case OMX_COLOR_FormatYUV420SemiPlanar:
        //pH263Param->FrameMap = NV12_LINEAR;
    //    break;
    //case OMX_SEC_COLOR_FormatNV12TPhysicalAddress:
    //case OMX_SEC_COLOR_FormatNV12Tiled:
    //    pH263Param->FrameMap = NV12_TILE;
    //    break;
    //case OMX_SEC_COLOR_FormatNV21LPhysicalAddress:
    //case OMX_SEC_COLOR_FormatNV21Linear:
    //    pH263Param->FrameMap = NV21_LINEAR;
    //    break;
    //default:
        pH263Param->FrameMap = NV12_TILE;
    //    break;
    //}

    //H263PrintParams(pH263Param);
}


MMP_RESULT CMmpEncoderMfc::EncodeDSI(CMmpMediaSampleEncode* pMediaSample, CMmpMediaSampleEncodeResult* pEncResult) {

   return MMP_SUCCESS;
}

