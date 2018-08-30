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

#include "MmpMpegAudioTool.hpp"
#include "MmpBitExtractor.hpp"

#define MPA_STEREO  0
#define MPA_JSTEREO 1
#define MPA_DUAL    2
#define MPA_MONO    3


static const unsigned short s_ff_mpa_bitrate_tab[2][3][15] = 
{
    { {0, 32, 64, 96, 128, 160, 192, 224, 256, 288, 320, 352, 384, 416, 448 },
      {0, 32, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320, 384 },
      {0, 32, 40, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320 } },
    { {0, 32, 48, 56, 64, 80, 96, 112, 128, 144, 160, 176, 192, 224, 256},
      {0, 8, 16, 24, 32, 40, 48, 56, 64, 80, 96, 112, 128, 144, 160},
      {0, 8, 16, 24, 32, 40, 48, 56, 64, 80, 96, 112, 128, 144, 160}
    }
};


static const unsigned short s_ff_mpa_freq_tab[3] = { 44100, 48000, 32000 };


MMP_BOOL CMmpMpegAudioTool::CheckHeader(unsigned int header)
{
    /* header */
    if ((header & 0xffe00000) != 0xffe00000)
        return MMP_FALSE;
    /* layer check */
    if ((header & (3<<17)) == 0)
        return MMP_FALSE;
    /* bit rate */
    if ((header & (0xf<<12)) == 0xf<<12)
        return MMP_FALSE;
    /* frequency */
    if ((header & (3<<10)) == 3<<10)
        return MMP_FALSE;

    return MMP_TRUE;
}

MMP_BOOL CMmpMpegAudioTool::DecodeHeader(unsigned int header, WAVEFORMATEX* pwf)
{
    int sample_rate, frame_size, mpeg25, padding;
    int sample_rate_index, bitrate_index;
    int bit_rate;

    int lsf, layer, mode;

    if (header & (1<<20)) 
    {
        lsf = (header & (1<<19)) ? 0 : 1;
        mpeg25 = 0;
    } 
    else 
    {
        lsf = 1;
        mpeg25 = 1;
    }

    layer = 4 - ((header >> 17) & 3);

    /* extract frequency */
    sample_rate_index = (header >> 10) & 3;
    sample_rate = s_ff_mpa_freq_tab[sample_rate_index] >> (lsf + mpeg25);
    //sample_rate_index += 3 * (s->lsf + mpeg25);
    //s->sample_rate_index = sample_rate_index;
    //s->error_protection = ((header >> 16) & 1) ^ 1;
    pwf->nSamplesPerSec= sample_rate;

    bitrate_index = (header >> 12) & 0xf;
    padding = (header >> 9) & 1;
    //extension = (header >> 8) & 1;
    mode = (header >> 6) & 3;
    //s->mode_ext = (header >> 4) & 3;
    //copyright = (header >> 3) & 1;
    //original = (header >> 2) & 1;
    //emphasis = header & 3;

    if (mode == MPA_MONO)
        pwf->nChannels=1;//s->nb_channels = 1;
    else
        pwf->nChannels=2;//s->nb_channels = 2;

    if (bitrate_index != 0) 
    {
        frame_size = s_ff_mpa_bitrate_tab[lsf][layer - 1][bitrate_index];
        bit_rate = frame_size * 1000;

        switch(layer) 
        {
            case 1:
                frame_size = (frame_size * 12000) / sample_rate;
                frame_size = (frame_size + padding) * 4;
                break;
            case 2:
                frame_size = (frame_size * 144000) / sample_rate;
                frame_size += padding;
                break;
            default:
            case 3:
                frame_size = (frame_size * 144000) / (sample_rate << lsf);
                frame_size += padding;
                break;
        }

        //s->frame_size = frame_size;
        pwf->nAvgBytesPerSec=frame_size;
    } 
    else 
    {
        /* if no frame size computed, signal it */
        return MMP_FALSE;//1;
    }

#if 0//defined(DEBUG)
    dprintf(s->avctx, "layer%d, %d Hz, %d kbits/s, ",
           s->layer, s->sample_rate, s->bit_rate);
    if (s->nb_channels == 2) {
        if (s->layer == 3) {
            if (s->mode_ext & MODE_EXT_MS_STEREO)
                dprintf(s->avctx, "ms-");
            if (s->mode_ext & MODE_EXT_I_STEREO)
                dprintf(s->avctx, "i-");
        }
        dprintf(s->avctx, "stereo");
    } else {
        dprintf(s->avctx, "mono");
    }
    dprintf(s->avctx, "\n");
#endif

    return MMP_TRUE;
}


