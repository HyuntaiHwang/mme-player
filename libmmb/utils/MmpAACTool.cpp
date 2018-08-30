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

#include "MmpAACTool.hpp"
#include "MmpBitExtractor.hpp"
#include "MmpUtil.hpp"

MMP_RESULT CMmpAACTool::UnpackADTSHeader(unsigned char* adts_stream, int adts_stream_size, unsigned char* aac_stream, int* aac_stream_size)
{
    SMmpADTSHeader adtsHeader;
    SMmpADTSHeader *fhADTS=&adtsHeader;
    CMmpBitExtractor be;
    unsigned long dummy, code;
    int offset;

    be.Start(adts_stream, adts_stream_size);

    /* verify that first 12 bits of header are syncword */
    code=be.Pop_BitCode(dummy, 12);
    if(code!=0x0fff)
    {
        //Not ADTS Format
        return MMP_FAILURE;
    }

    /* fixed fields - should not change from frame to frame */ 
    fhADTS->id =               (unsigned char)be.Pop_BitCode(dummy, 1);
	fhADTS->layer =            (unsigned char)be.Pop_BitCode(dummy, 2);
	fhADTS->protectBit =       (unsigned char)be.Pop_BitCode(dummy, 1);
	fhADTS->profile =          (unsigned char)be.Pop_BitCode(dummy, 2);
	fhADTS->sampRateIdx =      (unsigned char)be.Pop_BitCode(dummy, 4);
	fhADTS->privateBit =       (unsigned char)be.Pop_BitCode(dummy, 1);
	fhADTS->channelConfig =    (unsigned char)be.Pop_BitCode(dummy, 3);
	fhADTS->origCopy =         (unsigned char)be.Pop_BitCode(dummy, 1);
	fhADTS->home =             (unsigned char)be.Pop_BitCode(dummy, 1);

    /* variable fields - can change from frame to frame */ 
	fhADTS->copyBit =          (unsigned char)be.Pop_BitCode(dummy, 1);
	fhADTS->copyStart =        (unsigned char)be.Pop_BitCode(dummy, 1);
	fhADTS->frameLength =      (int)be.Pop_BitCode(dummy, 13);
	fhADTS->bufferFull =       (int)be.Pop_BitCode(dummy, 11);
	fhADTS->numRawDataBlocks = (unsigned char)be.Pop_BitCode(dummy, 2) + 1;

    /* note - MPEG4 spec, correction 1 changes how CRC is handled when protectBit == 0 and numRawDataBlocks > 1 */
	if (fhADTS->protectBit == 0)
		fhADTS->crcCheckWord = (int)be.Pop_BitCode(dummy, 16);

    offset=be.GetCurByteIndex();
    *aac_stream_size=adts_stream_size-offset;
    memcpy(aac_stream, &adts_stream[offset], *aac_stream_size);

  //  MMPDEBUGMSG(1, (TEXT("SampleRate: %d \n\r"), fhADTS->sampRateIdx));
    return MMP_SUCCESS;
}

MMP_RESULT CMmpAACTool::UnpackADTSHeaderAndMakeCbData(unsigned char* adts_stream, int adts_stream_size, unsigned char* cb, int* cbsize)
{
    SMmpADTSHeader adtsHeader;
    return CMmpAACTool::UnpackADTSHeaderAndMakeCbData(adts_stream, adts_stream_size, &adtsHeader, cb, cbsize);
}

MMP_RESULT CMmpAACTool::UnpackADTSHeaderAndMakeCbData(unsigned char* adts_stream, int adts_stream_size, SMmpADTSHeader *fhADTS, unsigned char* cb, int* cbsize)
{
    //SMmpADTSHeader adtsHeader;
    //SMmpADTSHeader *fhADTS=&adtsHeader;
    CMmpBitExtractor be;
    unsigned long dummy, code;
    
    be.Start(adts_stream, adts_stream_size);

    /* verify that first 12 bits of header are syncword */
    code=be.Pop_BitCode(dummy, 12);
    if(code!=0x0fff)
    {
        //Not ADTS Format
        return MMP_FAILURE;
    }

    /* fixed fields - should not change from frame to frame */ 
    fhADTS->id =               (unsigned char)be.Pop_BitCode(dummy, 1);
	fhADTS->layer =            (unsigned char)be.Pop_BitCode(dummy, 2);
	fhADTS->protectBit =       (unsigned char)be.Pop_BitCode(dummy, 1);
	fhADTS->profile =          (unsigned char)be.Pop_BitCode(dummy, 2);
	fhADTS->sampRateIdx =      (unsigned char)be.Pop_BitCode(dummy, 4);
	fhADTS->privateBit =       (unsigned char)be.Pop_BitCode(dummy, 1);
	fhADTS->channelConfig =    (unsigned char)be.Pop_BitCode(dummy, 3);
	fhADTS->origCopy =         (unsigned char)be.Pop_BitCode(dummy, 1);
	fhADTS->home =             (unsigned char)be.Pop_BitCode(dummy, 1);

    /* variable fields - can change from frame to frame */ 
	fhADTS->copyBit =          (unsigned char)be.Pop_BitCode(dummy, 1);
	fhADTS->copyStart =        (unsigned char)be.Pop_BitCode(dummy, 1);
	fhADTS->frameLength =      (int)be.Pop_BitCode(dummy, 13);
	fhADTS->bufferFull =       (int)be.Pop_BitCode(dummy, 11);
	fhADTS->numRawDataBlocks = (unsigned char)be.Pop_BitCode(dummy, 2) + 1;

    /* note - MPEG4 spec, correction 1 changes how CRC is handled when protectBit == 0 and numRawDataBlocks > 1 */
	if (fhADTS->protectBit == 0)
		fhADTS->crcCheckWord = (int)be.Pop_BitCode(dummy, 16);

    unsigned char object_type, ch, samplerate_index;

    *cbsize=2;

    switch(fhADTS->profile)
    {
        case 0: object_type=1; break; //Main Profile
        case 1: object_type=2; break; //LC (Low Complexity) Profile
        case 2: object_type=3; break; //SSR (Scalable Sample Rate ) Profile
        default:
                //object_type=4  LTP
                //object_type=5  SBR
                object_type=fhADTS->profile+1;
    }

    ch=fhADTS->channelConfig;
    samplerate_index=fhADTS->sampRateIdx;

    cb[0]=((object_type<<3)&0xF8)|((samplerate_index>>1)&0x07);
    cb[1]=((ch<<3)&0x78)|((samplerate_index<<7)&0x80);
    
    return MMP_SUCCESS;
}

int CMmpAACTool::GetSamplingFreq(int index)
{
    int SamplingFrequencyIndex[16] = {96000, 88200, 64000, 48000, 44100, 32000, 24000,
                                     22050, 16000, 12000, 11025, 8000, 7350, 0, 0, 0 };
   
    if(index<0 || index>=16) return 0;

    return SamplingFrequencyIndex[index];
}


#define ADTS_HEADER_SIZE 7
MMP_RESULT CMmpAACTool::Make_ADTS(MMP_U8 *adts_header, MMP_S32 maxhdrsz, MMP_S32 *rethdrsz, MMP_U32 profile, MMP_S32 samplefreq, MMP_S32 ch, MMP_S32 ausize)
{
    MMP_U8 *hdr;
    MMP_U8 c;

    hdr = adts_header;
    memset(hdr, 0x00, ADTS_HEADER_SIZE);
    if(rethdrsz) *rethdrsz = ADTS_HEADER_SIZE; 

    /* sync word */
    *hdr = 0xFF;  hdr++;
    *hdr = 0xF9;  hdr++; /* 1 0 0 1     mpeg2, no layer,  not crc */
    //*hdr = 0xF1;  hdr++; /* 1 0 0 1     mpeg4, no layer,  not crc */

    /* profile */
    *hdr |= ((MMP_U8)(profile-1))<<6; 

    /*sample freq*/
    switch(samplefreq)
    {
       case 96000 : c = MMP_AAC_SAMPLEFREQ_INDEX_96KHZ;   break;
       case 88200 : c = MMP_AAC_SAMPLEFREQ_INDEX_88_2KHZ; break;
       case 64000 : c = MMP_AAC_SAMPLEFREQ_INDEX_64KHZ;   break;
       case 48000 : c = MMP_AAC_SAMPLEFREQ_INDEX_48KHZ;   break;
       case 44100 : c = MMP_AAC_SAMPLEFREQ_INDEX_44_1KHZ; break;
       case 32000 : c = MMP_AAC_SAMPLEFREQ_INDEX_32KHZ;   break;
       case 24000 : c = MMP_AAC_SAMPLEFREQ_INDEX_24KHZ;   break;
       case 22500 : c = MMP_AAC_SAMPLEFREQ_INDEX_22_5KHZ; break;
       case 16000 : c = MMP_AAC_SAMPLEFREQ_INDEX_16KHZ;   break;
       case 25000 : c = MMP_AAC_SAMPLEFREQ_INDEX_11_25KHZ; break;
       case  8000 : c = MMP_AAC_SAMPLEFREQ_INDEX_8KHZ;     break;
       case  7350 : c = MMP_AAC_SAMPLEFREQ_INDEX_7_35KHZ;  break;
       default:
           return MMP_FAILURE;
    }
    *hdr |= c<<2; 

    /* private bit */
    c = 0;
    *hdr |= c<<1;

    /* channel */
    c = (MMP_U8)ch;
    *hdr |= c>>2;
    hdr++;
    *hdr |= c<<6;

    /* frame lenght */
    ausize += ADTS_HEADER_SIZE;
    c = (ausize>>11)&3;
    *hdr |= c; hdr++;
    c = (ausize>>3)&0xFF;
    *hdr |= c; hdr++;
    c = (ausize>>0)&0x7;
    c<<=5;
    *hdr |= c;

    /* adts buffer fullness */
    c = 0x12;
    *hdr |= c; hdr++;
    c = 0x44;
    *hdr |= c;

    return MMP_SUCCESS;
}


enum AudioObjectType {
    AOT_NULL,
                               // Support?                Name
    AOT_AAC_MAIN,              ///< Y                       Main
    AOT_AAC_LC,                ///< Y                       Low Complexity
    AOT_AAC_SSR,               ///< N (code in SoC repo)    Scalable Sample Rate
    AOT_AAC_LTP,               ///< Y                       Long Term Prediction
    AOT_SBR,                   ///< Y                       Spectral Band Replication
    AOT_AAC_SCALABLE,          ///< N                       Scalable
    AOT_TWINVQ,                ///< N                       Twin Vector Quantizer
    AOT_CELP,                  ///< N                       Code Excited Linear Prediction
    AOT_HVXC,                  ///< N                       Harmonic Vector eXcitation Coding
    AOT_TTSI             = 12, ///< N                       Text-To-Speech Interface
    AOT_MAINSYNTH,             ///< N                       Main Synthesis
    AOT_WAVESYNTH,             ///< N                       Wavetable Synthesis
    AOT_MIDI,                  ///< N                       General MIDI
    AOT_SAFX,                  ///< N                       Algorithmic Synthesis and Audio Effects
    AOT_ER_AAC_LC,             ///< N                       Error Resilient Low Complexity
    AOT_ER_AAC_LTP       = 19, ///< N                       Error Resilient Long Term Prediction
    AOT_ER_AAC_SCALABLE,       ///< N                       Error Resilient Scalable
    AOT_ER_TWINVQ,             ///< N                       Error Resilient Twin Vector Quantizer
    AOT_ER_BSAC,               ///< N                       Error Resilient Bit-Sliced Arithmetic Coding
    AOT_ER_AAC_LD,             ///< N                       Error Resilient Low Delay
    AOT_ER_CELP,               ///< N                       Error Resilient Code Excited Linear Prediction
    AOT_ER_HVXC,               ///< N                       Error Resilient Harmonic Vector eXcitation Coding
    AOT_ER_HILN,               ///< N                       Error Resilient Harmonic and Individual Lines plus Noise
    AOT_ER_PARAM,              ///< N                       Error Resilient Parametric
    AOT_SSC,                   ///< N                       SinuSoidal Coding
    AOT_PS,                    ///< N                       Parametric Stereo
    AOT_SURROUND,              ///< N                       MPEG Surround
    AOT_ESCAPE,                ///< Y                       Escape Value
    AOT_L1,                    ///< Y                       Layer 1
    AOT_L2,                    ///< Y                       Layer 2
    AOT_L3,                    ///< Y                       Layer 3
    AOT_DST,                   ///< N                       Direct Stream Transfer
    AOT_ALS,                   ///< Y                       Audio LosslesS
    AOT_SLS,                   ///< N                       Scalable LosslesS
    AOT_SLS_NON_CORE,          ///< N                       Scalable LosslesS (non core)
    AOT_ER_AAC_ELD,            ///< N                       Error Resilient Enhanced Low Delay
    AOT_SMR_SIMPLE,            ///< N                       Symbolic Music Representation Simple
    AOT_SMR_MAIN,              ///< N                       Symbolic Music Representation Main
    AOT_USAC_NOSBR,            ///< N                       Unified Speech and Audio Coding (no SBR)
    AOT_SAOC,                  ///< N                       Spatial Audio Object Coding
    AOT_LD_SURROUND,           ///< N                       Low Delay MPEG Surround
    AOT_USAC,                  ///< N                       Unified Speech and Audio Coding
};

static int avpriv_mpeg4audio_sample_rates[16] = {
    96000, 88200, 64000, 48000, 44100, 32000,
    24000, 22050, 16000, 12000, 11025, 8000, 7350
};

#define FF_SIZE_OF_CHANNELS 8
static unsigned char ff_mpeg4audio_channels[FF_SIZE_OF_CHANNELS] = {
    0, 1, 2, 3, 4, 5, 6, 8
};

static int get_object_type(CMmpBitExtractor &be)
{
    unsigned long pt;
    int object_type = be.Pop_BitCode(pt, 5); /*get_bits(gb, 5);*/
    if (object_type == AOT_ESCAPE)
        object_type = 32 + be.Pop_BitCode(pt, 5) /*get_bits(gb, 6)*/;
    return object_type;
}

static int get_sample_rate(CMmpBitExtractor &be, int *index)
{
    unsigned long pt;

    *index = be.Pop_BitCode(pt, 4) /*get_bits(gb, 4)*/;
    return *index == 0x0f ? be.Pop_BitCode(pt, 24)/*get_bits(gb, 24)*/ :
        avpriv_mpeg4audio_sample_rates[*index];
}

MMP_RESULT CMmpAACTool::mpeg4audio_get_config(int default_samplerate, int default_ch, unsigned char* extra_data, int extra_data_size, struct MmpMPEG4AudioConfig* c) {

    unsigned long pt;
    int i;
    CMmpBitExtractor be;
    const int sync_extension = 1;
    int sampling_index=0, chan_config=0, ext_sampling_index=0, ext_chan_config=0;
    int err = 0;
    
    memset(c, 0x00, sizeof(struct MmpMPEG4AudioConfig));
        
    if(extra_data_size == 0) {
        err++;
    }
    else {
        be.Start(extra_data, extra_data_size);

        /* set default samplerate index */
        if(err == 0) {
            sampling_index = 0;
            for(i = 0; i < 16; i++) {
                if( avpriv_mpeg4audio_sample_rates[i] == default_samplerate) {
                    sampling_index = i;
                    ext_sampling_index = i;
                    break;
                }
            }
            if(i == 16) {
                err++;
            }
        }
        
        if(err == 0) {
            c->object_type = get_object_type(be);
            c->sample_rate = get_sample_rate(be, &sampling_index);
            chan_config = be.Pop_BitCode(pt, 4) /*get_bits(&gb, 4)*/;
            if (chan_config < FF_SIZE_OF_CHANNELS) {
                c->channels = ff_mpeg4audio_channels[chan_config];
            }
            else {
                c->channels = 0;
                err++;
            }
            c->sbr = -1;
            c->ps  = -1;

            ext_chan_config = chan_config;
            c->ext_sample_rate = c->sample_rate;
            c->ext_channels = c->channels;
        }

        if(err == 0) {
            if (c->object_type == AOT_SBR || (c->object_type == AOT_PS &&
                // check for W6132 Annex YYYY draft MP3onMP4
                !(be.Query_BitCode(pt, 3)/*show_bits(&gb, 3)*/ & 0x03 && !( be.Query_BitCode(pt, 3)/*show_bits(&gb, 9)*/ & 0x3F)))) {

                if (c->object_type == AOT_PS)
                    c->ps = 1;
                c->ext_object_type = AOT_SBR;
                c->sbr = 1;
                c->ext_sample_rate = get_sample_rate(be, &ext_sampling_index);
                c->object_type = get_object_type(be);
                if (c->object_type == AOT_ER_BSAC) {
                    ext_chan_config = be.Pop_BitCode(pt, 4) /*get_bits(&gb, 4)*/;
                    if(ext_chan_config < FF_SIZE_OF_CHANNELS) {
                       c->ext_channels = ff_mpeg4audio_channels[ext_chan_config];
                    }
                }
            } 
            else {
                c->ext_object_type = AOT_NULL;
                c->ext_sample_rate = 0;
                c->ext_channels = 0;
            }
            //specific_config_bitindex = get_bits_count(&gb);
        }


        if(err == 0) {
            if (c->object_type == AOT_ALS) {
        #if 0
                skip_bits(&gb, 5);
                if (show_bits_long(&gb, 24) != MKBETAG('\0','A','L','S'))
                    skip_bits_long(&gb, 24);

                specific_config_bitindex = get_bits_count(&gb);

                if (parse_config_ALS(&gb, c))
                    return -1;
        #else
                err++;
        #endif
            }
        }


        if(err == 0) {
            if (c->ext_object_type != AOT_SBR && sync_extension) {
        
                while (  be.GetRemainBitSize() /*get_bits_left(&gb)*/ > 15) {
                    if ( be.Query_BitCode(pt, 11) /*show_bits(&gb, 11)*/ == 0x2b7) { // sync extension
                        be.Pop_BitCode(pt, 11) /*get_bits(&gb, 11)*/;
                        c->ext_object_type = get_object_type(be);
                        if (c->ext_object_type == AOT_SBR && (c->sbr = be.Pop_1Bit() /*get_bits1(&gb)*/) == 1) {
                            c->ext_sample_rate = get_sample_rate(be, &ext_sampling_index);
                            if (c->ext_sample_rate == c->sample_rate)
                                c->sbr = -1;
                        }
                        if ( be.GetRemainBitSize()/*get_bits_left(&gb)*/ > 11 && be.Pop_BitCode(pt, 11) /*get_bits(&gb, 11)*/ == 0x548)
                            c->ps = be.Pop_1Bit() /*get_bits1(&gb)*/;
                        break;
                    } else
                        be.Pop_1Bit() /*get_bits1(&gb)*/; // skip 1 bit
                }
            }
        }


        //PS requires SBR
        if(err == 0) {
            if (!c->sbr)
                c->ps = 0;
            //Limit implicit PS to the HE-AACv2 Profile
            if ((c->ps == -1 && c->object_type != AOT_AAC_LC) || (c->channels & ~0x01) )
                c->ps = 0;
        }

    }

    if(err != 0) {
        memset(c, 0x00, sizeof(struct MmpMPEG4AudioConfig));
        c->object_type = AOT_AAC_LC;
        c->sample_rate = default_samplerate;
        c->channels = default_ch;
        c->sbr = -1;
        c->ps  = -1;
    }


    return MMP_SUCCESS;
}

const char* CMmpAACTool::mpeg4audio_objtype_string(int object_type) {

    switch(object_type) {
        case AOT_NULL: return "AOT_NULL"; 
                                   // Support?                Name
        case AOT_AAC_MAIN: return "AOT_AAC_MAIN";               ///< Y                       Main
        case AOT_AAC_LC: return "AOT_AAC_LC";                 ///< Y                       Low Complexity
        case AOT_AAC_SSR: return "AOT_AAC_SSR";                ///< N (code in SoC repo)    Scalable Sample Rate
        case AOT_AAC_LTP: return "AOT_AAC_LTP";                ///< Y                       Long Term Prediction
        case AOT_SBR: return "AOT_SBR";                    ///< Y                       Spectral Band Replication
        case AOT_AAC_SCALABLE: return "AOT_AAC_SCALABLE";           ///< N                       Scalable
        case AOT_TWINVQ: return "AOT_TWINVQ";                 ///< N                       Twin Vector Quantizer
        case AOT_CELP: return "AOT_CELP";                   ///< N                       Code Excited Linear Prediction
        case AOT_HVXC: return "AOT_HVXC";                   ///< N                       Harmonic Vector eXcitation Coding
        case AOT_TTSI: return "AOT_TTSI"; //             = 12, ///< N                       Text-To-Speech Interface
        case AOT_MAINSYNTH: return "AOT_MAINSYNTH";              ///< N                       Main Synthesis
        case AOT_WAVESYNTH: return "AOT_WAVESYNTH";              ///< N                       Wavetable Synthesis
        case AOT_MIDI: return "AOT_MIDI";                   ///< N                       General MIDI
        case AOT_SAFX: return "AOT_SAFX";                   ///< N                       Algorithmic Synthesis and Audio Effects
        case AOT_ER_AAC_LC: return "AOT_ER_AAC_LC";              ///< N                       Error Resilient Low Complexity
        case AOT_ER_AAC_LTP: return "AOT_ER_AAC_LTP"; //       = 19, ///< N                       Error Resilient Long Term Prediction
        case AOT_ER_AAC_SCALABLE: return "AOT_ER_AAC_SCALABLE";        ///< N                       Error Resilient Scalable
        case AOT_ER_TWINVQ: return "AOT_ER_TWINVQ";              ///< N                       Error Resilient Twin Vector Quantizer
        case AOT_ER_BSAC: return "AOT_ER_BSAC";                ///< N                       Error Resilient Bit-Sliced Arithmetic Coding
        case AOT_ER_AAC_LD: return "AOT_ER_AAC_LD";              ///< N                       Error Resilient Low Delay
        case AOT_ER_CELP: return "AOT_ER_CELP";                ///< N                       Error Resilient Code Excited Linear Prediction
        case AOT_ER_HVXC: return "AOT_ER_HVXC";                ///< N                       Error Resilient Harmonic Vector eXcitation Coding
        case AOT_ER_HILN: return "AOT_ER_HILN";                ///< N                       Error Resilient Harmonic and Individual Lines plus Noise
        case AOT_ER_PARAM: return "AOT_ER_PARAM";               ///< N                       Error Resilient Parametric
        case AOT_SSC: return "AOT_SSC";                    ///< N                       SinuSoidal Coding
        case AOT_PS: return "AOT_PS";                     ///< N                       Parametric Stereo
        case AOT_SURROUND: return "AOT_SURROUND";               ///< N                       MPEG Surround
        case AOT_ESCAPE: return "AOT_ESCAPE";                 ///< Y                       Escape Value
        case AOT_L1: return "AOT_L1";                     ///< Y                       Layer 1
        case AOT_L2: return "AOT_L2";                     ///< Y                       Layer 2
        case AOT_L3: return "AOT_L3";                     ///< Y                       Layer 3
        case AOT_DST: return "AOT_DST";                    ///< N                       Direct Stream Transfer
        case AOT_ALS: return "AOT_ALS";                    ///< Y                       Audio LosslesS
        case AOT_SLS: return "AOT_SLS";                    ///< N                       Scalable LosslesS
        case AOT_SLS_NON_CORE: return "AOT_SLS_NON_CORE";           ///< N                       Scalable LosslesS (non core)
        case AOT_ER_AAC_ELD: return "AOT_ER_AAC_ELD";             ///< N                       Error Resilient Enhanced Low Delay
        case AOT_SMR_SIMPLE: return "AOT_SMR_SIMPLE";             ///< N                       Symbolic Music Representation Simple
        case AOT_SMR_MAIN: return "AOT_SMR_MAIN";               ///< N                       Symbolic Music Representation Main
        case AOT_USAC_NOSBR: return "AOT_USAC_NOSBR";             ///< N                       Unified Speech and Audio Coding (no SBR)
        case AOT_SAOC: return "AOT_SAOC";                   ///< N                       Spatial Audio Object Coding
        case AOT_LD_SURROUND: return "AOT_LD_SURROUND";            ///< N                       Low Delay MPEG Surround
        case AOT_USAC: return "AOT_USAC";                   ///< N                       Unified Speech and Audio Coding
    }

 
    return "AOT_Unknown"; 
}
