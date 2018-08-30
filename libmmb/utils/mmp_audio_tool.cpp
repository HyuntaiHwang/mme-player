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

#include "mmp_audio_tool.hpp"
#include "MmpBitExtractor.hpp"


const unsigned short MMP_AC3_FREQS[3] = { 48000, 44100, 32000 };
/* possible bitrates */
const unsigned short MMP_AC3_BITRATETAB[19] = {
    32, 40, 48, 56, 64, 80, 96, 112, 128,
    160, 192, 224, 256, 320, 384, 448, 512, 576, 640
};
const unsigned char MMP_AC3_CHANNELAS[8] = {
    2, 1, 2, 3, 3, 4, 4, 5
};

/**
 * Possible frame sizes.
 * from ATSC A/52 Table 5.18 Frame Size Code Table.
 */
const unsigned short MMP_AC3_FRAME_SIZES[38][3] = {
    { 64,   69,   96   },
    { 64,   70,   96   },
    { 80,   87,   120  },
    { 80,   88,   120  },
    { 96,   104,  144  },
    { 96,   105,  144  },
    { 112,  121,  168  },
    { 112,  122,  168  },
    { 128,  139,  192  },
    { 128,  140,  192  },
    { 160,  174,  240  },
    { 160,  175,  240  },
    { 192,  208,  288  },
    { 192,  209,  288  },
    { 224,  243,  336  },
    { 224,  244,  336  },
    { 256,  278,  384  },
    { 256,  279,  384  },
    { 320,  348,  480  },
    { 320,  349,  480  },
    { 384,  417,  576  },
    { 384,  418,  576  },
    { 448,  487,  672  },
    { 448,  488,  672  },
    { 512,  557,  768  },
    { 512,  558,  768  },
    { 640,  696,  960  },
    { 640,  697,  960  },
    { 768,  835,  1152 },
    { 768,  836,  1152 },
    { 896,  975,  1344 },
    { 896,  976,  1344 },
    { 1024, 1114, 1536 },
    { 1024, 1115, 1536 },
    { 1152, 1253, 1728 },
    { 1152, 1254, 1728 },
    { 1280, 1393, 1920 },
    { 1280, 1394, 1920 },
};


MMP_RESULT mmp_audio_tool::Ac3ParseHeader(unsigned char* buf, int bufSize, MmpAC3HeaderInfo *hdr)
{
    CMmpBitExtractor be;
    be.Start(buf, bufSize);
    unsigned long dummy;

    memset(hdr, 0, sizeof(MmpAC3HeaderInfo));

    
    hdr->sync_word = (unsigned short)be.Pop_BitCode(dummy, 16);
    if(hdr->sync_word != 0x0B77)
    {
        return MMP_FAILURE;//AC3_PARSE_ERROR_SYNC;
    }

    /* read ahead to bsid to make sure this is AC-3, not E-AC-3 */
    hdr->bsid = (unsigned char)be.Query_BitCode(dummy, 29)&0x1F; //show_bits_long(&gbc, 29) & 0x1F;
    if(hdr->bsid > 10)
    {
        return MMP_FAILURE;//AC3_PARSE_ERROR_BSID;
    }

    hdr->crc1 = (unsigned short)be.Pop_BitCode(dummy, 16);//get_bits(&gbc, 16);
    hdr->fscod = (unsigned char)be.Pop_BitCode(dummy, 2); //get_bits(&gbc, 2);
    if(hdr->fscod == 3)
    {
        return MMP_FAILURE;//AC3_PARSE_ERROR_SAMPLE_RATE;
    }

    hdr->frmsizecod = (unsigned char)be.Pop_BitCode(dummy, 6);//get_bits(&gbc, 6);
    if(hdr->frmsizecod > 37)
    {
        return MMP_FAILURE;//AC3_PARSE_ERROR_FRAME_SIZE;
    }

    dummy=be.Pop_BitCode(dummy, 5);//skip_bits(&gbc, 5); // skip bsid, already got it

    hdr->bsmod = (unsigned char)be.Pop_BitCode(dummy, 3);//get_bits(&gbc, 3);
    hdr->acmod = (unsigned char)be.Pop_BitCode(dummy, 3);//get_bits(&gbc, 3);
    if((hdr->acmod & 1) && hdr->acmod != MMP_AC3_ACMOD_MONO) 
    {
        hdr->cmixlev = (unsigned char)be.Pop_BitCode(dummy, 2);///get_bits(&gbc, 2);
    }
    if(hdr->acmod & 4) 
    {
        hdr->surmixlev = (unsigned char)be.Pop_BitCode(dummy, 2);//get_bits(&gbc, 2);
    }
    if(hdr->acmod == MMP_AC3_ACMOD_STEREO) 
    {
        hdr->dsurmod = (unsigned char)be.Pop_BitCode(dummy, 3);//get_bits(&gbc, 2);
    }
    hdr->lfeon = (unsigned char)be.Pop_BitCode(dummy, 1);//get_bits1(&gbc);

    hdr->halfratecod = MMP_MAX(hdr->bsid, 8) - 8;
    hdr->sample_rate = MMP_AC3_FREQS[hdr->fscod] >> hdr->halfratecod;
    hdr->bit_rate = (MMP_AC3_BITRATETAB[hdr->frmsizecod>>1] * 1000) >> hdr->halfratecod;
    hdr->channels = MMP_AC3_CHANNELAS[hdr->acmod] + hdr->lfeon;
    hdr->frame_size = MMP_AC3_FRAME_SIZES[hdr->frmsizecod][hdr->fscod] * 2;

    return MMP_SUCCESS;
}

MMP_RESULT mmp_audio_tool::Convert48kPCM_To_8kPCM() {

#define SOURCE_PCM_FILE "C:\\work\\nemo_48000.pcm"
#define DEST_PCM_FILE "C:\\work\\nemo_8000.pcm"

    FILE* fp_src, *fp_dst;
    MMP_S16* src_pcm = NULL;
    MMP_S16* dest_pcm = NULL;
    MMP_S16* src_pcm_L = NULL;
    MMP_S16* src_pcm_R = NULL;
    int err_cnt = 0;
    int i,j,k, m;
    MMP_U16 u16;
    MMP_S32 src_pcmsize;
    MMP_S32 avg_L, avg_R;

    /* file open*/
    fp_src = fopen(SOURCE_PCM_FILE, "rb");
    if(fp_src == NULL) {
        err_cnt++;
    }
    else {
        fseek(fp_src, 0, SEEK_END);
        src_pcmsize = ftell(fp_src);
        fseek(fp_src, 0, SEEK_SET);
        
        src_pcm = new MMP_S16[src_pcmsize/2];
        src_pcm_L = new MMP_S16[src_pcmsize/2];
        src_pcm_R = new MMP_S16[src_pcmsize/2];
        dest_pcm = new MMP_S16[src_pcmsize/2];
        
        fread(src_pcm, 1, src_pcmsize, fp_src);

        for(i = 0, j = 0; i < src_pcmsize/2; i+=2, j++) {
        
            src_pcm_L[j] = src_pcm[i];
            src_pcm_R[j] = src_pcm[i+1];
        }

        avg_L = 0;
        avg_R = 0;
        k = 1;
        m = 0;
        for(i = 0; i < j; i++) {
        
            //avg_L = ((k-1)*avg_L + src_pcm_L[i])/k;
            //avg_R = ((k-1)*avg_R + src_pcm_R[i])/k;
            avg_L += (MMP_S32)src_pcm_L[i];
            avg_R += (MMP_S32)src_pcm_L[i];
            
            if( (k%6) == 0) {

                dest_pcm[m] = (MMP_S16)(avg_L/6);
                dest_pcm[m+1] = (MMP_S16)(avg_R/6);

                u16 = (MMP_U16)dest_pcm[m];
                dest_pcm[m] = MMP_SWAP_U16(u16);

                u16 = (MMP_U16)dest_pcm[m+1];
                dest_pcm[m+1] = MMP_SWAP_U16(u16);

                m+=2;

                k = 1;
                avg_L = 0;
                avg_R = 0;
            }
            else {
                k++;
            }
        }

    }

    fp_dst = fopen(DEST_PCM_FILE, "wb");
    if(fp_dst == NULL) {
        err_cnt++;
    }
    else {
        fwrite(dest_pcm, 1, m*2, fp_dst);
    }

    
    if(fp_src != NULL) fclose(fp_src);
    if(fp_dst != NULL) fclose(fp_dst);

    if(src_pcm != NULL) delete [] src_pcm;
    if(src_pcm_L != NULL) delete [] src_pcm_L;
    if(src_pcm_R != NULL) delete [] src_pcm_R;
    if(dest_pcm != NULL) delete [] dest_pcm;

    return MMP_SUCCESS;
}

MMP_RESULT mmp_audio_tool::make_audio_packet_header(void* dsi_data, MMP_S32 dsi_size, 
                                                    void* ffmpeg_hdr, MMP_S32 ffmpeg_hdr_size,
                                                    MMP_ADDR dst_buffer) {

    struct mmp_audio_packet_header* p_audio_pkt_hdr = (struct mmp_audio_packet_header*)dst_buffer;
    MMP_ADDR data;

    /* set hdr */
    p_audio_pkt_hdr->key = MMP_AUDIO_PACKET_HEADER_KEY;
    p_audio_pkt_hdr->packet_hdr_size = sizeof(struct mmp_audio_packet_header);
    
    /* set prop */
    
    /* set dsi */
    p_audio_pkt_hdr->dsi_size = dsi_size;
    p_audio_pkt_hdr->dsi_pos = MMP_BYTE_ALIGN_16B(p_audio_pkt_hdr->packet_hdr_size);
    data = dst_buffer + p_audio_pkt_hdr->dsi_pos;
    if(p_audio_pkt_hdr->dsi_size > 0) {
        memcpy((void*)data, dsi_data, p_audio_pkt_hdr->dsi_size);
    }
    
    /* set ffmpeg hdr */
    p_audio_pkt_hdr->ffmpeg_hdr_size = ffmpeg_hdr_size;
    p_audio_pkt_hdr->ffmpeg_hdr_pos = MMP_BYTE_ALIGN_16B(p_audio_pkt_hdr->dsi_pos + p_audio_pkt_hdr->dsi_size);
    data = dst_buffer + p_audio_pkt_hdr->ffmpeg_hdr_pos;
    if(p_audio_pkt_hdr->ffmpeg_hdr_size > 0) {
        memcpy((void*)data, ffmpeg_hdr, p_audio_pkt_hdr->ffmpeg_hdr_size);
    }

    /* set pkt size */
    p_audio_pkt_hdr->packet_size =  p_audio_pkt_hdr->ffmpeg_hdr_pos + p_audio_pkt_hdr->ffmpeg_hdr_size;
    
    return MMP_SUCCESS;
}

MMP_BOOL mmp_audio_tool::is_valid_audio_packet_header(MMP_ADDR pkt_addr, MMP_S32 buf_size) {

    MMP_BOOL is_valid = MMP_TRUE;
    struct mmp_audio_packet_header audio_pkt_hdr;
    
    /* check buf size */
    if(buf_size >= sizeof(struct mmp_audio_packet_header)) {
        memcpy(&audio_pkt_hdr, (void*)pkt_addr, sizeof(struct mmp_audio_packet_header));
    }
    else {
        is_valid = MMP_FALSE;
    }

    /* check key */
    if( (is_valid == MMP_TRUE) && (audio_pkt_hdr.key != MMP_AUDIO_PACKET_HEADER_KEY) ) {
        is_valid = MMP_FALSE;
    }

    /* check pkt hdr size */
    if( (is_valid == MMP_TRUE) && (audio_pkt_hdr.packet_hdr_size != sizeof(struct mmp_audio_packet_header) ) ) {
        is_valid = MMP_FALSE;
    }

    /* check pkt size */
    if( (is_valid == MMP_TRUE) && (audio_pkt_hdr.packet_size !=  (audio_pkt_hdr.ffmpeg_hdr_pos + audio_pkt_hdr.ffmpeg_hdr_size) ) ) {
        is_valid = MMP_FALSE;
    }

    return is_valid;
}

MMP_S64 mmp_audio_tool::pcm_cal_dur_us(MMP_S32 pcm_byte, MMP_S32 sr /*sample rate*/, MMP_S32 ch/*channel*/, MMP_S32 bps /* bits per sample */) {

    MMP_S64 abps = (MMP_S64)pcm_cal_abps(sr, ch, bps); /* avg byte per sec */
    MMP_S64 dur_us = 0;
    //pcm_byte:dur_us  = abps : 1000000L 

    if(abps != 0) {
        dur_us = (MMP_S64)pcm_byte*1000000L / abps;
    }
    
    return dur_us;
}

