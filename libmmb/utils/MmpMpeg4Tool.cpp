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

#include "MmpMpeg4Tool.hpp"


//#define VIDEO_OBJECT_LAYER_SC            0x00000120
//#define VIDEO_OBJECT_LAYER_SC_SHORT      0x20
//#define VIDEO_OBJECT_SC_SHORT            0x00
//#define USER_DATA_SC                     0x000001B2
//#define USER_DATA_SC_SHORT               0xB2
//#define VIDEO_OBJECT_SC                  0x00000100

//#/define GROUP_OF_VOP_SC                  0x000001B3
//#define VOP_SC                           0x000001B6
//#define VOL_SC                           0x00000120

//#define SHORT_VIDEO_START_MK             0x00000020
//#define SHORT_VIDEO_END_MK               0x0000003F

#define RESYNC_MARKER                          0x00000001
#define GOB_RESYNC_MARKER                      0x00000001


/*******************************************************************************
 * Mpeg4 Viusal Object Type
 ******************************************************************************/
#define VIDEO_ID 0x01
#define SP_ID    0x02
#define MESH_ID  0x03
#define FACE_ID  0x04

/*******************************************************************************
 * Mpeg4 AspectRatioInfo
 ******************************************************************************/
enum ASPECT_RATIO_INFO {
   SQUARE       = 0x1,
   AS_12_11     = 0x2,
   AS_10_11     = 0x3,
   AS_16_11     = 0x4,
   AS_40_33     = 0x5,
   EXTENDED_PAR = 0xF
};

/*******************************************************************************
 * Mpeg4 Video Object Layer Shape
 ******************************************************************************/
enum VIDEO_OBJECT_LAYER_SHAPE {
   VOLS_RECTANGULAR = 0x00,
   VOLS_BINARY      = 0x01,
   VOLS_BINARY_ONLY = 0x02,
   VOLS_GRAYSCALE   = 0x03
};

/*******************************************************************************
 * Mpeg4 Constants
 ******************************************************************************/
//Macroblock Modes
#define MB_INTER       (0)      /* not_coded, mcbpc, cbpy,         mvd           */
#define MB_INTER_Q     (1)      /* not_coded, mcbpc, cbpy, dquant, mvd           */
#define MB_INTER4V     (2)      /* not_coded, mcbpc, cbpy,         mvd, mvd(2-4) */
#define MB_INTRA       (3)      /*            mcbpc, cbpy                        */
#define MB_INTRA_Q     (4)      /*            mcbpc, cbpy, dquant                */
#define MB_INTER4V_Q   (5)

// B-VOPs
#define MB_DIRECT      (6)      /* mvdb */
#define MB_INTERPOLATE (7)      /* dbquant mvd(f) mvdb */
#define MB_BACKWARD    (8)      /* dbquant mvd(b) */
#define MB_FORWARD     (9)      /* dbquant mvd(f) */

// This is not in the Standard
// This is implementation specific
#define MB_NOTCODED    (99)

//Sprite Enable Codewords
#define SE_NOTUSED     (0)
#define SE_STATIC      (1)
#define SE_GMC         (2)


/*******************************************************************************
 * Mpeg4 VOP Coding Type
 ******************************************************************************/
enum VOPCODINGYPE {
   VCT_I = 0x00,
   VCT_P = 0x01,
   VCT_B = 0x02,
   VCT_S = 0x03
};

/*******************************************************************************
 * Mpeg4 Sprite TransmitMode
 ******************************************************************************/
enum SPRITETRAANSMITMODE {
   STM_STOP = 0x00,
   STM_PIECE = 0x01,
   STM_UPDATE = 0x02,
   STM_PAUSE = 0x03
};


////////////////////////////////////////////////////////////////////////
// class CMmpMpeg4VOL

CMmpMpeg4VOL::CMmpMpeg4VOL() 
{
    // Video Object Layer Flags
   video_object_layer_start_code = 0;
   random_accessible_vol         = 0;
   video_object_type_indication  = 0;

   video_object_layer_verid      = 1;   // hjson 0->1
   
   video_object_layer_priority   = 0;
   aspect_ratio_info             = 0;
   low_delay                     = 0;
   first_half_vbv_buffer_size    = 0; 
   latter_half_vbv_buffer_size   = 0;
   first_half_vbv_occupancy      = 0; 
   latter_half_vbv_occupancy     = 0;
   video_object_layer_shape      = 0;
   video_object_layer_shape_extension=0;
   vop_time_increment_resolution=0;
   fixed_vop_rate=0;
   fixed_vop_time_increment=0;
   interlaced=0;
   obmc_disable=0;
   sprite_enable=0;
   sprite_width=0;
   sprite_height=0;
   sprite_left_coordinate=0;
   sprite_top_coordinate=0;
   no_of_sprite_warping_points=0;
   sprite_warping_accuracy=0;
   sprite_brightness_change=0;
   low_latency_sprite_enable=0;
   sadct_disable=0;
   not_8_bit=0;
   quant_precision=0;
   bits_per_pixel=0;
   no_gray_quant_update=0;
   composition_method=0;
   linear_composition=0;

   video_object_layer_width=0;
   video_object_layer_height=0;
   par_width=0;
   par_height=0;
   chroma_format=0;
   latter_half_bit_rate=0; 
   first_half_bit_rate=0; 
   scalability=0;

   quant_type=0;
   load_intra_quant_mat=0;
   //intra_quant_mat[2][64]=0;
   load_nonintra_quant_mat=0;
    //m_nonintra_quant_mat[2][64]=0;
   load_intra_quant_mat_grayscale=0;
    //m_intra_quant_mat_grayscale[2][64]=0;
   load_nonintra_quant_mat_grayscale=0;
    //m_nonintra_quant_mat_grayscale[2][64]=0;
   quarter_sample=0;
   complexity_estimation_disable=0;
   resync_marker_disable=0;
   data_partitioned=0;
   reversible_vlc=0;
   newpred_enable=0;
   requested_upstream_message_type=0;
   newpred_segment_type=0;
   reduced_resolution_vop_enable=0;
   hierarchy_type=0;
   ref_layer_id=0;
   ref_layer_sampling_direc=0;
   hor_sampling_factor_n=0;
   hor_sampling_factor_m=0;
   vert_sampling_factor_n=0;
   vert_sampling_factor_m=0;
   enhancement_type=0;
   use_ref_shape=0;
   use_ref_texture=0;
   shape_hor_sampling_factor_n=0;
   shape_hor_sampling_factor_m=0;
   shape_vert_sampling_factor_n=0;
   shape_vert_sampling_factor_m=0;

    // Video complexity estimation flags
   estimation_method=0;
   shape_complexity_estimation_disable=0;
   opaque=0;
   transparent=0;
   intra_cae=0;
   inter_cae=0;
   no_update=0;
   upsampling=0;
   texture_complexity_estimation_set_1_disable=0;
   intra_blocks=0;
   inter_blocks=0;
   inter4v_blocks=0;
   not_coded_blocks=0;
   texture_complexity_estimation_set_2_disable=0;
   dct_coefs=0;
   dct_lines=0;
   vlc_symbols=0;
   vlc_bits=0;
   motion_compensation_complexity_disable=0;
   apm=0;
   npm=0;
   interpolate_mc_q=0;
   forw_back_mc_q=0;
   halfpel2=0;
   halfpel4=0;
   version2_complexity_estimation_disable=0;
   sadct=0;
   quarterpel=0;

   vop_width_mb = 0;
   vop_height_mb = 0;
   
  // mMAX_WIDTH_SUPPORTED  = MMP_MPEG4_RESTRICTED_WIDTH;
  // mMAX_HEIGHT_SUPPORTED = MMP_MPEG4_RESTRICTED_HEIGHT;
}

////////////////////////////////////////////////////////////////////////
// class CMmpMpeg4Parser

MMP_RESULT CMmpMpeg4Parser::Decode_VOL(CMmpBitExtractor* pBE, CMmpMpeg4VOL* pVOL)
{
   int status_report = 0;
   
   unsigned long sc                         = 0;
   unsigned char is_object_layer_identifier = 0;
   unsigned char vol_control_parameters     = 0;
   unsigned char vbv_parameters             = 0;
   unsigned char marker_bit                 = 0;
   
   if( NULL != pBE )
   {
      #ifdef GENERAL_DEBUG
         std::cout << "Decoding Video Object Layer ... " << std::endl;
      #endif // GENERAL_DEBUG
      
      pVOL->video_object_layer_start_code = pBE->Pop_BitCode(pVOL->video_object_layer_start_code,8);
         
      // The least significant 4 bits are the Layer ID
      pVOL->video_object_layer_start_code &= 0x000000F0;
      if (pVOL->video_object_layer_start_code == MPEG4_VIDEO_SC_VIDEO_OBJECT_LAYER)
      {

         pVOL->random_accessible_vol        = (unsigned char)pBE->Pop_BitCode(sc, 1);
         pVOL->video_object_type_indication = (unsigned char)pBE->Pop_BitCode(sc, 8);
         
         is_object_layer_identifier = (unsigned char)pBE->Pop_BitCode(sc, 1);
         if (is_object_layer_identifier) 
         {
            pVOL->video_object_layer_verid    = (unsigned char)pBE->Pop_BitCode(sc, 4);
            pVOL->video_object_layer_priority =(unsigned char) pBE->Pop_BitCode(sc, 3);
         }
         
         pVOL->aspect_ratio_info = (unsigned char)pBE->Pop_BitCode(sc, 4);
         if (pVOL->aspect_ratio_info == (unsigned char) EXTENDED_PAR) 
         {
            pVOL->par_width  =(unsigned char) pBE->Pop_BitCode(sc, 8);
            pVOL->par_height =(unsigned char) pBE->Pop_BitCode(sc, 8);        
         }
         
         vol_control_parameters =(unsigned char) pBE->Pop_BitCode(sc, 1);
         if (vol_control_parameters) 
         {
            pVOL->chroma_format  =(unsigned char) pBE->Pop_BitCode(sc, 2);
            pVOL->low_delay      = (unsigned char)pBE->Pop_BitCode(sc, 1);
            
            vbv_parameters =(unsigned char) pBE->Pop_BitCode(sc, 1);
            if (vbv_parameters)
            {
               pVOL->first_half_bit_rate         = (unsigned short)pBE->Pop_BitCode(sc, 15);
               marker_bit                  = (unsigned char)pBE->Pop_BitCode(sc, 1 );
               pVOL->latter_half_bit_rate        = (unsigned short)pBE->Pop_BitCode(sc, 15);
               marker_bit                  =(unsigned char) pBE->Pop_BitCode(sc, 1 );
               pVOL->first_half_vbv_buffer_size  =(unsigned short) pBE->Pop_BitCode(sc, 15);
               marker_bit                  = (unsigned char)pBE->Pop_BitCode(sc, 1 );
               pVOL->latter_half_vbv_buffer_size = (unsigned short)pBE->Pop_BitCode(sc, 3 );
               pVOL->first_half_vbv_occupancy    = (unsigned short)pBE->Pop_BitCode(sc, 11);
               marker_bit                  = (unsigned char)pBE->Pop_BitCode(sc, 1 );
               pVOL->latter_half_vbv_occupancy   = (unsigned short)pBE->Pop_BitCode(sc, 15);
               marker_bit                  = (unsigned char)pBE->Pop_BitCode(sc, 1 );
            }
         }
         
         pVOL->video_object_layer_shape = (unsigned char)pBE->Pop_BitCode(sc, 2);
         if ((pVOL->video_object_layer_shape == 0x03) && (pVOL->video_object_layer_verid != 0x01))
         {
            pVOL->video_object_layer_shape_extension = (unsigned char)pBE->Pop_BitCode(sc, 4);
            return MMP_FAILURE;//MMP_ERROR_MPEG4DEC_DEC_UNSUPPORTED_CONTENT;
         }
         
         marker_bit =(unsigned char) pBE->Pop_BitCode(sc, 1);
         pVOL->vop_time_increment_resolution = (unsigned short)pBE->Pop_BitCode(sc, 16);
         marker_bit = (unsigned char)pBE->Pop_BitCode(sc, 1);
         
         pVOL->fixed_vop_rate =(unsigned char) pBE->Pop_BitCode(sc, 1);
         if (pVOL->fixed_vop_rate) 
         {
            unsigned short voptimeincres = pVOL->vop_time_increment_resolution-1;
            int time_inc_codeword = 0;
            do
            {
               voptimeincres = voptimeincres >> 1;
               time_inc_codeword++;
            } while(voptimeincres !=0x00);
            pVOL->fixed_vop_time_increment = (unsigned short)pBE->Pop_BitCode(sc,time_inc_codeword);
         }
         
         if (pVOL->video_object_layer_shape != VOLS_BINARY_ONLY)
         {
            if (pVOL->video_object_layer_shape == VOLS_RECTANGULAR)
            {
               marker_bit                =(unsigned char) pBE->Pop_BitCode(sc, 1 );
               pVOL->video_object_layer_width  =(unsigned short) pBE->Pop_BitCode(sc, 13);
               marker_bit                = (unsigned char)pBE->Pop_BitCode(sc, 1 );
               pVOL->video_object_layer_height = (unsigned short)pBE->Pop_BitCode(sc, 13);
               marker_bit                = (unsigned char)pBE->Pop_BitCode(sc, 1 );

           //#if 1
           //    pVOL->video_object_layer_width  &= 0xFFFFFFF0;
           //    pVOL->video_object_layer_height &= 0xFFFFFFF0;
           //#endif
               
           #if 0
               pVOL->vop_width_mb  = (pVOL->video_object_layer_width+15)  >> 4;     // vop_width_mb rounded up to nearest MB (see 6.3.3)
               pVOL->vop_height_mb = (pVOL->video_object_layer_height+15) >> 4;     // vop_height_mb rounded up to nearest MB (see 6.3.3)
               
               if ((pVOL->video_object_layer_width > pVOL->mMAX_WIDTH_SUPPORTED) || (pVOL->video_object_layer_height > pVOL->mMAX_HEIGHT_SUPPORTED))
               {
                  return DEC_IMAGE_SIZE_NOT_SUPPORTED;
               }
           #endif
            } 
            else
            {
              //non-rectangular shapes are not supported
                return MMP_FAILURE;//MMP_ERROR_MPEG4DEC_DEC_UNSUPPORTED_CONTENT;
            }
            
            pVOL->interlaced   = (unsigned char)pBE->Pop_BitCode(sc, 1);
            
            pVOL->obmc_disable = (unsigned char)pBE->Pop_BitCode(sc, 1);
            
            // add a workaround for DivX 4 Builds 167 to 208, ref.: fs_divx_cesdk_v4_0_videodecoder.pdf, DivX 4 Release Notes, Section 5: VOL Header Fields
            if (pVOL->vop_time_increment_resolution == 0 || pVOL->vop_time_increment_resolution == 1)
            {
               if(pVOL->obmc_disable == 0)
                  pVOL->obmc_disable = 1;
               else
                  pVOL->obmc_disable = 0;    
            }
            
            if (pVOL->video_object_layer_verid == 0x01)
            {
               pVOL->sprite_enable = (unsigned char)pBE->Pop_BitCode(sc, 1);
            }
            else
            {
               pVOL->sprite_enable = (unsigned char)pBE->Pop_BitCode(sc, 2);
            }
            
            if ((pVOL->sprite_enable == SE_STATIC) || (pVOL->sprite_enable == SE_GMC)) 
            {
               if (pVOL->sprite_enable != SE_GMC)
               {
                  pVOL->sprite_width           = (unsigned short)pBE->Pop_BitCode(sc, 13);
                  marker_bit             = (unsigned char)pBE->Pop_BitCode(sc, 1 );
                  pVOL->sprite_height          = (unsigned short)pBE->Pop_BitCode(sc, 13);
                  marker_bit             = (unsigned char)pBE->Pop_BitCode(sc, 1 );
                  pVOL->sprite_left_coordinate = (unsigned short)pBE->Pop_BitCode(sc, 13);
                  marker_bit             = (unsigned char)pBE->Pop_BitCode(sc, 1 );
                  pVOL->sprite_top_coordinate  = (unsigned short)pBE->Pop_BitCode(sc, 13);
                  marker_bit             = (unsigned char)pBE->Pop_BitCode(sc, 1 );
               }
               pVOL->no_of_sprite_warping_points =(unsigned char) pBE->Pop_BitCode(sc, 6);
               pVOL->sprite_warping_accuracy     = (unsigned char)pBE->Pop_BitCode(sc, 2);
               pVOL->sprite_brightness_change    = (unsigned char)pBE->Pop_BitCode(sc, 1);
               if (pVOL->sprite_enable != SE_GMC)
               {
                  pVOL->low_latency_sprite_enable = (unsigned char)pBE->Pop_BitCode(sc, 1);
               }
               
               //sprites are unsupported
               return MMP_FAILURE;//MMP_ERROR_MPEG4DEC_DEC_UNSUPPORTED_CONTENT; 
            }
            
            if ((pVOL->video_object_layer_verid != 0x01) && (pVOL->video_object_layer_shape != VOLS_RECTANGULAR))
            {
               pVOL->sadct_disable = (unsigned char)pBE->Pop_BitCode(sc, 1);
               return MMP_FAILURE;//MMP_ERROR_MPEG4DEC_DEC_UNSUPPORTED_CONTENT;
            }
            
            pVOL->not_8_bit = (unsigned char)pBE->Pop_BitCode(sc, 1);
            if (pVOL->not_8_bit) 
            {
               pVOL->quant_precision = (unsigned char)pBE->Pop_BitCode(sc, 4);
               pVOL->bits_per_pixel  =(unsigned char) pBE->Pop_BitCode(sc, 4);
            }
            else 
            {
               pVOL->quant_precision = 5;
               pVOL->bits_per_pixel  = 8;
            }
            
            if (pVOL->video_object_layer_shape == VOLS_GRAYSCALE) 
            {
               pVOL->no_gray_quant_update =(unsigned char) pBE->Pop_BitCode(sc, 1);
               pVOL->composition_method   = (unsigned char)pBE->Pop_BitCode(sc, 1);
               pVOL->linear_composition   =(unsigned char) pBE->Pop_BitCode(sc, 1);
            }
            
            pVOL->quant_type =(unsigned char) pBE->Pop_BitCode(sc, 1);
            if (pVOL->quant_type) 
            {
               // Intra 
               pVOL->load_intra_quant_mat = (unsigned char)pBE->Pop_BitCode(sc, 1);
               if (pVOL->load_intra_quant_mat) 
               {
                  int i, j;
                  for (i = 0; i < 2; i++) 
                  {
                     for (j = 0; j < 64; j++) 
                     {
                        pVOL->intra_quant_mat[i][j] = (unsigned char)pBE->Pop_BitCode(sc, 1);
                     }
                  }
               }
               
               // Non Intra 
               pVOL->load_nonintra_quant_mat = (unsigned char)pBE->Pop_BitCode(sc, 1);
               if (pVOL->load_nonintra_quant_mat) 
               {
                  int i, j;
                  for (i = 0; i < 2; i++) 
                  {
                     for (j = 0; j < 64; j++) 
                     {
                        pVOL->nonintra_quant_mat[i][j] =(unsigned char) pBE->Pop_BitCode(sc, 1);
                     }
                  }
               }
               
               if (pVOL->video_object_layer_shape == VOLS_GRAYSCALE) 
               {
                  int aux_comp_count = 1; //number of auxillary components
                  for (int i=0; i<aux_comp_count; i++)
                  {
                     // Intra 
                     pVOL->load_intra_quant_mat_grayscale =(unsigned char) pBE->Pop_BitCode(sc, 1);
                     if (pVOL->load_intra_quant_mat_grayscale) 
                     {
                        int i,j;
                        for (i = 0; i < 2; i++) 
                        {
                           for (j = 0; j < 64; j++) 
                           {
                              pVOL->intra_quant_mat_grayscale[i][j] = (unsigned char)pBE->Pop_BitCode(sc, 1);
                           }
                        }
                     }
                     
                     // Non Intra 
                     pVOL->load_nonintra_quant_mat_grayscale =(unsigned char) pBE->Pop_BitCode(sc, 1);
                     if (pVOL->load_nonintra_quant_mat_grayscale) 
                     {
                        int i,j;
                        for (i = 0; i < 2; i++) 
                        {
                           for (j = 0; j < 64; j++) 
                           {
                              pVOL->nonintra_quant_mat_grayscale[i][j] = (unsigned char)pBE->Pop_BitCode(sc, 1);
                           }
                        }
                     }
                  } //end for aux_comp_count
               } //end if video_object_layer_shape == VOLS_GRAYSCALE
            } //end if quant_type
            
            if (pVOL->video_object_layer_verid != 0x01)
            {
               pVOL->quarter_sample =(unsigned char) pBE->Pop_BitCode(sc, 1);
            }
            
            pVOL->complexity_estimation_disable = (unsigned char)pBE->Pop_BitCode(sc, 1);
            if (!pVOL->complexity_estimation_disable)
            {
                CMmpMpeg4Parser::Decode_define_vop_complexity_estimation_header(pBE, pVOL);
               //return DEC_UNSUPPORTED_CONTENT;
            }
            
            pVOL->resync_marker_disable = (unsigned char)pBE->Pop_BitCode(sc, 1);
            pVOL->data_partitioned      = (unsigned char)pBE->Pop_BitCode(sc, 1);
            
            if (pVOL->data_partitioned) 
            {
               pVOL->reversible_vlc = (unsigned char)pBE->Pop_BitCode(sc, 1);          
            }
            
            if (pVOL->video_object_layer_verid != 0x01)
            {
               pVOL->newpred_enable =(unsigned char) pBE->Pop_BitCode(sc, 1);
               if (pVOL->newpred_enable)
               {
                  pVOL->requested_upstream_message_type = (unsigned char)pBE->Pop_BitCode(sc, 2);
                  pVOL->newpred_segment_type            =(unsigned char) pBE->Pop_BitCode(sc, 1);
               }
               pVOL->reduced_resolution_vop_enable =(unsigned char) pBE->Pop_BitCode(sc, 1);
            }
            
            pVOL->scalability = (unsigned char)pBE->Pop_BitCode(sc, 1);
            if (pVOL->scalability)
            {
               pVOL->hierarchy_type =(unsigned char) pBE->Pop_BitCode(sc, 1);
               if (pVOL->hierarchy_type == 0x00)
               {
                  //spatial scalability is unsupported
                   return MMP_FAILURE;//MMP_ERROR_MPEG4DEC_DEC_UNSUPPORTED_CONTENT;
               }
               
               pVOL->ref_layer_id             = (unsigned char)pBE->Pop_BitCode(sc, 4);            
               pVOL->ref_layer_sampling_direc = (unsigned char)pBE->Pop_BitCode(sc, 1);
               pVOL->hor_sampling_factor_n    =(unsigned char) pBE->Pop_BitCode(sc, 5);
               pVOL->hor_sampling_factor_m    = (unsigned char)pBE->Pop_BitCode(sc, 5);
               pVOL->vert_sampling_factor_n   = (unsigned char)pBE->Pop_BitCode(sc, 5);
               pVOL->vert_sampling_factor_m   = (unsigned char)pBE->Pop_BitCode(sc, 5);
               pVOL->enhancement_type         = (unsigned char)pBE->Pop_BitCode(sc, 1);
               if ((pVOL->video_object_layer_shape == VOLS_BINARY) && (pVOL->hierarchy_type == 0))
               {
                  pVOL->use_ref_shape                =(unsigned char) pBE->Pop_BitCode(sc, 1);
                  pVOL->use_ref_texture              =(unsigned char) pBE->Pop_BitCode(sc, 1);
                  pVOL->shape_hor_sampling_factor_n  = (unsigned char)pBE->Pop_BitCode(sc, 5);
                  pVOL->shape_hor_sampling_factor_m  = (unsigned char)pBE->Pop_BitCode(sc, 5);
                  pVOL->shape_vert_sampling_factor_n = (unsigned char)pBE->Pop_BitCode(sc, 5);
                  pVOL->shape_vert_sampling_factor_m = (unsigned char)pBE->Pop_BitCode(sc, 5);
                  return MMP_FAILURE;//MMP_ERROR_MPEG4DEC_DEC_UNSUPPORTED_CONTENT;
               }
            } //end if scalability
         } //end if video_object_layer_shape != VOLS_BINARY_ONLY
         else 
         {
            //video_object_layer_shape == VOLS_BINARY_ONLY!! Currently unsupported.
            if (pVOL->video_object_layer_verid != 0x01)
            {
               pVOL->scalability = (unsigned char)pBE->Pop_BitCode(sc, 1);
               if (pVOL->scalability)
               {
                  pVOL->shape_hor_sampling_factor_n  =(unsigned char) pBE->Pop_BitCode(sc, 5);
                  pVOL->shape_hor_sampling_factor_m  =(unsigned char) pBE->Pop_BitCode(sc, 5);
                  pVOL->shape_vert_sampling_factor_n =(unsigned char) pBE->Pop_BitCode(sc, 5);
                  pVOL->shape_vert_sampling_factor_m =(unsigned char) pBE->Pop_BitCode(sc, 5);
               }
            }
          
            pVOL->resync_marker_disable = (unsigned char)pBE->Pop_BitCode(sc, 1);
            
            return MMP_FAILURE;//MMP_ERROR_MPEG4DEC_DEC_UNSUPPORTED_CONTENT;
         }
         
         pBE->NextStartCode();
   
         while(pBE->IsNextBits(MPEG4_VIDEO_SC_USER_DATA_LONG , 32))
             CMmpMpeg4Parser::Decode_userdata(pBE);
                        
      } //end if video_object_layer_start_code == VIDEO_OBJECT_LAYER_SC_SHORT
      else
      {
      }
      
      return (MMP_RESULT)status_report;
   }
   else 
   {
      return MMP_FAILURE;
   } // End if, else.
}

MMP_RESULT CMmpMpeg4Parser::Decode_define_vop_complexity_estimation_header(CMmpBitExtractor* pBE, CMmpMpeg4VOL* pVOL)
{
   unsigned long sc;
   if (pBE != NULL) 
   {
      pVOL->estimation_method = (unsigned char)pBE->Pop_BitCode(sc, 2);
      if (pVOL->estimation_method == 0x00 || pVOL->estimation_method == 0x01) 
      {
         pVOL->shape_complexity_estimation_disable =(unsigned char) pBE->Pop_BitCode(sc, 1);
         if ( !pVOL->shape_complexity_estimation_disable)
         {
            pVOL->opaque = (unsigned char)pBE->Pop_BitCode(sc, 1);
            pVOL->transparent = (unsigned char)pBE->Pop_BitCode(sc, 1);
            pVOL->intra_cae =(unsigned char) pBE->Pop_BitCode(sc, 1);
            pVOL->inter_cae = (unsigned char)pBE->Pop_BitCode(sc, 1);
            pVOL->no_update = (unsigned char)pBE->Pop_BitCode(sc, 1);
            pVOL->upsampling = (unsigned char)pBE->Pop_BitCode(sc, 1);
         }
         pVOL->texture_complexity_estimation_set_1_disable = (unsigned char)pBE->Pop_BitCode(sc, 1);
         if (!pVOL->texture_complexity_estimation_set_1_disable)
         {
            pVOL->intra_blocks =(unsigned char) pBE->Pop_BitCode(sc, 1);
            pVOL->inter_blocks = (unsigned char)pBE->Pop_BitCode(sc, 1);
            pVOL->inter4v_blocks =(unsigned char) pBE->Pop_BitCode(sc, 1);
            pVOL->not_coded_blocks = (unsigned char)pBE->Pop_BitCode(sc, 1);
         }
         pBE->Pop_BitCode(sc, 1);
         pVOL->texture_complexity_estimation_set_2_disable =(unsigned char) pBE->Pop_BitCode(sc, 1);
         if (!pVOL->texture_complexity_estimation_set_2_disable)
         {
            pVOL->dct_coefs = (unsigned char)pBE->Pop_BitCode(sc, 1);
            pVOL->dct_lines = (unsigned char)pBE->Pop_BitCode(sc, 1);
            pVOL->vlc_symbols = (unsigned char)pBE->Pop_BitCode(sc, 1);
            pVOL->vlc_bits =(unsigned char) pBE->Pop_BitCode(sc, 1);
         }
         pVOL->motion_compensation_complexity_disable = (unsigned char)pBE->Pop_BitCode(sc, 1);
         if (!pVOL->motion_compensation_complexity_disable)
         {
            pVOL->apm =(unsigned char) pBE->Pop_BitCode(sc, 1);
            pVOL->npm =(unsigned char) pBE->Pop_BitCode(sc, 1);
            pVOL->interpolate_mc_q = (unsigned char)pBE->Pop_BitCode(sc, 1);
            pVOL->forw_back_mc_q = (unsigned char)pBE->Pop_BitCode(sc, 1);
            pVOL->halfpel2 =(unsigned char) pBE->Pop_BitCode(sc, 1);
            pVOL->halfpel4 = (unsigned char)pBE->Pop_BitCode(sc, 1);
         }
         pBE->Pop_BitCode(sc, 1);
         if(pVOL->estimation_method == 0x01)
         {
            pVOL->version2_complexity_estimation_disable = (unsigned char)pBE->Pop_BitCode(sc, 1);
            if(!pVOL->version2_complexity_estimation_disable)
            {
               pVOL->sadct =(unsigned char) pBE->Pop_BitCode(sc, 1);
               pVOL->quarterpel = (unsigned char)pBE->Pop_BitCode(sc, 1);
            }      
         }
      } // End if.
      return MMP_SUCCESS;
   }
   else 
   {
      return MMP_FAILURE;
   } // End if, else.
} // End of the Mpeg4VideoDecoder::define_vop_complexity_estimation_header method.


MMP_RESULT CMmpMpeg4Parser::Decode_userdata(CMmpBitExtractor* pBE)
{
   unsigned long dummy;
   
   unsigned long user_data_start_code = 
         pBE->Pop_BitCode(user_data_start_code,32);
         
   if (user_data_start_code == MPEG4_VIDEO_SC_USER_DATA_LONG)
   {
      //The correct check for this while loop is to look for the 
      //24 bit code '0000 0000 0000 0000 0000 0001' (start code prefix).
      //However, it will not occur since the DSI is standalone and the DSI
      //bitstream does not contain the start code for the next AU.
      while (!pBE->IsNextBits(0x0, 23))
         pBE->Pop_BitCode(dummy,8);
   }
   else
   {
      return MMP_FAILURE;//MMP_ERROR_MPEG4DEC_DEC_BITSTREAM_ERROR;
   }       
   
   return MMP_SUCCESS; 
}

MMP_RESULT CMmpMpeg4Parser::Decode_NextStartCodePrefix(CMmpBitExtractor* pBE, MMP_U32 maxcount)
{
   unsigned long dummy;
   unsigned int codes[3];
   int count;

   // Locating the byte-alligned start code prefix 0x000001
   // -----------------------------------------------------
   codes[0] = (unsigned int)pBE->Pop_BitCode(dummy, 8);
   codes[1] = (unsigned int)pBE->Pop_BitCode(dummy, 8);
   codes[2] = (unsigned int)pBE->Pop_BitCode(dummy, 8);
   
   count=0;
   while (!((codes[0] == 0x00) && (codes[1] == 0x00) && (codes[2] == 0x01)))
   {
      //if (codes[0] == 0xAB && codes[1] == 0xCD && codes[2] == 0xEF )
      //   return MMP_FAILURE;

      codes[0] = codes[1];
      codes[1] = codes[2];
      codes[2] = (unsigned int)pBE->Pop_BitCode(dummy, 8);

      count++;
      if( count>(int)maxcount) return MMP_FAILURE;
      if( !pBE->CanGetBit(8) ) return MMP_FAILURE;
   }
   
   return MMP_SUCCESS;
}

MMP_S32 CMmpMpeg4Parser::GetFirstVOPIndex(MMP_U8* stream, MMP_S32 stream_size) {

    CMmpBitExtractor be;
    MMP_U32 vop_index = -1,  maxcount;
    unsigned long d;
    MMP_RESULT mmpResult;

    if( (stream != NULL) && (stream_size>0) ) {

        be.Start(stream, stream_size);
        maxcount = stream_size;
        while(1) {
    
            maxcount -= be.GetCurByteIndex();

            mmpResult = CMmpMpeg4Parser::Decode_NextStartCodePrefix(&be, maxcount);
            if(mmpResult != MMP_SUCCESS) {
                break;
            }

            be.Pop_BitCode(d, 8);
            d&=0xFF;

            if(d == MPEG4_VIDEO_VOP) {

                vop_index = be.GetCurByteIndex();
                vop_index -= (3 + 1); // 00 00 01 B6
                break;
            }
        }

    }

    
    return vop_index;
}

