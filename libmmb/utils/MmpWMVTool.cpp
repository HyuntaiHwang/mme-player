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
#include "MmpWMVTool.hpp"
#include "MmpUtil.hpp"
#include "mmp_video_tool.hpp"

#define AV_INPUT_BUFFER_PADDING_SIZE 32

/** Markers used in VC-1 AP frame data */
//@{
enum VC1Code {
    VC1_CODE_RES0       = 0x00000100,
    VC1_CODE_ENDOFSEQ   = 0x0000010A,
    VC1_CODE_SLICE,
    VC1_CODE_FIELD,
    VC1_CODE_FRAME,
    VC1_CODE_ENTRYPOINT,
    VC1_CODE_SEQHDR,
};
//@}

#define IS_MARKER(x) (((x) & ~0xFF) == VC1_CODE_RES0)

/** Available Profiles */
//@{
enum Profile {
    PROFILE_SIMPLE,
    PROFILE_MAIN,
    PROFILE_COMPLEX, ///< TODO: WMV9 specific
    PROFILE_ADVANCED
};
//@}



static const MMP_U8* find_next_marker(const MMP_U8 *src, const MMP_U8 *end)
{
    if (end - src >= 4) {
        MMP_U32 mrk = 0xFFFFFFFF;
        src = mmp_video_tool::find_start_code(src, end, &mrk);
        if (IS_MARKER(mrk))
            return src - 4;
    }
    return end;
}

static int vc1_unescape_buffer(const MMP_U8 *src, int size, MMP_U8 *dst)
{
    int dsize = 0, i;

    if (size < 4) {
        for (dsize = 0; dsize < size; dsize++)
            *dst++ = *src++;
        return size;
    }
    for (i = 0; i < size; i++, src++) {
        if (src[0] == 3 && i >= 2 && !src[-1] && !src[-2] && i < size-1 && src[1] < 4) {
            dst[dsize++] = src[1];
            src++;
            i++;
        } else
            dst[dsize++] = *src;
    }
    return dsize;
}



static int decode_sequence_header_adv(MmpVC1Context *v, CMmpBitExtractor& be)
{
    unsigned long pt;

    v->res_rtm_flag = 1;
    v->level = be.Pop_BitCode(pt, 3); //get_bits(gb, 3);
    if (v->level >= 5) {
        //av_log(v->s.avctx, AV_LOG_ERROR, "Reserved LEVEL %i\n",v->level);
    }
    v->chromaformat = be.Pop_BitCode(pt, 2); //get_bits(gb, 2);
    if (v->chromaformat != 1) {
        //av_log(v->s.avctx, AV_LOG_ERROR,
         //      "Only 4:2:0 chroma format supported\n");
        return -1;
    }

    // (fps-2)/4 (->30)
    v->frmrtq_postproc       = be.Pop_BitCode(pt, 3); //get_bits(gb, 3); //common
    // (bitrate-32kbps)/64kbps
    v->bitrtq_postproc       = be.Pop_BitCode(pt, 5); //get_bits(gb, 5); //common
    v->postprocflag          = be.Pop_BitCode(pt, 1); //get_bits1(gb);   //common

    v->max_coded_width       = (be.Pop_Bit(12) + 1) << 1; //(get_bits(gb, 12) + 1) << 1;
    v->max_coded_height      = (be.Pop_Bit(12) + 1) << 1; //(get_bits(gb, 12) + 1) << 1;
    v->broadcast             = be.Pop_1Bit();//get_bits1(gb);
    v->interlace             = be.Pop_1Bit();//get_bits1(gb);
    v->tfcntrflag            = be.Pop_1Bit();//get_bits1(gb);
    v->finterpflag           = be.Pop_1Bit();//get_bits1(gb);
    be.Pop_1Bit();//skip_bits1(gb); // reserved

    //av_log(v->s.avctx, AV_LOG_DEBUG,
    //       "Advanced Profile level %i:\nfrmrtq_postproc=%i, bitrtq_postproc=%i\n"
    //       "LoopFilter=%i, ChromaFormat=%i, Pulldown=%i, Interlace: %i\n"
    //       "TFCTRflag=%i, FINTERPflag=%i\n",
    //       v->level, v->frmrtq_postproc, v->bitrtq_postproc,
    //       v->s.loop_filter, v->chromaformat, v->broadcast, v->interlace,
    //       v->tfcntrflag, v->finterpflag);

    v->psf = be.Pop_1Bit();//get_bits1(gb);
    if (v->psf) { //PsF, 6.1.13
        //av_log(v->s.avctx, AV_LOG_ERROR, "Progressive Segmented Frame mode: not supported (yet)\n");
        return -1;
    }
    //v->s.max_b_frames = v->s.avctx->max_b_frames = 7;
    if(be.Pop_1Bit() /*get_bits1(gb)*/) { //Display Info - decoding is not affected by it
        int w, h, ar = 0;
        //av_log(v->s.avctx, AV_LOG_DEBUG, "Display extended info:\n");
        w = be.Pop_BitCode(pt, 14) + 1; //get_bits(gb, 14) + 1;
        h = be.Pop_BitCode(pt, 14) + 1;  //get_bits(gb, 14) + 1;
        //av_log(v->s.avctx, AV_LOG_DEBUG, "Display dimensions: %ix%i\n", w, h);
        if (be.Pop_1Bit() /*get_bits1(gb)*/)
            ar = be.Pop_BitCode(pt, 4);//get_bits(gb, 4);
        if (ar && ar < 14) {
            //v->s.avctx->sample_aspect_ratio = ff_vc1_pixel_aspect[ar];
        } else if (ar == 15) {
            w = be.Pop_BitCode(pt, 8)+1; //get_bits(gb, 8) + 1;
            h = be.Pop_BitCode(pt, 8)+1; //get_bits(gb, 8) + 1;
            //v->s.avctx->sample_aspect_ratio = AVRational_GetObject(w,h); //(AVRational){w, h};
        } else {
            //av_reduce(&v->s.avctx->sample_aspect_ratio.num,
            //          &v->s.avctx->sample_aspect_ratio.den,
            //          v->s.avctx->height * w,
            //          v->s.avctx->width * h,
            //          1 << 30);
        }
        //ff_set_sar(v->s.avctx, v->s.avctx->sample_aspect_ratio);
        //av_log(v->s.avctx, AV_LOG_DEBUG, "Aspect: %i:%i\n",
        //       v->s.avctx->sample_aspect_ratio.num,
        //       v->s.avctx->sample_aspect_ratio.den);

        if (be.Pop_1Bit() /*get_bits1(gb)*/) { //framerate stuff
            if (be.Pop_1Bit() /*get_bits1(gb)*/) {
                //v->s.avctx->framerate.den = 32;
                be.Pop_BitCode(pt, 16); //v->s.avctx->framerate.num = get_bits(gb, 16) + 1;
            } else {
                int nr, dr;
                nr = be.Pop_BitCode(pt, 8);//get_bits(gb, 8);
                dr = be.Pop_BitCode(pt, 4); //get_bits(gb, 4);
                if (nr > 0 && nr < 8 && dr > 0 && dr < 3) {
                    //v->s.avctx->framerate.den = ff_vc1_fps_dr[dr - 1];
                    //v->s.avctx->framerate.num = ff_vc1_fps_nr[nr - 1] * 1000;
                }
            }
            //if (v->broadcast) { // Pulldown may be present
            //    v->s.avctx->ticks_per_frame = 2;
            //}
        }

        //if (get_bits1(gb)) {
        //    v->color_prim    = get_bits(gb, 8);
        //    v->transfer_char = get_bits(gb, 8);
        //    v->matrix_coef   = get_bits(gb, 8);
        //}
    }

    //v->hrd_param_flag = get_bits1(gb);
    //if (v->hrd_param_flag) {
    //    int i;
    //    v->hrd_num_leaky_buckets = get_bits(gb, 5);
    //    skip_bits(gb, 4); //bitrate exponent
    //    skip_bits(gb, 4); //buffer size exponent
    //    for (i = 0; i < v->hrd_num_leaky_buckets; i++) {
    //        skip_bits(gb, 16); //hrd_rate[n]
    //        skip_bits(gb, 16); //hrd_buffer[n]
    //    }
    //}

    return 0;
}

/**
 * Decode Simple/Main Profiles sequence header
 * @see Figure 7-8, p16-17
 * @param avctx Codec context
 * @param gb GetBit context initialized from Codec context extra_data
 * @return Status
 */
static int ff_vc1_decode_sequence_header( /*AVCodecContext *avctx,*/ MmpVC1Context *v, CMmpBitExtractor& be)
{
    unsigned long pt;
    //av_log(avctx, AV_LOG_DEBUG, "Header: %0X\n", show_bits_long(gb, 32));
    v->profile = be.Pop_BitCode(pt, 2); // beget_bits(gb, 2);
    if (v->profile == PROFILE_COMPLEX) {
        //av_log(avctx, AV_LOG_WARNING, "WMV3 Complex Profile is not fully supported\n");
    }

    if (v->profile == PROFILE_ADVANCED) {
        //v->zz_8x4 = ff_vc1_adv_progressive_8x4_zz;
        //v->zz_4x8 = ff_vc1_adv_progressive_4x8_zz;
        return decode_sequence_header_adv(v, be);
    } else {
        v->chromaformat = 1;
        //v->zz_8x4 = ff_wmv2_scantableA;
        //v->zz_4x8 = ff_wmv2_scantableB;
        v->res_y411   = be.Pop_1Bit();//get_bits1(gb);
        v->res_sprite = be.Pop_1Bit(); //get_bits1(gb);
        if (v->res_y411) {
            //av_log(avctx, AV_LOG_ERROR,
             //      "Old interlaced mode is not supported\n");
            return -1;
        }
    }

    // (fps-2)/4 (->30)
    v->frmrtq_postproc = be.Pop_BitCode(pt, 3); //get_bits(gb, 3); //common
    // (bitrate-32kbps)/64kbps
    v->bitrtq_postproc = be.Pop_BitCode(pt, 5); //get_bits(gb, 5); //common
    v->s.loop_filter   = be.Pop_1Bit(); //get_bits1(gb); //common
    if (v->s.loop_filter == 1 && v->profile == PROFILE_SIMPLE) {
        //av_log(avctx, AV_LOG_ERROR,
          //     "LOOPFILTER shall not be enabled in Simple Profile\n");
    }
    //if (v->s.avctx->skip_loop_filter >= AVDISCARD_ALL)
    //    v->s.loop_filter = 0;

    v->res_x8          = be.Pop_1Bit(); //get_bits1(gb); //reserved
    v->multires        = be.Pop_1Bit(); //get_bits1(gb);
    v->res_fasttx      = be.Pop_1Bit(); //get_bits1(gb);
    if (!v->res_fasttx) {
        //v->vc1dsp.vc1_inv_trans_8x8    = ff_simple_idct_8;
        //v->vc1dsp.vc1_inv_trans_8x4    = ff_simple_idct84_add;
        //v->vc1dsp.vc1_inv_trans_4x8    = ff_simple_idct48_add;
        //v->vc1dsp.vc1_inv_trans_4x4    = ff_simple_idct44_add;
        //v->vc1dsp.vc1_inv_trans_8x8_dc = ff_simple_idct_add_8;
        //v->vc1dsp.vc1_inv_trans_8x4_dc = ff_simple_idct84_add;
        //v->vc1dsp.vc1_inv_trans_4x8_dc = ff_simple_idct48_add;
        //v->vc1dsp.vc1_inv_trans_4x4_dc = ff_simple_idct44_add;
    }

    v->fastuvmc        = be.Pop_1Bit(); //get_bits1(gb); //common
    if (!v->profile && !v->fastuvmc) {
        //av_log(avctx, AV_LOG_ERROR,
         //      "FASTUVMC unavailable in Simple Profile\n");
        return -1;
    }
    v->extended_mv     = be.Pop_1Bit(); //get_bits1(gb); //common
    if (!v->profile && v->extended_mv) {
        //av_log(avctx, AV_LOG_ERROR,
         //      "Extended MVs unavailable in Simple Profile\n");
        return -1;
    }
    v->dquant          = be.Pop_BitCode(pt, 2); //get_bits(gb, 2); //common
    v->vstransform     = be.Pop_1Bit(); //get_bits1(gb); //common

    v->res_transtab    = be.Pop_1Bit();//get_bits1(gb);
    if (v->res_transtab) {
        //av_log(avctx, AV_LOG_ERROR,
               //"1 for reserved RES_TRANSTAB is forbidden\n");
        return -1;
    }

    v->overlap         = be.Pop_1Bit();//get_bits1(gb); //common

    v->resync_marker   = be.Pop_1Bit();//get_bits1(gb);
    v->rangered        = be.Pop_1Bit();//get_bits1(gb);
    if (v->rangered && v->profile == PROFILE_SIMPLE) {
        //av_log(avctx, AV_LOG_INFO,
          //     "RANGERED should be set to 0 in Simple Profile\n");
    }

    v->s.max_b_frames = be.Pop_BitCode(pt, 3); //avctx->max_b_frames = get_bits(gb, 3); //common
    v->quantizer_mode = be.Pop_BitCode(pt, 2); //get_bits(gb, 2); //common

    v->finterpflag = be.Pop_1Bit(); //get_bits1(gb); //common

    if (v->res_sprite) {
        int w = be.Pop_BitCode(pt, 11); //get_bits(gb, 11);
        int h = be.Pop_BitCode(pt, 11); //get_bits(gb, 11);
        //int ret = ff_set_dimensions(v->s.avctx, w, h);
        //if (ret < 0) {
        //    av_log(avctx, AV_LOG_ERROR, "Failed to set dimensions %d %d\n", w, h);
        //    return ret;
        //}
        be.Pop_BitCode(pt, 5); //skip_bits(gb, 5); //frame rate
        v->res_x8 = be.Pop_1Bit();// //get_bits1(gb);
        if (be.Pop_1Bit() /*get_bits1(gb)*/) { // something to do with DC VLC selection
            //av_log(avctx, AV_LOG_ERROR, "Unsupported sprite feature\n");
            return -1;
        }
        be.Pop_BitCode(pt, 3); //skip_bits(gb, 3); //slice code
        v->res_rtm_flag = 0;
    } else {
        v->res_rtm_flag = be.Pop_1Bit(); //get_bits1(gb); //reserved
    }
    if (!v->res_rtm_flag) {
        //av_log(avctx, AV_LOG_ERROR,
         //      "Old WMV3 version detected, some frames may be decoded incorrectly\n");
        //return -1;
    }
    //TODO: figure out what they mean (always 0x402F)

    //if (!v->res_fasttx)
     //   skip_bits(gb, 16);
    //av_log(avctx, AV_LOG_DEBUG,
    //       "Profile %i:\nfrmrtq_postproc=%i, bitrtq_postproc=%i\n"
    //       "LoopFilter=%i, MultiRes=%i, FastUVMC=%i, Extended MV=%i\n"
    //       "Rangered=%i, VSTransform=%i, Overlap=%i, SyncMarker=%i\n"
    //       "DQuant=%i, Quantizer mode=%i, Max B-frames=%i\n",
    //       v->profile, v->frmrtq_postproc, v->bitrtq_postproc,
    //       v->s.loop_filter, v->multires, v->fastuvmc, v->extended_mv,
    //       v->rangered, v->vstransform, v->overlap, v->resync_marker,
    //       v->dquant, v->quantizer_mode, avctx->max_b_frames);

    return 0;
}

static int ff_vc1_decode_entry_point(MmpVC1Context *v, CMmpBitExtractor &be)
{
    int i;
    int w,h;
    //int ret;

    //av_log(avctx, AV_LOG_DEBUG, "Entry point: %08X\n", show_bits_long(gb, 32));
    v->broken_link    = be.Pop_1Bit();//get_bits1(gb);
    v->closed_entry   = be.Pop_1Bit();//get_bits1(gb);
    v->panscanflag    = be.Pop_1Bit();//get_bits1(gb);
    v->refdist_flag   = be.Pop_1Bit();//get_bits1(gb);
    v->s.loop_filter  = be.Pop_1Bit();//get_bits1(gb);
    //if (v->s.avctx->skip_loop_filter >= AVDISCARD_ALL)
    //    v->s.loop_filter = 0;
    v->fastuvmc       = be.Pop_1Bit();//get_bits1(gb);
    v->extended_mv    = be.Pop_1Bit();//get_bits1(gb);
    v->dquant         = be.Pop_Bit(2);//get_bits(gb, 2);
    v->vstransform    = be.Pop_1Bit();//get_bits1(gb);
    v->overlap        = be.Pop_1Bit();//get_bits1(gb);
    v->quantizer_mode = be.Pop_Bit(2);//get_bits(gb, 2);

    if (v->hrd_param_flag) {
        for (i = 0; i < v->hrd_num_leaky_buckets; i++) {
            //skip_bits(gb, 8); //hrd_full[n]
            be.Pop_Bit(8);
        }
    }

    if(be.Pop_1Bit() /*get_bits1(gb)*/){
        w = (be.Pop_Bit(12)+1)<<1; //(get_bits(gb, 12)+1)<<1;
        h = (be.Pop_Bit(12)+1)<<1; //(get_bits(gb, 12)+1)<<1;
    } else {
        w = v->max_coded_width;
        h = v->max_coded_height;
    }
    //if ((ret = ff_set_dimensions(avctx, w, h)) < 0) {
        //av_log(avctx, AV_LOG_ERROR, "Failed to set dimensions %d %d\n", w, h);
      //  return ret;
    //}

    if (v->extended_mv)
        v->extended_dmv = be.Pop_1Bit();//get_bits1(gb);
    if ((v->range_mapy_flag = be.Pop_1Bit() /*get_bits1(gb)*/)) {
        //av_log(avctx, AV_LOG_ERROR, "Luma scaling is not supported, expect wrong picture\n");
        v->range_mapy = (MMP_U8)be.Pop_Bit(3); //get_bits(gb, 3);
    }
    if ((v->range_mapuv_flag = be.Pop_1Bit() /*get_bits1(gb)*/)) {
        //av_log(avctx, AV_LOG_ERROR, "Chroma scaling is not supported, expect wrong picture\n");
        v->range_mapuv = (MMP_U8)be.Pop_Bit(3); //get_bits(gb, 3);
    }

    //av_log(avctx, AV_LOG_DEBUG, "Entry point info:\n"
    //       "BrokenLink=%i, ClosedEntry=%i, PanscanFlag=%i\n"
    //       "RefDist=%i, Postproc=%i, FastUVMC=%i, ExtMV=%i\n"
    //       "DQuant=%i, VSTransform=%i, Overlap=%i, Qmode=%i\n",
      //     v->broken_link, v->closed_entry, v->panscanflag, v->refdist_flag, v->s.loop_filter,
       //    v->fastuvmc, v->extended_mv, v->dquant, v->vstransform, v->overlap, v->quantizer_mode);

    return 0;
}

/*
    ref  Z:\develop\odysseus\android\android-6.0.1_r43\external\ffmpeg\ffmpeg-n3.1.1\libavcodec\vc1dec.c 
*/
enum MMP_FOURCC CMmpWMVParser::get_fourcc_with_extradata(MMP_U8 *extra_data, MMP_S32 extra_data_size) {

    MmpVC1Context vc1ctx;
    return CMmpWMVParser::get_fourcc_with_extradata(extra_data, extra_data_size, &vc1ctx);
}

enum MMP_FOURCC CMmpWMVParser::get_fourcc_with_extradata(MMP_U8 *extra_data, MMP_S32 extra_data_size, MmpVC1Context* p_vc1ctx) {

    CMmpBitExtractor be;
    enum MMP_FOURCC fourcc_ret = MMP_FOURCC_VIDEO_UNKNOWN;
    //MmpVC1Context vc1ctx;
    int ret;

    /*
        Check VC1,  VC1 extra_data_size�� 16byte���� ũ�� 
    */
    if(extra_data_size > 16) {

        const MMP_U8 *start = extra_data;
        const MMP_U8 *next;
        const MMP_U8 *buf2 = NULL;
        MMP_U8 *end = extra_data + extra_data_size;
        MMP_S32 size, buf2_size;
        int seq_initialized = 0;
        int ep_initialized = 0;
        
        buf2  = (const MMP_U8 *)MMP_MALLOC(extra_data_size + AV_INPUT_BUFFER_PADDING_SIZE);

        start = find_next_marker(start, end); // in WVC1 extradata first byte is its size, but can be 0 in mkv
        next  = start;
        for (; next < end; start = next) {

            next = find_next_marker(start + 4, end);
            size = next - start - 4;
            if (size <= 0)
                continue;

            buf2_size = vc1_unescape_buffer( (const MMP_U8 *)(start + 4), size, (MMP_U8 *)buf2);
            be.Start((unsigned char*)buf2, buf2_size);

            switch(MMP_RB32(start)) {

                case VC1_CODE_SEQHDR:
                    if ((ret = ff_vc1_decode_sequence_header(p_vc1ctx, be)) < 0) {
                        MMP_FREE((void*)buf2); //av_free(buf2);
                        return MMP_FOURCC_VIDEO_UNKNOWN;
                    }
                    seq_initialized = 1;
                    break;

                case VC1_CODE_ENTRYPOINT:
                    if ((ret = ff_vc1_decode_entry_point(p_vc1ctx, be)) < 0) {
                        MMP_FREE((void*)buf2); //av_free(buf2);
                        return MMP_FOURCC_VIDEO_UNKNOWN;
                    }
                    ep_initialized = 1;
                    break;
            }

        }

        MMP_FREE((void*)buf2);

        if( (seq_initialized == 1) && (ep_initialized == 1)  ) {
            fourcc_ret = MMP_FOURCC_VIDEO_WVC1;
        }
    }

    /*
        Check WMV3 
    */
    if(fourcc_ret == MMP_FOURCC_VIDEO_UNKNOWN) {
    
        be.Start((unsigned char*)extra_data, extra_data_size);
        ret = ff_vc1_decode_sequence_header(p_vc1ctx, be);
        if(ret == 0) {
            fourcc_ret = MMP_FOURCC_VIDEO_WMV3;
        }
    
    }

    return fourcc_ret;
}

enum MmpWMV3Profile CMmpWMVParser::get_profile(MMP_U8 *extra_data, MMP_S32 extra_data_size) /* Check if WMV3, Compled_Profile */ {

    
    MmpVC1Context vc1ctx;
    enum MMP_FOURCC fourcc;
    enum MmpWMV3Profile profile = MMP_WMV3_PROFILE_UNKNOWN;

    if(extra_data_size > 0) {
        fourcc = CMmpWMVParser::get_fourcc_with_extradata(extra_data, extra_data_size, &vc1ctx);
        if(fourcc != MMP_FOURCC_VIDEO_UNKNOWN) {
            profile = (enum MmpWMV3Profile)vc1ctx.profile;
        }
    }

    return profile;
}

