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

#ifndef _MMPMPEG4TOOL_HPP__
#define _MMPMPEG4TOOL_HPP__

#include "MmpBitExtractor.hpp"

/*******************************************************************************
 * Mpeg4 Start Code
 ******************************************************************************/
#define MPEG4_VIDEO_SC_VISUAL_OBJECT_SEQUENCE  (0xB0)
#define MPEG4_VIDEO_SC_VISUAL_OBJECT           (0xB5)
#define MPEG4_VIDEO_SC_VIDEO_OBJECT_LAYER      (0x20) //0x20~0x2F
#define MPEG4_VIDEO_SC_VIDEO_OBJECT            (0x00)
#define MPEG4_VIDEO_SC_USER_DATA               (0xB2)
#define MPEG4_VIDEO_SC_USER_DATA_LONG          (0x000001B2)     /* 0x00000100 (24-bit start code) + 0xB2 () */

#define MPEG4_VIDEO_GROUP_OF_VOP               (0xB3)
#define MPEG4_VIDEO_VOP                        (0xB6)


////////////////////////////////////////////////////////
// class CMmpMpeg4VOL
class CMmpMpeg4VOL
{
public:
    // Video Object Layer Flags
    unsigned long video_object_layer_start_code;
    unsigned char random_accessible_vol;
    unsigned char video_object_type_indication;
    unsigned char video_object_layer_verid;
    unsigned char video_object_layer_priority;
    unsigned char aspect_ratio_info;
    unsigned char low_delay;
    unsigned short first_half_vbv_buffer_size; 
    unsigned short latter_half_vbv_buffer_size;
    unsigned short first_half_vbv_occupancy; 
    unsigned short latter_half_vbv_occupancy;
    unsigned char  video_object_layer_shape;
    unsigned char  video_object_layer_shape_extension;
    unsigned short vop_time_increment_resolution;
    unsigned short fixed_vop_time_increment;
    unsigned char  fixed_vop_rate;
    unsigned char interlaced;
    unsigned char obmc_disable;
    unsigned char sprite_enable;
    unsigned short sprite_width;
    unsigned short sprite_height;
    unsigned short sprite_left_coordinate;
    unsigned short sprite_top_coordinate;
    unsigned char no_of_sprite_warping_points;
    unsigned char sprite_warping_accuracy;
    unsigned char sprite_brightness_change;
    unsigned char low_latency_sprite_enable;
    unsigned char sadct_disable;
    unsigned char not_8_bit;
    unsigned char quant_precision;
    unsigned char bits_per_pixel;
    unsigned char no_gray_quant_update;
    unsigned char composition_method;
    unsigned char linear_composition;

    //data members that outside entities might be interested in
    unsigned short video_object_layer_width;
    unsigned short video_object_layer_height;
    unsigned char par_width;
    unsigned char par_height;
    unsigned char chroma_format;
    unsigned short latter_half_bit_rate; 
    unsigned short first_half_bit_rate; 
    unsigned char scalability;

    unsigned char quant_type;
    unsigned char load_intra_quant_mat;
    unsigned char intra_quant_mat[2][64];
    unsigned char load_nonintra_quant_mat;
    unsigned char nonintra_quant_mat[2][64];
    unsigned char load_intra_quant_mat_grayscale;
    unsigned char intra_quant_mat_grayscale[2][64];
    unsigned char load_nonintra_quant_mat_grayscale;
    unsigned char nonintra_quant_mat_grayscale[2][64];
    unsigned char quarter_sample;
    unsigned char complexity_estimation_disable;
    unsigned char resync_marker_disable;
    unsigned char data_partitioned;
    unsigned char reversible_vlc;
    unsigned char newpred_enable;
    unsigned char requested_upstream_message_type;
    unsigned char newpred_segment_type;
    unsigned char reduced_resolution_vop_enable;
    unsigned char hierarchy_type;
    unsigned char ref_layer_id;
    unsigned char ref_layer_sampling_direc;
    unsigned char hor_sampling_factor_n;
    unsigned char hor_sampling_factor_m;
    unsigned char vert_sampling_factor_n;
    unsigned char vert_sampling_factor_m;
    unsigned char enhancement_type;
    unsigned char use_ref_shape;
    unsigned char use_ref_texture;
    unsigned char shape_hor_sampling_factor_n;
    unsigned char shape_hor_sampling_factor_m;
    unsigned char shape_vert_sampling_factor_n;
    unsigned char shape_vert_sampling_factor_m;

    // Video complexity estimation flags
    unsigned char estimation_method;
    unsigned char shape_complexity_estimation_disable;
    unsigned char opaque;
    unsigned char transparent;
    unsigned char intra_cae;
    unsigned char inter_cae;
    unsigned char no_update;
    unsigned char upsampling;
    unsigned char texture_complexity_estimation_set_1_disable;
    unsigned char intra_blocks;
    unsigned char inter_blocks;
    unsigned char inter4v_blocks;
    unsigned char not_coded_blocks;
    unsigned char texture_complexity_estimation_set_2_disable;
    unsigned char dct_coefs;
    unsigned char dct_lines;
    unsigned char vlc_symbols;
    unsigned char vlc_bits;
    unsigned char motion_compensation_complexity_disable;
    unsigned char apm;
    unsigned char npm;
    unsigned char interpolate_mc_q;
    unsigned char forw_back_mc_q;
    unsigned char halfpel2;
    unsigned char halfpel4;
    unsigned char version2_complexity_estimation_disable;
    unsigned char sadct;
    unsigned char quarterpel;

    unsigned short vop_width_mb;
    unsigned short vop_height_mb;

    unsigned int   mMAX_WIDTH_SUPPORTED;
    unsigned int   mMAX_HEIGHT_SUPPORTED;

public:
    CMmpMpeg4VOL();
};

//////////////////////////////////////////////
// class CMmpMpeg4Parser
class CMmpMpeg4Parser
{
public:
    static MMP_RESULT __MmpApiCall Decode_VOL(CMmpBitExtractor* pBE, CMmpMpeg4VOL* pVOL);
    static MMP_RESULT __MmpApiCall Decode_define_vop_complexity_estimation_header(CMmpBitExtractor* pBE, CMmpMpeg4VOL* pVOL);
    static MMP_RESULT __MmpApiCall Decode_userdata(CMmpBitExtractor* pBE);
    static MMP_RESULT __MmpApiCall Decode_NextStartCodePrefix(CMmpBitExtractor* pBE, MMP_U32 maxcount);
    
    static MMP_S32 GetFirstVOPIndex(MMP_U8* stream, MMP_S32 stream_size);

};
#endif

