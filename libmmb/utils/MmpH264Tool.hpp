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

#ifndef _MMPH264TOOL_HPP__
#define _MMPH264TOOL_HPP__

#include "MmpBitExtractor.hpp"


///////////////////////////////////////////////////////////
// Nal Type
#define NAL_TYPE_RESERVED              0
#define NAL_TYPE_SLICE_NO_PART         1
#define NAL_TYPE_DATA_PART_A           2
#define NAL_TYPE_DATA_PART_B           3
#define NAL_TYPE_DATA_PART_C           4
#define NAL_TYPE_SLICE_IDR             5
#define NAL_TYPE_SEI_MESSAGE           6
#define NAL_TYPE_SEQ_PARAM_SET         7
#define NAL_TYPE_PIC_PARAM_SET         8
#define NAL_TYPE_ACCESS_UNIT_DELIM     9
#define NAL_TYPE_END_OF_SEQ            10
#define NAL_TYPE_END_OF_STREAM         11


////////////////////////////////////////////////////////
// class CMmpH264SPS
class CMmpH264SPS
{
public:
   unsigned int seq_parameter_set_id;

   unsigned char  profile_idc;
   unsigned char  level_idc;
   
   unsigned char  constraint_set012_flags;
   
   unsigned short log2_max_frame_num_minus4; // 0-12 inclusive
   unsigned int   MaxFrameNum;               // = 2^(log2_max_frame_num_minus4+4)

   unsigned char  pic_order_cnt_type;
   
   // If pic_order_cnt_type == 0
   unsigned short log_max_pic_order_cnt_lsb_minus4; // 0-12 inclusive
   unsigned int   MaxPicOrderCntLsb;                // = 2^(log_max_pic_order_cnt_lsb_minus4+4)
   
   // If pic_order_cnt_type == 1
   unsigned char  delta_pic_order_always_zero_flag;
   int            offset_for_non_ref_pic;
   int            offset_for_top_to_bottom_field;
   unsigned char  num_ref_frames_in_pic_order_cnt_cycle;
   int            offset_for_ref_frame[256];
   
   unsigned char  num_ref_frames;                   // 0-16 inclusive
   
   unsigned char  gaps_in_frame_num_value_allowed_flag;
   
   unsigned int   pic_width_in_mbs_minus_1;
   unsigned int   pic_height_in_map_units_minus1;
  
   unsigned char  frame_msb_only_flag;             // Always 1 for Baseline
   
   unsigned char  mb_adaptive_frame_field_flag;
   
   unsigned char  direct_8x8_inference_flag;
   
   unsigned char  frame_cropping_flag;
   unsigned int   frame_crop_left_offset;
   unsigned int   frame_crop_right_offset;
   unsigned int   frame_crop_top_offset;
   unsigned int   frame_crop_bottom_offset;
    
   // non Standard, 
   unsigned int   mStatus;

   unsigned int chroma_format_idc;
   unsigned char separate_colour_plane_flag;
   unsigned int bit_depth_luma_minus8;
   unsigned int bit_depth_chroma_minus8;
   unsigned int img_lossless_qpprime_fla;
   unsigned int seq_scaling_matrix_present_flag;

public :
    int pic_width;
    int pic_height;
    int mb_row_count;
    int mb_col_count;
    int mb_count;

    void AnalyzeInfo();

};

//////////////////////////////////////////////
// class CMmpMpeg4Parser
class CMmpH264Parser
{
public:
    static MMP_RESULT __MmpApiCall Seq_parameter_set_rbsp(CMmpBitExtractor* pBE, CMmpH264SPS* pSPS);
    static MMP_RESULT __MmpApiCall Seq_parameter_set_data(CMmpBitExtractor* pBE, CMmpH264SPS* pSPS);
    static MMP_RESULT __MmpApiCall rbsp_trailing_bits(CMmpBitExtractor* pBE, CMmpH264SPS* pSPS);
    
    static MMP_RESULT __MmpApiCall GetNextNalInfo(unsigned char* stream, int streamSize,  //IN
                                                  int* nalStartCodeIndex, //OUT,  start code index 
                                                  int* nalIndex,          //OUT
                                                  unsigned char* nal,     //OUT
                                                  bool* bStandard,        //OUT
                                                  int* nextNalStartCodeIndex );       //OUT 

    static MMP_BOOL __MmpApiCall IsThisStreamHas(MMP_U8* stream, MMP_S32 streamSize, MMP_U8 nalType, MMP_S32* nalIndex, MMP_S32 *nalSize);
    static MMP_RESULT __MmpApiCall Remake_VideoDSI(unsigned int fourcc, unsigned char* dsi, int dsiSize, int* newDsiSize=NULL, int* spsIndex=NULL, int* spsSize=NULL, int* ppsIndex=NULL, int* ppsSize=NULL);
    static MMP_RESULT __MmpApiCall Remake_VideoDSI_H264(unsigned char* dsi, int dsiSize, int* newDsiSize, int* spsIndex, int* spsSize, int* ppsIndex, int* ppsSize);
    static MMP_RESULT __MmpApiCall Remake_VideoDSI_AVC1(unsigned char* dsi, int dsiSize, int* newDsiSize, int* spsIndex, int* spsSize, int* ppsIndex, int* ppsSize);
    static MMP_RESULT __MmpApiCall ConvertDSI_AVC1_To_H264(MMP_U8* avc_dsi, MMP_S32 avc_dsi_size, MMP_U8* h264_dsi, MMP_S32* h264_dsi_size, MMP_S32* is_avc, MMP_S32* nal_length_size);
    static MMP_RESULT __MmpApiCall ParsingSPS(unsigned char* spsStream, int spsSize, CMmpH264SPS* pSPS);
    static unsigned int __MmpApiCall CheckH264SPS(unsigned int fourcc, CMmpH264SPS* pSPS);
  
    static MMP_RESULT __MmpApiCall ConvertDavinciH264Foramt(unsigned char* pAu, int auSize);
    static MMP_RESULT __MmpApiCall ConvertDavinciDSIForamt(unsigned char* pAu, int auSize, int* dsiSize);
    static MMP_RESULT __MmpApiCall ConvertDavinciAuForamt(unsigned char* pAu, int auSize, unsigned int* readAuIndex, int* realAuSize);
    static MMP_RESULT __MmpApiCall ConvertAvc1ToH264(MMP_U8* pAvc1Au, MMP_S32 avc_size, MMP_S32 nal_length_size, MMP_U8* pH264Au, MMP_S32* h264_size, MMP_BOOL is_one_nal = MMP_FALSE);
    
    static MMP_RESULT __MmpApiCall Decode_NextNal_H264(CMmpBitExtractor* pBE, MMP_U32 maxcount, MMP_S32* prefix_size);
};
#endif

