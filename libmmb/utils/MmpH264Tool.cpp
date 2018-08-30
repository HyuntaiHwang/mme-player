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
#include "MmpH264Tool.hpp"
#include "MmpUtil.hpp"
//#include "D:\\WINCE600\\PLATFORM\\ZEUS1\\Src\\Common\\MmpLib\\MmpGlobal\\MmpError.h"


/////////////////////////////////////////////////////////////
//Paramter Max Size

#define MAX_SEQ_PARAMETERS  32
#define MAX_PIC_PARAMETERS  256

///////////////////////////////////////////////////////////////
// PPS, SPS status
#define H264_INVALID_PPS 0xAAAAAAAA
#define H264_INVALID_SPS 0xBBBBBBBB

#define H264_VALID_PPS   0xAAAAAA00
#define H264_VALID_SPS   0xBBBBBB00

#define H264_ERR_SPS     0xFFFFFFFF
#define H264_ERR_PPS     0xFFFFFFFF

///////////////////////////////////////////////////////////////////
//Slice Type, Name
#define SLICE_TYPE_P          0
#define SLICE_TYPE_B          1
#define SLICE_TYPE_I          2
#define SLICE_TYPE_SP         3
#define SLICE_TYPE_SI         4
#define SLICE_TYPE_PinP       5
#define SLICE_TYPE_BinB       6
#define SLICE_TYPE_IinI       7

#define SLICE_NAME_P   0
#define SLICE_NAME_B   1
#define SLICE_NAME_I   2
#define SLICE_NAME_SP  3
#define SLICE_NAME_SI  4


////////////////////////////////////////////////////////////////////
//H264 Parametners
#define NEW_MMCO_RESETS_E121 0

#define __DEFAULT_WIDTH     176
#define __DEFAULT_HEIGHT    144

#define QUANT_SHIFT         12
#define MIN_QUANT           0
#define MAX_QUANT           51
#define BLOCK_SIZE          4
#define MB_BLOCK_SIZE       16
#define MB_PIXELS           384             // (Not exactly, number of 8-bit components)

#define PRED_SAMPLES_NORTH  2  
#define PRED_SAMPLES_WEST   2
#define PRED_SAMPLES_EAST   3  
#define PRED_SAMPLES_SOUTH  3

#define MAX_REF_FRAMES      10
#define MAX_PIC_PTR         ((MAX_REF_FRAMES+1)*2)

#define MAX_MBNUM_Y         (240/16)        // QVGA

#define MAX_POC_CYCLE       2

#define MAX_SEQ_PARAMETERS  32
#define MAX_PIC_PARAMETERS  256

#define MAX_NAL_SIZE        (32*1024)

// Reference Frame Reordering Parameters
//
// The maximum number of reordering operations shall not exceed
// num_ref_idx_l0_active_minus1 + 1 <  num_ref_frames <= 16
// ----------------------------------------------------------------------------
#define MAX_RRCO                        17                        


// Adaptive buffering commands
// ----------------------------------------------------------------------------
#define MMCO_NOOP                       0
#define MMCO_MARK_SHORT_TERM_UNUSED     1
#define MMCO_MARK_LONG_TERM_UNUSED      2
#define MMCO_ASSIGN_LONG_TERM_INDEX     3
#define MMCO_SET_LONG_TERM_BUFFER       4

#if    (NEW_MMCO_RESETS_E121 == 1)
   #define MMCO_RESET_REF               5
   #define MMCO_RESET_NON_REF           6
   #define MMCO_RESET_ALL               7
#else
   #define MMCO_RESET                   5
#endif

#define MAX_MMCO                        20

#define MAX_REORDERING_OPERATIONS       32
#define MAX_MARKINGCONTROL_OPERATIONS   32

#define H264_SIMULATION

#ifdef  H264_SIMULATION
//#include <stdio.h>
//#define H264_PRINT(X) printf(X);
#endif


///////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// MacroBlock Type

// Macroblock type vlaues 0 to 4 for P and SP slice
// mb_type  Nameofmb_type  NumMbPart(mb_type)  MbPartPredMode(mb_type,0)  MbPartPredMode(mb_type,1), MbParatWidth, MbPartHeight
// 0        P_L0_16x16     1                   Pred_L0                    na                         16            16
// 1        P_L0_L0_16x8   2                   Pred_L0                    Pred_L0
// 2        P_L0_L0_8x16   2                   Pred_L0                    Pred_L0
// 3        P_8x8          4                   na                         na
// 4        P_8x8ref0      4                   na                         na
// inferred P_Skip         1                   Pred_L0                    na                         16            16


// Macroblock types for I slices
// mb_type
// 0        I_4x4
// 1        I_16x16_0_0_0
// 2
// 3
// 4
// 5
// 6
// 7
// 8
// 9
// 10
// 11
// 12       I_16x16_3_2_0
// 13       I_16x16_0_0_1
// 14       I_16x16_1_0_1
// ...
// 25       I_PCM


// Macroblock types
#define I_4x4         0
#define I_16x16       1
#define I_PCM         25

#define P_L0_16x16    0
#define P_L0_L0_16x8  1
#define P_L0_L0_8x16  2
#define P_8x8         3
#define P_8x8ref0     4
#define P_SKIP        99

#define P_I_4x4         (0+5)
#define P_I_16x16       (1+5)
#define P_I_PCM         (25+5)

#define P_MAX_MBTYPE  5
#define I_MAX_MBTYPE  25
#define P_I_MAX_MBTYPE  (P_MAX_MBTYPE+I_MAX_MBTYPE)

// Subpartition types
#define P_L0_8x8      0
#define P_L0_8x4      1
#define P_L0_4x8      2
#define P_L0_4x4      3


////////////////////////////////////////////////////////////////
// VLD Definition
/**
  * Macros
  */
#define H264_VLD_API_SUCCESS                    (0)  
#define H264_VLD_API_INVALID_PARAMETER          (-1)  
#define H264_VLD_API_OUT_OF_MEMORY              (-2)  
#define H264_VLD_MISSING_REFERENCE              (-3)  
#define H264_VLD_LFCQ_TOO_LONG                  (-5)  
#define H264_VLD_INVALID_BUFFER                 (-4) 

// mMBType values
#define H264_VLD_INTRA           0
#define H264_VLD_P_L0_16x16      0x80
#define H264_VLD_P_L0_16x8       0x81
#define H264_VLD_P_L0_8x16       0x82
#define H264_VLD_P_8x8           0x83
#define H264_VLD_P_8x8ref0       0x84
#define H264_VLD_INTER           0x85
#define H264_VLD_PCM             0x40
#define H264_VLD_PKIP            0xC0
#define INVALID_MB_TYPE          0xFF

// Macroblock prediction modes
#define Intra_4x4     0
#define Intra_16x16   0x80
#define NA            2
#define Pred_L0       3

#define IsIntraPredMode(x)    (((x) == Intra_4x4) || ((x) == Intra_16x16))
#define IsInterPredMode(x)    ((x) == 3)

#define AP_MIN(a, b) ((a) < (b) ? (a) : (b))
#define AP_MAX(a, b) ((a) > (b) ? (a) : (b))
#define AP_Median(x,y,z)  (x + y + z - AP_MIN(x, AP_MIN(y,z)) - AP_MAX(x, AP_MAX(y,z)))
#define AP_ABS(x)       ((x > 0) ? (x) : (-(x)))


//FREXT Profile IDC definitions
#define FREXT_HP        100      //!< YUV 4:2:0/8 "High"
#define FREXT_Hi10P     110      //!< YUV 4:2:0/10 "High 10"
#define FREXT_Hi422     122      //!< YUV 4:2:2/10 "High 4:2:2"
#define FREXT_Hi444     244      //!< YUV 4:4:4/14 "High 4:4:4"
#define FREXT_CAVLC444   44      //!< YUV 4:4:4/14 "CAVLC 4:4:4"

#define MMP_H264_YUV400 0
#define MMP_H264_YUV420 1
#define MMP_H264_YUV422 2
#define MMP_H264_YUV444 3


////////////////////////////////////////////////////////////////////////
// class CMmpH264SPS

void CMmpH264SPS::AnalyzeInfo()
{
    mb_row_count=pic_height_in_map_units_minus1+1;
    mb_col_count=pic_width_in_mbs_minus_1+1;
    mb_count=mb_row_count*mb_col_count;
    pic_width=mb_col_count*16;
    pic_height=mb_row_count*16;
}

/////////////////////////////////////////////////////////////////////////
//class CMmpH264Parser

MMP_RESULT CMmpH264Parser::Seq_parameter_set_rbsp(CMmpBitExtractor* pBE, CMmpH264SPS* pSPS)
{
    MMP_RESULT apResult;

    apResult=CMmpH264Parser::Seq_parameter_set_data(pBE, pSPS);
    if( apResult==MMP_SUCCESS )
    {
        apResult=CMmpH264Parser::rbsp_trailing_bits(pBE, pSPS);
    }
    return apResult;
}

MMP_RESULT CMmpH264Parser::Seq_parameter_set_data(CMmpBitExtractor* pBE, CMmpH264SPS* pSPS)
{
    unsigned int pv;
    unsigned long pt;
    
    pv=pBE->Pop_BitCode(pt, 24);
    pSPS->seq_parameter_set_id = pBE->ue_GolombCode();
    if( pSPS->seq_parameter_set_id > MAX_SEQ_PARAMETERS)
    {
        return MMP_FAILURE;//MMP_ERROR_H264VDECODER_INVALID_SPS;
    }

    // Now that x points to something meaningful, setting up
    // PROFILE and LEVEL
    pSPS->level_idc               = (unsigned char) pv;
    pv >>= 8;
    pSPS->constraint_set012_flags = (unsigned char) pv;
    pv >>= 8;
    pSPS->profile_idc             = (unsigned char) pv;

    if ((pSPS->profile_idc!=66 ) &&
      (pSPS->profile_idc!=77 ) &&
      (pSPS->profile_idc!=88 ) &&
      (pSPS->profile_idc!=FREXT_HP    ) &&
      (pSPS->profile_idc!=FREXT_Hi10P ) &&
      (pSPS->profile_idc!=FREXT_Hi422 ) &&
      (pSPS->profile_idc!=FREXT_Hi444 ) &&
      (pSPS->profile_idc!=FREXT_CAVLC444 ))
    {
        //Not supported profile
        return MMP_FAILURE; 
    }

    if((pSPS->profile_idc==FREXT_HP   ) ||
     (pSPS->profile_idc==FREXT_Hi10P) ||
     (pSPS->profile_idc==FREXT_Hi422) ||
     (pSPS->profile_idc==FREXT_Hi444) ||
     (pSPS->profile_idc==FREXT_CAVLC444))
     {
        pSPS->chroma_format_idc                      = pBE->ue_GolombCode();//ue_v ("SPS: chroma_format_idc"                       , s);

        if(pSPS->chroma_format_idc == MMP_H264_YUV444)
        {
             pSPS->separate_colour_plane_flag         = pBE->Pop_1Bit(); //u_1  ("SPS: separate_colour_plane_flag"              , s);
        }

        pSPS->bit_depth_luma_minus8                  = pBE->ue_GolombCode();//ue_v ("SPS: bit_depth_luma_minus8"                   , s);
        pSPS->bit_depth_chroma_minus8                = pBE->ue_GolombCode();//ue_v ("SPS: bit_depth_chroma_minus8"                 , s);
        pSPS->img_lossless_qpprime_fla               = pBE->Pop_1Bit(); //u_1  ("SPS: lossless_qpprime_y_zero_flag"            , s);

        pSPS->seq_scaling_matrix_present_flag        = pBE->Pop_1Bit();//u_1  (   "SPS: seq_scaling_matrix_present_flag"       , s);

        if(pSPS->seq_scaling_matrix_present_flag)
        {
#if 0
             n_ScalingList = (pSPS->chroma_format_idc != YUV444) ? 8 : 12;
             for(i=0; i<n_ScalingList; i++)
             {
                pSPS->seq_scaling_list_present_flag[i]   = pBE->Pop_1Bit(); //u_1  (   "SPS: seq_scaling_list_present_flag"         , s);
                if(pSPS->seq_scaling_list_present_flag[i])
                {
                  if(i<6)
                    Scaling_List(pSPS->ScalingList4x4[i], 16, &pSPS->UseDefaultScalingMatrix4x4Flag[i], s);
                  else
                    Scaling_List(pSPS->ScalingList8x8[i-6], 64, &pSPS->UseDefaultScalingMatrix8x8Flag[i-6], s);
                }
             }
#else
             return MMP_FAILURE; //Not Support Scaling 
#endif
        }

    }

     
    pSPS->log2_max_frame_num_minus4      = pBE->ue_GolombCode();
    pSPS->pic_order_cnt_type             = pBE->ue_GolombCode();
    if(pSPS->pic_order_cnt_type == 0)
    {
        pSPS->log_max_pic_order_cnt_lsb_minus4 = pBE->ue_GolombCode();
    }
    else if (pSPS->pic_order_cnt_type == 1)
    {
        pSPS->delta_pic_order_always_zero_flag = pBE->Pop_1Bit(); //pBE->Query_1Bit();
        //pBE->FlushLast();
        pSPS->offset_for_non_ref_pic           = pBE->se_GolombCode();
        pSPS->offset_for_top_to_bottom_field   = pBE->se_GolombCode();
        pSPS->num_ref_frames_in_pic_order_cnt_cycle = pBE->ue_GolombCode();
      
        int i;
        for (i=0;i<pSPS->num_ref_frames_in_pic_order_cnt_cycle;i++)
        {
           pSPS->offset_for_ref_frame[i] = pBE->se_GolombCode();
        }  
    }
   
    pSPS->num_ref_frames = pBE->ue_GolombCode();
    pSPS->gaps_in_frame_num_value_allowed_flag = pBE->Pop_1Bit(); //pBE->Query_1Bit();
    //pBE->FlushLast();
   
    // Sequence sizes (Width and Height)
    pSPS->pic_width_in_mbs_minus_1             = pBE->ue_GolombCode();
    pSPS->pic_height_in_map_units_minus1       = pBE->ue_GolombCode();
   
    pSPS->frame_msb_only_flag                  = pBE->Pop_1Bit(); //pBE->Query_1Bit();
   
    if (pSPS->frame_msb_only_flag == 0) //Always 1 for Baseline
    {
        pSPS->mb_adaptive_frame_field_flag      = pBE->Pop_1Bit();//pBE->FlushLastAndQuery_1Bit();
    }
    else
    {
        pSPS->mb_adaptive_frame_field_flag      = 0;
    }
   
    pSPS->direct_8x8_inference_flag = pBE->Pop_1Bit();//pBE->FlushLastAndQuery_1Bit();
   
    pSPS->frame_cropping_flag       = pBE->Pop_1Bit();//pBE->FlushLastAndQuery_1Bit();
    //pBE->FlushLast();
   
    if (pSPS->frame_cropping_flag)
    {
        pSPS->frame_crop_left_offset     = pBE->ue_GolombCode();
        pSPS->frame_crop_right_offset    = pBE->ue_GolombCode();
        pSPS->frame_crop_top_offset      = pBE->ue_GolombCode();
        pSPS->frame_crop_bottom_offset   = pBE->ue_GolombCode();     
    }
   
    // VUI (NEED to be implemented, would be required by the player/GDI)
#if 0   
    sequencedata.mSeqParameterSetId      = seq_parameter_set_id;
    sequencedata.mImageWidth             = (pSPS->pic_width_in_mbs_minus_1 + 1) * 16;
    sequencedata.mImageHeight            = (pSPS->pic_height_in_map_units_minus1 + 1) * 16;
    sequencedata.mLog2MaxFrameNumMinus4  = pSPS->log2_max_frame_num_minus4;
    sequencedata.mNumberOfReferenceFrame = pSPS->num_ref_frames;
   
    int result = pVLD_API->h264_RegisterSequence(&sequencedata); 
    if (result == H264_VLD_API_SUCCESS)
    {   
      // Mark the SPS as VALID
      pSPS->mStatus = H264_VALID_SPS;
    }
    else
    {  
      //translate result from VLD error to h264 error
      if (result == H264_VLD_API_OUT_OF_MEMORY)
         rv = H264_ERR_OUT_OF_MEMORY;
      else if (result ==  H264_VLD_MISSING_REFERENCE)
         rv = H264_ERR_MISSING_REFERENCE;
      else          
         rv = MMP_ERROR;
    }   
#endif

    // Test for SIMPLE PROFILE
    //if (pSPS->profile_idc != 66)
    //{
    //    return MMP_FAILURE;//MMP_ERROR_H264VDECODER_ONLY_SUPPORT_BASELINE_PROFILE;
    //}

    return MMP_SUCCESS;
}

MMP_RESULT CMmpH264Parser::rbsp_trailing_bits(CMmpBitExtractor* pBE, CMmpH264SPS* pSPS)
{
    return MMP_SUCCESS;
}

MMP_RESULT CMmpH264Parser::ConvertDavinciH264Foramt(unsigned char* pAu, int auSize)
{
    CMmpBitExtractor be;
    unsigned long dummy;
    unsigned char NALHeaderByte;
    unsigned char ForbiddenBit;
    unsigned char NALRefIdc;
    unsigned char NALUnitType;
    int iSPSIndex, iPPSIndex, iSliceIndex;
    int iCurIndex;
    int iSPSSize, iPPSSize, iSliceSize;
    unsigned char* header;
    int i;
    
    iSPSIndex=-1;
    iPPSIndex=-1;
    iSliceIndex=-1;
    
    iSPSSize=0;
    iPPSSize=0;
    iSliceSize=0;
    
    be.Start(pAu, auSize);
    
    while(1)
    {
        if( be.Decode_NextStartCodePrefix4()!=MMP_SUCCESS )
        {
            break;
        }    
        
        NALHeaderByte=(unsigned char)be.Pop_BitCode(dummy, 8);
        ForbiddenBit  = NALHeaderByte >> 7; 
        if(ForbiddenBit) 
        {
            return MMP_FAILURE;
        }
        NALRefIdc   = (NALHeaderByte & 0x60) >> 5;
        NALUnitType = (NALHeaderByte & 0x1F);     

        iCurIndex=be.GetCurByteIndex()-5;

        if( NALUnitType==NAL_TYPE_SEQ_PARAM_SET ) //SPS
        {
            iSPSIndex=iCurIndex;
        }    
        else if( NALUnitType==NAL_TYPE_PIC_PARAM_SET ) //PPS
        {
            iPPSIndex=iCurIndex;
        }
        else
        {
            iSliceIndex=iCurIndex;
            break;
        }    
        
    }
    
    if(iSliceIndex<0)
    {
        if( iSPSIndex!=0 || iPPSIndex<=0 ) //if exist SPS PPS
           return MMP_FAILURE;
          
        iSPSSize=iPPSIndex-iSPSIndex-4;
        iPPSSize=auSize-iPPSIndex-4;
    }
    else if( iSPSIndex==0 && iPPSIndex>0 ) //if exist  SPS PPS Slice
    {
        if(iSliceIndex<=iPPSIndex)
           return MMP_FAILURE;  
    
        iSPSSize=iPPSIndex-iSPSIndex-4;
        iPPSSize=iSliceIndex-iPPSIndex-4;
        iSliceSize=auSize-iSliceIndex-4;
    }
    else if(iSliceIndex==0) 
    {
        iSliceSize=auSize-4;
    }
    else
        return MMP_FAILURE;
        
    
    if(iSliceSize>0)  
    {
        header=(unsigned char*)&pAu[iSliceIndex];
        for(i=0;i<4;i++)
        {
            header[i]=(iSliceSize>>(24-i*8))&0xFF;
        }
    }
    
    if(iSPSSize>0)  
    {
        header=(unsigned char*)&pAu[iSPSIndex];
        for(i=0;i<4;i++)
        {
            header[i]=(iSPSSize>>(24-i*8))&0xFF;
        }
    }
    
    if(iPPSSize>0)  
    {
        header=(unsigned char*)&pAu[iPPSIndex];
        for(i=0;i<4;i++)
        {
            header[i]=(iPPSSize>>(24-i*8))&0xFF;
        }
    }
    
    return MMP_SUCCESS;
}

MMP_RESULT CMmpH264Parser::ConvertDavinciDSIForamt(unsigned char* pAu, int auSize, int* dsiSize)
{
    CMmpBitExtractor be;
    unsigned long dummy;
    unsigned char NALHeaderByte;
    unsigned char ForbiddenBit;
    unsigned char NALRefIdc;
    unsigned char NALUnitType;
    int iSPSIndex, iPPSIndex, iSliceIndex;
    int iCurIndex;
    int iSPSSize, iPPSSize, iSliceSize;
    unsigned char* header;
    int i;
    
    iSPSIndex=-1;
    iPPSIndex=-1;
    iSliceIndex=-1;
    
    iSPSSize=0;
    iPPSSize=0;
    iSliceSize=0;
    
    be.Start(pAu, auSize);
    
    while(1)
    {
        if( be.Decode_NextStartCodePrefix4()!=MMP_SUCCESS )
        {
            break;
        }    
        
        NALHeaderByte=(unsigned char)be.Pop_BitCode(dummy, 8);
        ForbiddenBit  = NALHeaderByte >> 7; 
        if(ForbiddenBit) 
        {
            return MMP_FAILURE;
        }
        NALRefIdc   = (NALHeaderByte & 0x60) >> 5;
        NALUnitType = (NALHeaderByte & 0x1F);     

        iCurIndex=be.GetCurByteIndex()-5;

        if( NALUnitType==NAL_TYPE_SEQ_PARAM_SET ) //SPS
        {
            iSPSIndex=iCurIndex;
        }    
        else if( NALUnitType==NAL_TYPE_PIC_PARAM_SET ) //PPS
        {
            iPPSIndex=iCurIndex;
        }
        else
        {
            iSliceIndex=iCurIndex;
            break;
        }    
        
    }
    
    if(iSliceIndex<0 &&  iSPSIndex==0 && iPPSIndex>0 )
    {
        iSPSSize=iPPSIndex-iSPSIndex-4;
        iPPSSize=auSize-iPPSIndex-4;
    }
    else if( iSPSIndex==0 && iPPSIndex>0 ) //if exist  SPS PPS Slice
    {
        if(iSliceIndex<=iPPSIndex)
           return MMP_FAILURE;  
    
        iSPSSize=iPPSIndex-iSPSIndex-4;
        iPPSSize=iSliceIndex-iPPSIndex-4;
        iSliceSize=auSize-iSliceIndex-4;
    }
    else 
    {
        return MMP_FAILURE;
    }
    
    if(iSliceSize>0)  
    {
        header=(unsigned char*)&pAu[iSliceIndex];
        for(i=0;i<4;i++)
        {
            header[i]=(iSliceSize>>(24-i*8))&0xFF;
        }
    }
    
    if(iSPSSize>0)  
    {
        header=(unsigned char*)&pAu[iSPSIndex];
        for(i=0;i<4;i++)
        {
            header[i]=(iSPSSize>>(24-i*8))&0xFF;
        }
    }
    
    if(iPPSSize>0)  
    {
        header=(unsigned char*)&pAu[iPPSIndex];
        for(i=0;i<4;i++)
        {
            header[i]=(iPPSSize>>(24-i*8))&0xFF;
        }
    }
    
    *dsiSize=8+iSPSSize+iPPSSize;
    
    return MMP_SUCCESS;
}

MMP_RESULT CMmpH264Parser::ConvertDavinciAuForamt(unsigned char* pAu, int auSize, unsigned int* realAuIndex, int* realAuSize)
{
    CMmpBitExtractor be;
    unsigned long dummy;
    unsigned char NALHeaderByte;
    unsigned char ForbiddenBit;
    unsigned char NALRefIdc;
    unsigned char NALUnitType;
    int iSPSIndex, iPPSIndex, iSliceIndex;
    int iCurIndex;
    int iSPSSize, iPPSSize, iSliceSize;
    unsigned char* header;
    int i;
    
    iSPSIndex=-1;
    iPPSIndex=-1;
    iSliceIndex=-1;
    
    iSPSSize=0;
    iPPSSize=0;
    iSliceSize=0;
    
    be.Start(pAu, auSize);
    
    while(1)
    {
        if( be.Decode_NextStartCodePrefix4()!=MMP_SUCCESS )
        {
            break;
        }    
        
        NALHeaderByte=(unsigned char)be.Pop_BitCode(dummy, 8);
        ForbiddenBit  = NALHeaderByte >> 7; 
        if(ForbiddenBit) 
        {
            return MMP_FAILURE;
        }
        NALRefIdc   = (NALHeaderByte & 0x60) >> 5;
        NALUnitType = (NALHeaderByte & 0x1F);     

        iCurIndex=be.GetCurByteIndex()-5;

        if( NALUnitType==NAL_TYPE_SEQ_PARAM_SET ) //SPS
        {
            iSPSIndex=iCurIndex;
        }    
        else if( NALUnitType==NAL_TYPE_PIC_PARAM_SET ) //PPS
        {
            iPPSIndex=iCurIndex;
        }
        else
        {
            iSliceIndex=iCurIndex;
            break;
        }    
        
    }
    
    if(iSliceIndex<0 )
    {
        return MMP_FAILURE;
    }
    else if( iSPSIndex==0 && iPPSIndex>0 ) //if exist  SPS PPS Slice
    {
        if(iSliceIndex<=iPPSIndex)
           return MMP_FAILURE;  
    
        iSPSSize=iPPSIndex-iSPSIndex-4;
        iPPSSize=iSliceIndex-iPPSIndex-4;
        iSliceSize=auSize-iSliceIndex-4;
    }
    else if(iSliceIndex==0) 
    {
        iSliceSize=auSize-4;
    }
    else if(iSliceIndex>0 && iPPSIndex==0 ) 
    {
        iPPSSize=iSliceIndex-iPPSIndex-4;
        iSliceSize=auSize-iSliceIndex-4;
    }
    else 
    {
        return MMP_FAILURE;
    }
    
    if(iSliceSize>0)  
    {
        header=(unsigned char*)&pAu[iSliceIndex];
        for(i=0;i<4;i++)
        {
            header[i]=(iSliceSize>>(24-i*8))&0xFF;
        }
    }
    
    if(iSPSSize>0)  
    {
        header=(unsigned char*)&pAu[iSPSIndex];
        for(i=0;i<4;i++)
        {
            header[i]=(iSPSSize>>(24-i*8))&0xFF;
        }
    }
    
    if(iPPSSize>0)  
    {
        header=(unsigned char*)&pAu[iPPSIndex];
        for(i=0;i<4;i++)
        {
            header[i]=(iPPSSize>>(24-i*8))&0xFF;
        }
    }
        
    *realAuIndex=0;//iSliceIndex;
    *realAuSize=auSize;//iSliceSize+4;
    
    return MMP_SUCCESS;
}

//MMP_RESULT CMmpH264Parser::ConvertAvc1ToH264(unsigned char* pAvc1Au, unsigned char* pH264Au, int auSize)
MMP_RESULT CMmpH264Parser::ConvertAvc1ToH264(MMP_U8* pAvc1Au, MMP_S32 avc_size, MMP_S32 nal_length_size, MMP_U8* pH264Au, MMP_S32* h264_size, MMP_BOOL is_one_nal)
{
    MMP_U32 sz;
    MMP_U32 sz32;
    MMP_U16 sz16;
    int avc_idx = 0, h264_idx = 0;
    MMP_RESULT mmpResult = MMP_SUCCESS;
        
    if(avc_size < (nal_length_size+1) ) {
        mmpResult = MMP_FAILURE;
    }

    if( (nal_length_size == 2) 
        || (nal_length_size == 3) 
        || (nal_length_size == 4) 
        ){
        /* Nothing to do */
    }
    else {
        return MMP_FAILURE;
    }
  
    if(mmpResult == MMP_SUCCESS) {

        for(avc_idx=0; avc_idx<avc_size; )
        {
            //Copy StartCode
            if(nal_length_size == 4) {
                memcpy(&sz32, pAvc1Au, nal_length_size);
                sz = sz32;
                sz=MMP_SWAP_U32(sz);
            }
            else if(nal_length_size == 3) {
                sz = ((pAvc1Au[0]<<16)&0x00FF0000) | ((pAvc1Au[1]<<8)&0xFF00) | ((pAvc1Au[2]<<0)&0xFF) ;
            }
            else if(nal_length_size == 2) {
                memcpy(&sz16, pAvc1Au, nal_length_size);
                sz16 = MMP_SWAP_U16(sz16);
                sz = (MMP_U32)sz16;
            }

            if( (avc_idx+sz) > (MMP_U32)avc_size) {
               if(h264_size) *h264_size = 0; 
               return MMP_FAILURE;
            }

            pAvc1Au += nal_length_size;
            avc_idx += nal_length_size; 

            pH264Au[0]=0;
            pH264Au[1]=0;
            pH264Au[2]=0;
            pH264Au[3]=1;

            pH264Au+=4;
            h264_idx +=4;
                        
            memcpy(pH264Au, pAvc1Au, sz);

            pH264Au += sz;
            h264_idx += sz;

            pAvc1Au += sz;
            avc_idx += sz;

            if(is_one_nal == MMP_TRUE) {
                break;
            }
        }
    }

    if(h264_size) *h264_size = h264_idx;

    return mmpResult;
}

#if 0
MMP_RESULT CMmpH264Parser::ConvertH264FormatSPSPPSToAvc1(unsigned char* stream, int streamSize )
{
    MMP_RESULT mmpResult;
    CMmpBitExtractor be;
    bool bRun;
    unsigned long dummy;

    unsigned char NALHeaderByte;
    unsigned char ForbiddenBit;
    unsigned char NALRefIdc;
    unsigned char NALUnitType;
    int spsIndex, ppsIndex;
    
    if( !(stream[0]==0x00 &&
       stream[1]==0x00 &&
       stream[2]==0x00 &&
       stream[3]==0x01 )
    {
        return MMP_SUCCESS;
    }


    be.Start(stream, streamSize);

    pBE=&be;
    mmpResult=MMP_FAILURE;
    bRun = true;
    while(bRun)
    {
        if( MMP_SUCCESS != be.Decode_NextStartCodePrefix4() )   // find 0x000001 <- start code
        {
            break;
        }

        NALHeaderByte=(unsigned char)be.Pop_BitCode(dummy, 8);
        ForbiddenBit  = NALHeaderByte >> 7; 
        if(ForbiddenBit) 
        {
            break;
        }
        NALRefIdc   = (NALHeaderByte & 0x60) >> 5;
        NALUnitType = (NALHeaderByte & 0x1F);     

        switch(NALUnitType)
        {
            case NAL_TYPE_SLICE_IDR:  //I Slice
            case NAL_TYPE_SLICE_NO_PART:  //P Slice
                break;

            case NAL_TYPE_SEQ_PARAM_SET: //SPS
                break;

            case NAL_TYPE_PIC_PARAM_SET: //PPS
                break;
        }

    }

    return MMP_SUCCESS;
}
#endif

MMP_RESULT CMmpH264Parser::GetNextNalInfo(unsigned char* stream, int streamSize,  //IN
                                          int* nalStartCodeIndex, //OUT,  start code index 
                                          int* nalIndex,          //OUT
                                          unsigned char* nal,     //OUT
                                          bool* bStandard,        //OUT
                                          int* nextNalStartCodeIndex )       //OUT 
{
#if 0
    CMmpBitExtractor be;
    unsigned long dummy;
    unsigned int codes[4];
    bool bAvc;
    unsigned char NALHeaderByte;
    unsigned char ForbiddenBit;
    unsigned char NALRefIdc;
    unsigned char NALUnitType;

    if(streamSize<4)
    {
        return MMP_FAILURE;
    }   

    be.Start(steam, streamSize);

    codes[0] = (unsigned int)this->Pop_BitCode(dummy, 8);
    codes[1] = (unsigned int)this->Pop_BitCode(dummy, 8);
    codes[2] = (unsigned int)this->Pop_BitCode(dummy, 8);
    codes[3] = (unsigned int)this->Pop_BitCode(dummy, 8);

    if( (codes[0] == 0x00) && (codes[1] == 0x00) && (codes[2] == 0x01) )
    {
       bAvc=false;

       NALHeaderByte=(unsigned char)be.Pop_BitCode(dummy, 8);
       ForbiddenBit  = NALHeaderByte >> 7; 
       if(ForbiddenBit==0) 
       {
            NALRefIdc  = (NALHeaderByte & 0x60) >> 5;
            NALUnitType= (NALHeaderByte & 0x1F);          

       }
       
    }
#else

    return MMP_FAILURE;
#endif


}

MMP_BOOL CMmpH264Parser::IsThisStreamHas(MMP_U8* stream, MMP_S32 streamSize, MMP_U8 nalType, MMP_S32* nalIndex, MMP_S32 *nalSize)
{
    CMmpBitExtractor be;
    MMP_U8 codes[4];
    MMP_U8 NALHeaderByte;
    MMP_U8 ForbiddenBit;
    MMP_U8 NALRefIdc;
    MMP_U8 NALUnitType;
    MMP_S32 auSize;
    MMP_S32 i;
    MMP_BOOL bAvc, bflag;
    MMP_U8* orgstream;
    
    orgstream = stream;
    if(nalIndex) *nalIndex = -1;
    if(nalSize) *nalSize = 0;
        
    while(1) {

        if(streamSize<5) {
            return MMP_FALSE;
        }

        codes[0] = stream[0];
        codes[1] = stream[1];
        codes[2] = stream[2];
        codes[3] = stream[3];

        if( (codes[0] == 0x00) && (codes[1] == 0x00) && (codes[2] == 0x00) && (codes[3] == 0x01) ) {
            bAvc = MMP_FALSE;
        }
        else {
            memcpy(&auSize, codes, 4);
            auSize=MMP_SWAP_U32(auSize);
            if(auSize>streamSize-4)  {
                return MMP_FALSE;
            }
            bAvc = MMP_TRUE;
        }

        NALHeaderByte=(MMP_U8)stream[4];
        ForbiddenBit  = NALHeaderByte >> 7; 
        if(ForbiddenBit==0) {

            NALRefIdc  = (NALHeaderByte & 0x60) >> 5;
            NALUnitType= (NALHeaderByte & 0x1F);

            if(NALUnitType==nalType) {

                if(nalIndex) *nalIndex=(int)(stream-orgstream)+4;
                if(nalSize) {
                    
                    if(bAvc == MMP_TRUE) {
                        *nalSize = auSize;
                    }
                    else {
                        CMmpBitExtractor be;
                        MMP_S32 remain_stream_size = streamSize-4;
                        MMP_U8* remain_stream = stream+4;
                        MMP_S32 next_start_code_index;
                        MMP_S32 prefix_size;

                        be.Start(remain_stream, remain_stream_size);
                        if( CMmpH264Parser::Decode_NextNal_H264(&be, remain_stream_size, &prefix_size) == MMP_SUCCESS) {
                            next_start_code_index = be.GetCurByteIndex() - prefix_size;
                            *nalSize = next_start_code_index;
                        }
                        else {
                            *nalSize = remain_stream_size;
                        }

                    }

                }
                return MMP_TRUE;
            }
        }

        if(bAvc)
        {
            stream+=auSize+4;
            streamSize-=auSize+4;
        }
        else
        {
            codes[0] = codes[1];
            codes[1] = codes[2];
            codes[2] = codes[3];
            for(i=1, bflag=false; i<streamSize-2;i++)
            {
                codes[3]=stream[i+3];    
                if( (codes[0] == 0x00) && (codes[1] == 0x00) && (codes[2] == 0x00) && (codes[3] == 0x01) )
                {
                    bflag=true;
                    break;
                }

                codes[0] = codes[1];
                codes[1] = codes[2];
                codes[2] = codes[3];
            }
            if(bflag)
            {
                stream+=i;
                streamSize-=i;
            }
            else
            {
                return false;
            }
        }

    }


    return false;
}


MMP_RESULT CMmpH264Parser::Remake_VideoDSI(unsigned int fourcc, unsigned char* dsi, int dsiSize, int* newDsiSize, int* spsIndex, int* spsSize, int* ppsIndex, int* ppsSize)
{
    switch(fourcc)
    {
        case MMPMAKEFOURCC('A','V','C','1'):
        case MMPMAKEFOURCC('a','v','c','1'):
            return CMmpH264Parser::Remake_VideoDSI_AVC1(dsi, dsiSize, newDsiSize, spsIndex, spsSize, ppsIndex, ppsSize);

        case MMPMAKEFOURCC('H','2','6','4'):
            return CMmpH264Parser::Remake_VideoDSI_H264(dsi, dsiSize, newDsiSize, spsIndex, spsSize, ppsIndex, ppsSize);
    }

    return MMP_FAILURE;
}

/*

Namo_352x288_AVC1 DSI
01 42 c0 14 ff e1 00 15 67 42 c0 14 f4 0b 04 b4 20 00 00 03 00 20 00 00 06 01 e2 85 54 01 00 04 68 ce 3c 80 

ConfigurationVersion : 01
ACVProfileIndication : 42
ProfileCompatibility : c0
AVCLevelIndication   : 14
lenghtSizeMinusOne   : ff

counter   : e1      e1&1F = 01  SPS Counter
SPS Size  : 00 15  
SPS       : 67 42 c0 14 f4 0b 04 b4 20 00 00 03 00 20 00 00 06 01 e2 85 54

counter   : 01
PPS Size  : 00 04
PPS       : 68 ce 3c 80
*/

MMP_RESULT CMmpH264Parser::Remake_VideoDSI_H264(unsigned char* dsi, int dsiSize, int* newDsiSize, int* spsIndex, int* spsSize, int* ppsIndex, int* ppsSize)
{
   unsigned char ConfigurationVersion;
   unsigned char lenghtSizeMinusOne;
   unsigned char ACVProfileIndication;
   unsigned char ProfileCompatibility;
   unsigned char AVCLevelIndication;
   unsigned char* ByteStreamBuffer;
   int dsiIndex;
   unsigned char* pSPSStream;

   if(dsiSize==0 || dsi==NULL)
   {
        return MMP_FAILURE;
   }

   ByteStreamBuffer=dsi;

   ConfigurationVersion = *ByteStreamBuffer++;
   
   ACVProfileIndication  = *ByteStreamBuffer++;
   ProfileCompatibility  = *ByteStreamBuffer++;
   AVCLevelIndication    = *ByteStreamBuffer++;
   lenghtSizeMinusOne   = *ByteStreamBuffer++;
   lenghtSizeMinusOne &= 0x03; // 0b00000011 Keep only the 2 lsb

   dsiIndex=0;

   unsigned char counter = *ByteStreamBuffer++;
   unsigned char i;
   unsigned short sizeOfSPS, sizeOfPPS;

   //make SPS
   counter &= 0x1F; // 0b00011111 Keep only the 5 lsb
   for(i=0;i<counter;i++)
   {
       sizeOfSPS = 0;
       sizeOfSPS |=  (*ByteStreamBuffer++ << 8);
       sizeOfSPS |=  (*ByteStreamBuffer++); 

       dsi[dsiIndex++]=0;
       dsi[dsiIndex++]=0;
       dsi[dsiIndex++]=0;
       dsi[dsiIndex++]=1;
       memcpy(&dsi[dsiIndex], ByteStreamBuffer, sizeOfSPS ); 
       if(spsIndex) *spsIndex=dsiIndex;
       if(spsSize) *spsSize=sizeOfSPS;
       pSPSStream=(unsigned char*)&dsi[dsiIndex];

       dsiIndex+=sizeOfSPS;
       ByteStreamBuffer+= sizeOfSPS;
   }

   // PPSs
   counter = *ByteStreamBuffer++;
   for (i=0;i<counter;i++)
   {
      sizeOfPPS = 0;
      sizeOfPPS |=  (*ByteStreamBuffer++ << 8);
      sizeOfPPS |=  (*ByteStreamBuffer++);
      
      dsi[dsiIndex++]=0;
      dsi[dsiIndex++]=0;
      dsi[dsiIndex++]=0;
      dsi[dsiIndex++]=1;
      
      memcpy(&dsi[dsiIndex], ByteStreamBuffer, sizeOfPPS ); 
      if(ppsIndex) *ppsIndex=dsiIndex;
      if(ppsSize) *ppsSize=sizeOfPPS;

      dsiIndex+=sizeOfPPS;
      ByteStreamBuffer += sizeOfPPS;
   }
   
   if(newDsiSize) *newDsiSize=dsiIndex;

   //Parsing SPS
   //this->MakeH264SPS(pSPSStream, sizeOfSPS);
   return MMP_SUCCESS;
}

MMP_RESULT CMmpH264Parser::Remake_VideoDSI_AVC1(unsigned char* dsi, int dsiSize, int* newDsiSize, int* spsIndex, int* spsSize, int* ppsIndex, int* ppsSize)
{
   unsigned char ConfigurationVersion;
   //unsigned char CurrentSeqParameterSetIdx;
   //unsigned char CurrentPicParameterSetIdx;
   //unsigned char SliceDataDecoded;
   unsigned char lenghtSizeMinusOne;
   unsigned char ACVProfileIndication;
   unsigned char ProfileCompatibility;
   unsigned char AVCLevelIndication;
   unsigned char* ByteStreamBuffer;
   int dsiIndex;
   unsigned char* pSPSStream;

   if(dsiSize==0 || dsi==NULL)
   {
       return MMP_FAILURE;
   }

   ByteStreamBuffer=dsi;

   ConfigurationVersion = *ByteStreamBuffer++;
   if(ConfigurationVersion != 1)
   {
      return MMP_FAILURE;
   }
  
   ACVProfileIndication  = *ByteStreamBuffer++;
   ProfileCompatibility  = *ByteStreamBuffer++;
   AVCLevelIndication    = *ByteStreamBuffer++;
   lenghtSizeMinusOne   = *ByteStreamBuffer++;
   lenghtSizeMinusOne &= 0x03; // 0b00000011 Keep only the 2 lsb

   dsiIndex=0;

   unsigned char counter = *ByteStreamBuffer++;
   unsigned char i;
   unsigned short sizeOfSPS, sizeOfPPS;
   //unsigned int sliceHeaderMarker;

   //make SPS
   counter &= 0x1F; // 0b00011111 Keep only the 5 lsb
   for(i=0;i<counter;i++)
   {
       sizeOfSPS = 0;
       sizeOfSPS |=  (*ByteStreamBuffer++ << 8);
       sizeOfSPS |=  (*ByteStreamBuffer++); 
       
#if 1
       dsi[dsiIndex++]=0;
       dsi[dsiIndex++]=0;
       dsi[dsiIndex++]=0;
       dsi[dsiIndex++]=1;
#else
       sliceHeaderMarker=sizeOfSPS;
       sliceHeaderMarker&=0xFFFF;
       sliceHeaderMarker=MMP_SWAP_U32(sliceHeaderMarker);
       memcpy(&dsi[dsiIndex], &sliceHeaderMarker, 4);
       dsiIndex+=4;
#endif

       memcpy(&dsi[dsiIndex], ByteStreamBuffer, sizeOfSPS ); 
       if(spsIndex) *spsIndex=dsiIndex;
       if(spsSize) *spsSize=sizeOfSPS;

       pSPSStream=(unsigned char*)&dsi[dsiIndex];

       dsiIndex+=sizeOfSPS;
       ByteStreamBuffer+= sizeOfSPS;
   }
   //dsi[dsiIndex]=0xFC; dsiIndex++;
   //dsi[dsiIndex]=0xA8; dsiIndex++;
   //*spsSize+=2;
   
   // PPSs
   counter = *ByteStreamBuffer++;
   for (i=0;i<counter;i++)
   {
      sizeOfPPS = 0;
      sizeOfPPS |=  (*ByteStreamBuffer++ << 8);
      sizeOfPPS |=  (*ByteStreamBuffer++);
      
#if 1
      dsi[dsiIndex++]=0;
      dsi[dsiIndex++]=0;
      dsi[dsiIndex++]=0;
      dsi[dsiIndex++]=1;
#else
      sliceHeaderMarker=sizeOfPPS;
      sliceHeaderMarker&=0xFFFF;
      sliceHeaderMarker=MMP_SWAP_U32(sliceHeaderMarker);
      memcpy(&dsi[dsiIndex], &sliceHeaderMarker, 4);
      dsiIndex+=4;
#endif
      
      memcpy(&dsi[dsiIndex], ByteStreamBuffer, sizeOfPPS ); 
      if(ppsIndex) *ppsIndex=dsiIndex;
      if(ppsSize) *ppsSize=sizeOfPPS;

      dsiIndex+=sizeOfPPS;
      ByteStreamBuffer += sizeOfPPS;
   }
   
   if(newDsiSize) *newDsiSize=dsiIndex;

   //Parsing SPS
   //this->MakeH264SPS(pSPSStream, sizeOfSPS);
   return MMP_SUCCESS;
}

MMP_RESULT CMmpH264Parser::ConvertDSI_AVC1_To_H264(MMP_U8* avc_dsi, MMP_S32 avc_dsi_size, MMP_U8* h264_dsi, MMP_S32* h264_dsi_size, MMP_S32* is_avc, MMP_S32* nal_length_size) {

   MMP_U8 ConfigurationVersion;
   //unsigned char CurrentSeqParameterSetIdx;
   //unsigned char CurrentPicParameterSetIdx;
   //unsigned char SliceDataDecoded;
   MMP_U8 lenghtSizeMinusOne;
   MMP_U8 ACVProfileIndication;
   MMP_U8 ProfileCompatibility;
   MMP_U8 AVCLevelIndication;
   MMP_U8* ByteStreamBuffer;
   MMP_S32 dsiIndex;
   MMP_S32 nal_size;

   if(is_avc) *is_avc = 0;
   if(nal_length_size) *nal_length_size = 0;
   
   if( (avc_dsi_size == 0) || (avc_dsi == NULL) ) {
       return MMP_FAILURE;
   }

   ByteStreamBuffer = avc_dsi;

   ConfigurationVersion = *ByteStreamBuffer++;
   if(ConfigurationVersion != 1)  {
      return MMP_FAILURE;
   }
      
  
   ACVProfileIndication  = *ByteStreamBuffer++;
   ProfileCompatibility  = *ByteStreamBuffer++;
   AVCLevelIndication    = *ByteStreamBuffer++;
   lenghtSizeMinusOne   = *ByteStreamBuffer++;
   lenghtSizeMinusOne &= 0x03; // 0b00000011 Keep only the 2 lsb

   nal_size = (lenghtSizeMinusOne+1);
   if( (nal_size == 2) 
       || (nal_size == 3) 
       || (nal_size == 4) ) {
      /* Nothing to do */
   }
   else {
        return MMP_FAILURE;
   }

   if(is_avc) *is_avc = 1;
   if(nal_length_size) *nal_length_size = nal_size;
   
   
   dsiIndex=0;

   MMP_U8 counter = *ByteStreamBuffer++;
   MMP_U8 i;
   MMP_U16 sizeOfSPS, sizeOfPPS;
   //MMP_U32 sliceHeaderMarker;

   //make SPS
   counter &= 0x1F; // 0b00011111 Keep only the 5 lsb
   for(i=0;i<counter;i++)
   {
       sizeOfSPS = 0;
       sizeOfSPS |=  (*ByteStreamBuffer++ << 8);
       sizeOfSPS |=  (*ByteStreamBuffer++); 
       
#if 1
       h264_dsi[dsiIndex++]=0;
       h264_dsi[dsiIndex++]=0;
       h264_dsi[dsiIndex++]=0;
       h264_dsi[dsiIndex++]=1;
#else
       sliceHeaderMarker=sizeOfSPS;
       sliceHeaderMarker&=0xFFFF;
       sliceHeaderMarker=MMP_SWAP_U32(sliceHeaderMarker);
       memcpy(&dsi[dsiIndex], &sliceHeaderMarker, 4);
       dsiIndex+=4;
#endif

       memcpy(&h264_dsi[dsiIndex], ByteStreamBuffer, sizeOfSPS ); 
       //if(spsIndex) *spsIndex=dsiIndex;
       //if(spsSize) *spsSize=sizeOfSPS;

       //pSPSStream = (MMP_U8*)&dsi[dsiIndex];

       dsiIndex += sizeOfSPS;
       ByteStreamBuffer += sizeOfSPS;
   }
   //dsi[dsiIndex]=0xFC; dsiIndex++;
   //dsi[dsiIndex]=0xA8; dsiIndex++;
   //*spsSize+=2;
   
   // PPSs
   counter = *ByteStreamBuffer++;
   for (i=0;i<counter;i++)
   {
      sizeOfPPS = 0;
      sizeOfPPS |=  (*ByteStreamBuffer++ << 8);
      sizeOfPPS |=  (*ByteStreamBuffer++);
      
#if 1
      h264_dsi[dsiIndex++]=0;
      h264_dsi[dsiIndex++]=0;
      h264_dsi[dsiIndex++]=0;
      h264_dsi[dsiIndex++]=1;
#else
      sliceHeaderMarker=sizeOfPPS;
      sliceHeaderMarker&=0xFFFF;
      sliceHeaderMarker=MMP_SWAP_U32(sliceHeaderMarker);
      memcpy(&dsi[dsiIndex], &sliceHeaderMarker, 4);
      dsiIndex+=4;
#endif
      
      memcpy(&h264_dsi[dsiIndex], ByteStreamBuffer, sizeOfPPS ); 
      //if(ppsIndex) *ppsIndex=dsiIndex;
      //if(ppsSize) *ppsSize=sizeOfPPS;

      dsiIndex+=sizeOfPPS;
      ByteStreamBuffer += sizeOfPPS;
   }
   
   if(h264_dsi_size != NULL) {
       *h264_dsi_size = dsiIndex;
   }

   //Parsing SPS
   //this->MakeH264SPS(pSPSStream, sizeOfSPS);
   return MMP_SUCCESS;
}

MMP_RESULT CMmpH264Parser::ParsingSPS(unsigned char* spsStream, int spsSize, CMmpH264SPS* pSPS)
{
    CMmpBitExtractor be;
    unsigned long dummy;
    unsigned char NALHeaderByte;
    unsigned char ForbiddenBit;
    unsigned char NALRefIdc;
    unsigned char NALUnitType;

    if(pSPS==NULL)
    {
      return MMP_FAILURE;
    }

    be.Start(spsStream, spsSize);
    NALHeaderByte=(unsigned char)be.Pop_BitCode(dummy, 8);
    ForbiddenBit  = NALHeaderByte >> 7; 
    if(ForbiddenBit) 
    {
       return MMP_FAILURE;
    }
    NALRefIdc  = (NALHeaderByte & 0x60) >> 5;
    NALUnitType= (NALHeaderByte & 0x1F);     

    if(NALUnitType!=NAL_TYPE_SEQ_PARAM_SET)
    {
        return MMP_FAILURE;
    }

    return CMmpH264Parser::Seq_parameter_set_rbsp(&be, pSPS);
}

unsigned int CMmpH264Parser::CheckH264SPS(unsigned int fourcc, CMmpH264SPS* pSPS)
{
    unsigned int error;

    error=MMP_SUCCESS;//VIDEO_DECODER_SUCCESS;

    switch(fourcc)
    {
        case MMPMAKEFOURCC('A','V','C','1'):
        case MMPMAKEFOURCC('a','v','c','1'):
        case MMPMAKEFOURCC('H','2','6','4'):
        case MMPMAKEFOURCC('h','2','6','4'):

        case MMPMAKEFOURCC('X','2','6','4'):
        case MMPMAKEFOURCC('x','2','6','4'):
            if(pSPS)
            {
                switch(pSPS->profile_idc)
                {
                    case 66:  //baseline profine
                        break;

                    case 77:
                        //error=MMP_VIDEO_DECODER_NOT_SUPPORT_AVC_MAINPROFILE;
                        break;

                    case 100:
#if (MMP_OS==MMP_OS_WINCE60)
                        error=MMP_VIDEO_DECODER_NOT_SUPPORT_AVC_HIGHPROFILE;
#endif
                        break;

                    default:        
                        error=MMP_FAILURE;//MMP_VIDEO_DECODER_NOT_SUPPORT_AVC_UNKNONWPROFILE;
                        break;

                }
                
            }
            else
            {
                error=MMP_FAILURE;//error=MMP_VIDEO_DECODER_CANNOT_FIND_AVC_SPS_PPS;
            }
            break;

    }

    return error;
}


MMP_RESULT CMmpH264Parser::Decode_NextNal_H264(CMmpBitExtractor* pBE, MMP_U32 maxcount, MMP_S32* prefix_size)
{
    unsigned long dummy;
    unsigned int codes[4];
    int count = 0;
    MMP_S32 pfsz = 4;

    codes[0] = (unsigned int)pBE->Pop_BitCode(dummy, 8);
    codes[1] = (unsigned int)pBE->Pop_BitCode(dummy, 8);
    codes[2] = (unsigned int)pBE->Pop_BitCode(dummy, 8);

    // Locating the byte-alligned start code prefix 00 00 00 01 or 00 00 01
    // -----------------------------------------------------
    while(1) {

       if( (codes[0] == 0x00) && (codes[1] == 0x00) && (codes[2] == 0x01) )  {
           pfsz = 3;
           break;
       }

       if( !pBE->CanGetBit(8) ) return MMP_FAILURE;

       codes[3] = (unsigned int)pBE->Pop_BitCode(dummy, 8);
       if( (codes[0] == 0x00) && (codes[1] == 0x00) && (codes[2] == 0x00) && (codes[3] == 0x01) )  {
           pfsz = 4;
           break;
       }

       codes[0] = codes[1];
       codes[1] = codes[2];
       codes[2] = codes[3];
       
       count++;
       if( count>(int)maxcount) return MMP_FAILURE;
   }

    if(prefix_size) *prefix_size = pfsz;
   
   return MMP_SUCCESS;
}

