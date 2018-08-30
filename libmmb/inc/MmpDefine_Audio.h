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

#ifndef MMPDEFINE_AUDIO_H__
#define MMPDEFINE_AUDIO_H__

#define MMP_WAVE_FORMAT_PCM                    1  

#if 0
#define MMP_WAVE_FORMAT_ADPCM                  2  
#define MMP_WAVE_FORMAT_MPEGLAYER3             0x0055 // ISO/MPEG Layer3 Format Tag 
#define MMP_WAVE_FORMAT_MPEGLAYER2             0x0050 // ISO/MPEG Layer2 Format Tag 
#define MMP_WAVE_FORMAT_BSAC                   0xD000 // Korea TDMB Audio Bsac

#define MMP_WAVE_FORMAT_AAC                    0x00FF // China TDMB Audio AAXC
#define MMP_WAVE_FORMAT_EAAC                   0x00FE // China TDMB Audio AAXC
#define MMP_WAVE_FORMAT_AAC_LC                 0x706D //Format: AAC(Advanced Audio Codec) version: Version2  Profile: LC

#define MMP_WAVE_FORMAT_RA_COOK                0x6F63 //Real Audio  Cook 
#define MMP_WAVE_FORMAT_RA_RAAC                0x504d //Real Audio  Raac
#define MMP_WAVE_FORMAT_RA_SIPR                0x6973 //Real Audio  SIPR

#define MMP_WAVE_FORMAT_AC3                    0x2000   //AC3
#define MMP_WAVE_FORMAT_DTS                    0x2001  //DTS 
#define MMP_WAVE_FORMAT_WMA2                   0x0161  //WMA 2
#define MMP_WAVE_FORMAT_WMA3                   0x0162  //WMA 3

#define MMP_WAVE_MY_EXTEND                     0xFF00   //I don't knwo what the wave number is..
#define MMP_WAVE_FORMAT_FLAC                   (MMP_WAVE_MY_EXTEND+1)  //FLAC
#define MMP_WAVE_FORMAT_ADPCM_MS               (MMP_WAVE_MY_EXTEND+2) 
#define MMP_WAVE_FORMAT_FFMPEG                 (MMP_WAVE_MY_EXTEND+3) 
#define MMP_WAVE_FORMAT_MPEGLAYER1             (MMP_WAVE_MY_EXTEND+4)  // ISO/MPEG Layer1 Format Tag 
#endif

typedef enum MMP_AUDIO_AACSTREAMFORMATTYPE {
    MMP_AUDIO_AACStreamFormatMP2ADTS = 0, /**< AAC Audio Data Transport Stream 2 format */
    MMP_AUDIO_AACStreamFormatMP4ADTS,     /**< AAC Audio Data Transport Stream 4 format */
    MMP_AUDIO_AACStreamFormatMP4LOAS,     /**< AAC Low Overhead Audio Stream format */
    MMP_AUDIO_AACStreamFormatMP4LATM,     /**< AAC Low overhead Audio Transport Multiplex */
    MMP_AUDIO_AACStreamFormatADIF,        /**< AAC Audio Data Interchange Format */
    MMP_AUDIO_AACStreamFormatMP4FF,       /**< AAC inside MPEG-4/ISO File Format */
    MMP_AUDIO_AACStreamFormatRAW,         /**< AAC Raw Format */
    MMP_AUDIO_AACStreamFormatKhronosExtensions = 0x6F000000, /**< Reserved region for introducing Khronos Standard Extensions */ 
    MMP_AUDIO_AACStreamFormatVendorStartUnused = 0x7F000000, /**< Reserved region for introducing Vendor Extensions */
    MMP_AUDIO_AACStreamFormatMax = 0x7FFFFFFF
} MMP_AUDIO_AACSTREAMFORMATTYPE;

typedef struct _MMPWAVEFORMATEX
{
    unsigned short        wFormatTag;         /* format type */
    unsigned short        nChannels;          /* number of channels (i.e. mono, stereo...) */
    MMP_U32       nSamplesPerSec;     /* sample rate */
    MMP_U32       nAvgBytesPerSec;    /* for buffer estimation */
    unsigned short        nBlockAlign;        /* block size of data */
    unsigned short        wBitsPerSample;     /* number of bits per sample of mono data */
    unsigned short        cbSize;             /* the count in bytes of the size of */
                                    /* extra information (after cbSize) */
}MMPWAVEFORMATEX;
#define MMPWAVEFORMATEX_CAL_nAvgBytesPerSec(wf) (wf.nSamplesPerSec*wf.nChannels*(wf.wBitsPerSample/8))

/** WMA Version */
enum MMP_AUDIO_WMAFORMATTYPE {
  MMP_AUDIO_WMAFormatUnused = 0, /**< format unused or unknown */
  MMP_AUDIO_WMAFormat7,          /**< Windows Media Audio format 7 */
  MMP_AUDIO_WMAFormat8,          /**< Windows Media Audio format 8 */
  MMP_AUDIO_WMAFormat9,          /**< Windows Media Audio format 9 */
  MMP_AUDIO_WMAFormatKhronosExtensions = 0x6F000000, /**< Reserved region for introducing Khronos Standard Extensions */
  MMP_AUDIO_WMAFormatVendorStartUnused = 0x7F000000, /**< Reserved region for introducing Vendor Extensions */
  MMP_AUDIO_WMAFormatMax = 0x7FFFFFFF
};


/** WMA Profile */
enum MMP_AUDIO_WMAPROFILETYPE {
  MMP_AUDIO_WMAProfileUnused = 0,  /**< profile unused or unknown */
  MMP_AUDIO_WMAProfileL1,          /**< Windows Media audio version 9 profile L1 */
  MMP_AUDIO_WMAProfileL2,          /**< Windows Media audio version 9 profile L2 */
  MMP_AUDIO_WMAProfileL3,          /**< Windows Media audio version 9 profile L3 */
  MMP_AUDIO_WMAProfileKhronosExtensions = 0x6F000000, /**< Reserved region for introducing Khronos Standard Extensions */
  MMP_AUDIO_WMAProfileVendorStartUnused = 0x7F000000, /**< Reserved region for introducing Vendor Extensions */
  MMP_AUDIO_WMAProfileMax = 0x7FFFFFFF
};

/************************************************************************************************************
    Audio Format
************************************************************************************************************/
#define MMP_AUDIO_FORMAT_KEY 0xFFCCAA00
#define MMP_AUDIO_FORMAT_MAX_EXTRA_DATA_SIZE 64
struct mmp_audio_format {
    MMP_U32 key;
    enum MMP_FOURCC fourcc;
  
    MMP_S32 sample_rate;
    MMP_S32 channels;
    MMP_S32 bits_per_sample;
    MMP_S32 bitrate;
    MMP_S32 block_align;
    MMP_S32 frame_size;
    MMP_U32 codec_tag;
    
    MMP_S32 extra_data_size;
    MMP_U8  extra_data[MMP_AUDIO_FORMAT_MAX_EXTRA_DATA_SIZE];
};

#define MMP_AUDIO_PACKET_HEADER_KEY 0x0ABBCCDDEE101004L
struct mmp_audio_packet_header {
    MMP_U64 key;
    MMP_S32 packet_hdr_size;
    MMP_S32 packet_size; /* packet_hdr_size + dsi_size + ffmpeg_hdr_size */ 
        
    enum MMP_FOURCC fourcc;
  
    MMP_S32 sample_rate;
    MMP_S32 channels;
    MMP_S32 bits_per_sample;
    MMP_S32 bitrate;
    MMP_S32 block_align;
    MMP_S32 frame_size;
    MMP_U32 codec_tag;
    
    MMP_S32 dsi_size;
    MMP_S32 dsi_pos;

    MMP_S32 ffmpeg_hdr_size;
    MMP_S32 ffmpeg_hdr_pos;
};

/* anapass adsp codec config structure : WMA,  see [android]/system/media/audio/include/system */
typedef struct android_adsp_codec_wma_s {
    MMP_U16 i_samp_freq;
	MMP_U16 i_num_chan;
	MMP_U16 i_pcm_wd_sz;
	MMP_U16 i_w_fmt_tag;
	MMP_U16 i_blk_align;
	MMP_U16 i_encode_opt;
	MMP_U16 i_avg_byte;
    MMP_U16 extra_data_size;
    MMP_U8 extra_data[16];
}android_adsp_codec_wma_t;

/* anapass adsp codec config structure : FLAC */
#define FFMPEG_FLAC_STREAMINFO_SIZE   34
#define FFMPEG_FLAC_MAX_CHANNELS       8
#define FFMPEG_FLAC_MIN_BLOCKSIZE     16
#define FFMPEG_FLAC_MAX_BLOCKSIZE  65535
#define FFMPEG_FLAC_MIN_FRAME_SIZE    11
typedef struct android_adsp_codec_flac_s {
    MMP_U16 extra_data_size;
    MMP_U16 reserved1;
    MMP_U8 extra_data[FFMPEG_FLAC_STREAMINFO_SIZE];
}android_adsp_codec_flac_t;

#endif
