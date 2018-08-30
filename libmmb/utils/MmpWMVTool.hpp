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

#ifndef _MMPWMVTOOL_HPP__
#define _MMPWMVTOOL_HPP__

#include "MmpDefine.h"
#include "MmpBitExtractor.hpp"

/*
VC1 extra data
   size = 22
   data = 25 00 00 01 0f d3 7e 16 70 ef 88 80 00 00 01 0e 10 44 59 c3 bc 80
   "F:\\QA\\WMV\\wmv 024¡Ú¡ÚWVC1 + wmav9  ¡Ú 720¡¿480_7303kbps_23ÂÀ97F_48Khz_237kbps\\[«¢«Ë«á DVD] FLAG ð¯01ü¥ (720x480 WMV9AP QL93).wmv"

WMV3 extra data
   size = 4
   data = 4e 79 1a 01
   "C:\MediaSample\WMV\0912_HwangChaWon.wmv"

WMV2 extra data
   size = 4
   data = a0 ba fc 80
   "C:\MediaSample\WMV\GarField.wmv"

*/


enum MmpWMV3Profile {
    MMP_WMV3_PROFILE_UNKNOWN = -1, 
    MMP_WMV3_PROFILE_SIMPLE = 0,
    MMP_WMV3_PROFILE_MAIN,
    MMP_WMV3_PROFILE_COMPLEX, ///< TODO: WMV9 specific
    MMP_WMV3_PROFILE_ADVANCED
};

/** The VC1 Context
 * @todo Change size wherever another size is more efficient
 * Many members are only used for Advanced Profile
 */
struct MmpVC1_MpegEncContext {
    int loop_filter;
    int max_b_frames;
};

struct MmpVC1Context{
    MmpVC1_MpegEncContext s;
    //IntraX8Context x8;
    //H264ChromaContext h264chroma;
    //VC1DSPContext vc1dsp;

    int bits;

    /** Simple/Main Profile sequence header */
    //@{
    int res_sprite;       ///< reserved, sprite mode
    int res_y411;         ///< reserved, old interlaced mode
    int res_x8;           ///< reserved
    int multires;         ///< frame-level RESPIC syntax element present
    int res_fasttx;       ///< reserved, always 1
    int res_transtab;     ///< reserved, always 0
    int rangered;         ///< RANGEREDFRM (range reduction) syntax element present
                          ///< at frame level
    int res_rtm_flag;     ///< reserved, set to 1
    int reserved;         ///< reserved
    //@}

    /** Advanced Profile */
    //@{
    int level;            ///< 3 bits, for Advanced/Simple Profile, provided by TS layer
    int chromaformat;     ///< 2 bits, 2=4:2:0, only defined
    int postprocflag;     ///< Per-frame processing suggestion flag present
    int broadcast;        ///< TFF/RFF present
    int interlace;        ///< Progressive/interlaced (RPTFTM syntax element)
    int tfcntrflag;       ///< TFCNTR present
    int panscanflag;      ///< NUMPANSCANWIN, TOPLEFT{X,Y}, BOTRIGHT{X,Y} present
    int refdist_flag;     ///< REFDIST syntax element present in II, IP, PI or PP field picture headers
    int extended_dmv;     ///< Additional extended dmv range at P/B-frame-level
    int color_prim;       ///< 8 bits, chroma coordinates of the color primaries
    int transfer_char;    ///< 8 bits, Opto-electronic transfer characteristics
    int matrix_coef;      ///< 8 bits, Color primaries->YCbCr transform matrix
    int hrd_param_flag;   ///< Presence of Hypothetical Reference
                          ///< Decoder parameters
    int psf;              ///< Progressive Segmented Frame
    //@}

    /** Sequence header data for all Profiles
     * TODO: choose between ints, MMP_U8s and monobit flags
     */
    //@{
    int profile;          ///< 2 bits, Profile
    int frmrtq_postproc;  ///< 3 bits,
    int bitrtq_postproc;  ///< 5 bits, quantized framerate-based postprocessing strength
    int max_coded_width, max_coded_height;
    int fastuvmc;         ///< Rounding of qpel vector to hpel ? (not in Simple)
    int extended_mv;      ///< Ext MV in P/B (not in Simple)
    int dquant;           ///< How qscale varies with MBs, 2 bits (not in Simple)
    int vstransform;      ///< variable-size [48]x[48] transform type + info
    int overlap;          ///< overlapped transforms in use
    int quantizer_mode;   ///< 2 bits, quantizer mode used for sequence, see QUANT_*
    int finterpflag;      ///< INTERPFRM present
    //@}

    /** Frame decoding info for all profiles */
    //@{
    MMP_U8 mv_mode;             ///< MV coding mode
    MMP_U8 mv_mode2;            ///< Secondary MV coding mode (B-frames)
    int k_x;                     ///< Number of bits for MVs (depends on MV range)
    int k_y;                     ///< Number of bits for MVs (depends on MV range)
    int range_x, range_y;        ///< MV range
    MMP_U8 pq, altpq;           ///< Current/alternate frame quantizer scale
    MMP_U8 zz_8x8[4][64];       ///< Zigzag table for TT_8x8, permuted for IDCT
    int left_blk_sh, top_blk_sh; ///< Either 3 or 0, positions of l/t in blk[]
    const MMP_U8* zz_8x4;       ///< Zigzag scan table for TT_8x4 coding mode
    const MMP_U8* zz_4x8;       ///< Zigzag scan table for TT_4x8 coding mode
    /** pquant parameters */
    //@{
    MMP_U8 dquantfrm;
    MMP_U8 dqprofile;
    MMP_U8 dqsbedge;
    MMP_U8 dqbilevel;
    //@}
    /** AC coding set indexes
     * @see 8.1.1.10, p(1)10
     */
    //@{
    int c_ac_table_index;    ///< Chroma index from ACFRM element
    int y_ac_table_index;    ///< Luma index from AC2FRM element
    //@}
    int ttfrm;               ///< Transform type info present at frame level
    MMP_U8 ttmbf;           ///< Transform type flag
    int *ttblk_base, *ttblk; ///< Transform type at the block level
    int codingset;           ///< index of current table set from 11.8 to use for luma block decoding
    int codingset2;          ///< index of current table set from 11.8 to use for chroma block decoding
    int pqindex;             ///< raw pqindex used in coding set selection
    int a_avail, c_avail;
    MMP_U8 *mb_type_base, *mb_type[3];


    /** Luma compensation parameters */
    //@{
    MMP_U8 lumscale;
    MMP_U8 lumshift;
    //@}
    int16_t bfraction;    ///< Relative position % anchors=> how to scale MVs
    MMP_U8 halfpq;       ///< Uniform quant over image and qp+.5
    MMP_U8 respic;       ///< Frame-level flag for resized images
    int buffer_fullness;  ///< HRD info
    /** Ranges:
     * -# 0 -> [-64n 63.f] x [-32, 31.f]
     * -# 1 -> [-128, 127.f] x [-64, 63.f]
     * -# 2 -> [-512, 511.f] x [-128, 127.f]
     * -# 3 -> [-1024, 1023.f] x [-256, 255.f]
     */
    MMP_U8 mvrange;                ///< Extended MV range flag
    MMP_U8 pquantizer;             ///< Uniform (over sequence) quantizer in use
    //VLC *cbpcy_vlc;                 ///< CBPCY VLC table
    int tt_index;                   ///< Index for Transform Type tables (to decode TTMB)
    MMP_U8* mv_type_mb_plane;      ///< bitplane for mv_type == (4MV)
    MMP_U8* direct_mb_plane;       ///< bitplane for "direct" MBs
    MMP_U8* forward_mb_plane;      ///< bitplane for "forward" MBs
    int mv_type_is_raw;             ///< mv type mb plane is not coded
    int dmb_is_raw;                 ///< direct mb plane is raw
    int fmb_is_raw;                 ///< forward mb plane is raw
    int skip_is_raw;                ///< skip mb plane is not coded
    MMP_U8 last_luty[2][256], last_lutuv[2][256];  ///< lookup tables used for intensity compensation
    MMP_U8  aux_luty[2][256],  aux_lutuv[2][256];  ///< lookup tables used for intensity compensation
    MMP_U8 next_luty[2][256], next_lutuv[2][256];  ///< lookup tables used for intensity compensation
    MMP_U8 (*curr_luty)[256]  ,(*curr_lutuv)[256];
    int last_use_ic, *curr_use_ic, next_use_ic, aux_use_ic;
    int rnd;                        ///< rounding control

    /** Frame decoding info for S/M profiles only */
    //@{
    MMP_U8 rangeredfrm;            ///< out_sample = CLIP((in_sample-128)*2+128)
    MMP_U8 interpfrm;
    //@}

    /** Frame decoding info for Advanced profile */
    //@{
    //enum FrameCodingMode fcm;
    MMP_U8 numpanscanwin;
    MMP_U8 tfcntr;
    MMP_U8 rptfrm, tff, rff;
    uint16_t topleftx;
    uint16_t toplefty;
    uint16_t bottomrightx;
    uint16_t bottomrighty;
    MMP_U8 uvsamp;
    MMP_U8 postproc;
    int hrd_num_leaky_buckets;
    MMP_U8 bit_rate_exponent;
    MMP_U8 buffer_size_exponent;
    MMP_U8* acpred_plane;       ///< AC prediction flags bitplane
    int acpred_is_raw;
    MMP_U8* over_flags_plane;   ///< Overflags bitplane
    int overflg_is_raw;
    MMP_U8 condover;
    uint16_t *hrd_rate, *hrd_buffer;
    MMP_U8 *hrd_fullness;
    MMP_U8 range_mapy_flag;
    MMP_U8 range_mapuv_flag;
    MMP_U8 range_mapy;
    MMP_U8 range_mapuv;
    //@}

    /** Frame decoding info for interlaced picture */
    MMP_U8 dmvrange;   ///< Extended differential MV range flag
    int fourmvswitch;
    int intcomp;
    MMP_U8 lumscale2;  ///< for interlaced field P picture
    MMP_U8 lumshift2;
    //VLC* mbmode_vlc;
    //VLC* imv_vlc;
    //VLC* twomvbp_vlc;
    //VLC* fourmvbp_vlc;
    MMP_U8 twomvbp;
    MMP_U8 fourmvbp;
    MMP_U8* fieldtx_plane;
    int fieldtx_is_raw;
    MMP_U8 zzi_8x8[64];
    MMP_U8 *blk_mv_type_base, *blk_mv_type;    ///< 0: frame MV, 1: field MV (interlaced frame)
    MMP_U8 *mv_f_base, *mv_f[2];               ///< 0: MV obtained from same field, 1: opposite field
    MMP_U8 *mv_f_next_base, *mv_f_next[2];
    int field_mode;         ///< 1 for interlaced field pictures
    int fptype;
    int second_field;
    int refdist;            ///< distance of the current picture from reference
    int numref;             ///< number of past field pictures used as reference
                            // 0 corresponds to 1 and 1 corresponds to 2 references
    int reffield;           ///< if numref = 0 (1 reference) then reffield decides which
                            // field to use among the two fields from previous frame
    int intcompfield;       ///< which of the two fields to be intensity compensated
                            // 0: both fields, 1: bottom field, 2: top field
    int cur_field_type;     ///< 0: top, 1: bottom
    int ref_field_type[2];  ///< forward and backward reference field type (top or bottom)
    int blocks_off, mb_off;
    int qs_last;            ///< if qpel has been used in the previous (tr.) picture
    int bmvtype;
    int frfd, brfd;         ///< reference frame distance (forward or backward)
    int first_pic_header_flag;
    int pic_header_flag;

    /** Frame decoding info for sprite modes */
    //@{
    int new_sprite;
    int two_sprites;
    //AVFrame *sprite_output_frame;
    int output_width, output_height, sprite_width, sprite_height;
    MMP_U8* sr_rows[2][2];      ///< Sprite resizer line cache
    //@}

    int p_frame_skipped;
    int bi_type;
    int x8_type;

    int16_t (*block)[6][64];
    int n_allocated_blks, cur_blk_idx, left_blk_idx, topleft_blk_idx, top_blk_idx;
    uint32_t *cbp_base, *cbp;
    MMP_U8 *is_intra_base, *is_intra;
    int16_t (*luma_mv_base)[2], (*luma_mv)[2];
    MMP_U8 bfraction_lut_index; ///< Index for BFRACTION value (see Table 40, reproduced into ff_vc1_bfraction_lut[])
    MMP_U8 broken_link;         ///< Broken link flag (BROKEN_LINK syntax element)
    MMP_U8 closed_entry;        ///< Closed entry point flag (CLOSED_ENTRY syntax element)

    int end_mb_x;                ///< Horizontal macroblock limit (used only by mss2)

    int parse_only;              ///< Context is used within parser
    int resync_marker;           ///< could this stream contain resync markers
};

//////////////////////////////////////////////
// class CMmpMpeg4Parser
class CMmpWMVParser
{
public:
    static enum MMP_FOURCC get_fourcc_with_extradata(MMP_U8 *extra_data, MMP_S32 extra_data_size);
    static enum MMP_FOURCC get_fourcc_with_extradata(MMP_U8 *extra_data, MMP_S32 extra_data_size, MmpVC1Context* p_vc1ctx);
    static enum MmpWMV3Profile get_profile(MMP_U8 *extra_data, MMP_S32 extra_data_size); /* Check if WMV3, Compled_Profile */
    
};
#endif

