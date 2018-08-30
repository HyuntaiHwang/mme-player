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


#include "MmpPlayerCompress.hpp"
#include "MmpUtil.hpp"

#define __force
#define __bitwise
#include <sound/asound.h>
#include <sound/compress_offload.h>
#include "MmpAACTool.hpp"

#define FRAGMENT_SIZE 131072
#define FRAGMENT_COUNT 16

/////////////////////////////////////////////////////////////
//CMmpPlayerCompress Member Functions

CMmpPlayerCompress::CMmpPlayerCompress(CMmpPlayerCreateProp* pPlayerProp) : CMmpPlayer(pPlayerProp, "Audio")

,m_pDemuxer(NULL)
,m_compress(NULL)
,m_stream_buffer(NULL)
,m_stream_max_size(FRAGMENT_SIZE)


,m_p_buf_as(NULL)

#if (CMmpPlayerCompress_PCM_DUMP == 1)
,m_pcm_dump_hdl(NULL)
#endif

{
    memset(&m_config, 0x00, sizeof(m_config));
    memset(&m_codec, 0x00, sizeof(m_codec));

}

CMmpPlayerCompress::~CMmpPlayerCompress()
{
#if (CMmpPlayerCompress_PCM_DUMP == 1)
    pcm_dump_destroy(m_pcm_dump_hdl);
#endif

    
}

MMP_RESULT CMmpPlayerCompress::Open()
{
    MMP_RESULT mmpResult = MMP_SUCCESS;

    CMmpRendererCreateProp RendererProp;
    CMmpRendererCreateProp* pRendererProp=&RendererProp; 
    struct mmp_audio_format af;
    
    CMmpPlayer::Open();
        
    m_stream_buffer = new unsigned char[FRAGMENT_SIZE];
    m_stream_index = 0;
    
    /* alloc audio stream buffer */
    if(mmpResult == MMP_SUCCESS) {
        m_p_buf_as = mmp_buffer_mgr::get_instance()->alloc_media_audiostream((MMP_MEDIA_ID)this, mmp_buffer_audiostream::DEFAULT_MAX_STREAM_SIZE, mmp_buffer::HEAP);
        if(m_p_buf_as == NULL) {
            mmpResult = MMP_FAILURE;
            MMPDEBUGMSG(MMPZONE_ERROR, (TEXT(MMP_CTAG"FAIL: alloc audio stream"), MMP_CNAME));
        }
    }

        
    /* create demuxer */
    if(mmpResult == MMP_SUCCESS ) {
        m_pDemuxer = this->CreateDemuxer();
        if(m_pDemuxer == NULL) {
            mmpResult = MMP_FAILURE;
            MMPDEBUGMSG(MMPZONE_ERROR, (TEXT(MMP_CTAG"FAIL: CreateDemuxer"), MMP_CNAME));
        }
    }
    
    /* create compress handle  */
    if(mmpResult == MMP_SUCCESS ) {
    
        af = m_pDemuxer->get_audio_format();

        switch(af.fourcc) {
            case  MMP_FOURCC_AUDIO_MP3: 
                m_codec.id = SND_AUDIOCODEC_MP3; 
                m_codec.ch_in = af.channels;
	            m_codec.ch_out = af.channels;
                m_codec.sample_rate = af.sample_rate;
                m_codec.bit_rate = af.bitrate;
	            m_codec.rate_control = 0;
	            m_codec.profile = 0;
	            m_codec.level = 0;
	            m_codec.ch_mode = 0;
	            m_codec.format = 0;
                m_codec.align = af.block_align;
                break;

            case  MMP_FOURCC_AUDIO_WMA2: 
                m_codec.id = SND_AUDIOCODEC_WMA; 
                m_codec.ch_in = af.channels;
	            m_codec.ch_out = af.channels;
                m_codec.sample_rate = af.sample_rate;
                m_codec.bit_rate = af.bitrate;
	            m_codec.rate_control = 0;
	            m_codec.profile = af.codec_tag;
	            m_codec.ch_mode = 0;
	            m_codec.format = 0;
                m_codec.align = af.block_align;

                if(af.bits_per_sample == 16) {
                    m_codec.format = SNDRV_PCM_FORMAT_S16_LE;
                }
                else if(af.bits_per_sample == 24) {
                    m_codec.format = SNDRV_PCM_FORMAT_S24_LE;
                }
                else if(af.bits_per_sample == 32) {
                    m_codec.format = SNDRV_PCM_FORMAT_S32_LE;
                }
                else {
                    m_codec.format = SNDRV_PCM_FORMAT_S16_LE;
                }

                m_codec.level = af.extra_data_size;
                if(m_codec.level > 12) {
                    m_codec.level = 0;
                }
                if(m_codec.level > 0) {
                    memcpy(m_codec.reserved, af.extra_data, m_codec.level);
                }

                m_codec.options.wma.super_block_align = 0;
                if(af.extra_data_size >= 6) {
                    unsigned short* sp = (unsigned short*)af.extra_data;
                    m_codec.options.wma.super_block_align = sp[2];
                }
                	            
                break;

            case  MMP_FOURCC_AUDIO_FLAC: 
                m_codec.id = SND_AUDIOCODEC_FLAC; 
                m_codec.ch_in = af.channels;
	            m_codec.ch_out = af.channels;
                m_codec.sample_rate = af.sample_rate;
                m_codec.bit_rate = af.bitrate;
	            m_codec.rate_control = 0;
	            m_codec.profile = 0;
	            m_codec.level = 0;
	            m_codec.ch_mode = 0;
	            m_codec.format = 0;
                m_codec.align = af.block_align;
                break;

            case  MMP_FOURCC_AUDIO_AAC:
                {
                    struct MmpMPEG4AudioConfig mp4a_config;

                    memset(&mp4a_config, 0x00, sizeof(mp4a_config));

                    m_codec.id = SND_AUDIOCODEC_AAC; 
                    m_codec.ch_in = af.channels;
	                m_codec.ch_out = af.channels;
                    m_codec.sample_rate = af.sample_rate;
                    m_codec.bit_rate = af.bitrate;
	                m_codec.rate_control = 0;
	                m_codec.profile = 0;
	                m_codec.level = 0;
	                m_codec.ch_mode = 0;
	                m_codec.format = 0;
                    m_codec.align = af.block_align;

                    if(af.extra_data_size > 0) {

                        CMmpAACTool::mpeg4audio_get_config(af.sample_rate, af.channels, af.extra_data, af.extra_data_size, &mp4a_config);

                        MMPDEBUGMSG(1, (TEXT("AAC(Raw) object_type : %d(%s)"),  mp4a_config.object_type, CMmpAACTool::mpeg4audio_objtype_string(mp4a_config.object_type) ));
                        MMPDEBUGMSG(1, (TEXT("AAC(Raw) sample_rate : %d"),  mp4a_config.sample_rate));
                        MMPDEBUGMSG(1, (TEXT("AAC(Raw) channels : %d"),  mp4a_config.channels));
                        MMPDEBUGMSG(1, (TEXT("AAC(Raw) sbr : %d"),  mp4a_config.sbr));
                        MMPDEBUGMSG(1, (TEXT("AAC(Raw) ps : %d"),  mp4a_config.ps));
                        MMPDEBUGMSG(1, (TEXT("AAC(Raw) ext_object_type : %d(%s)"),  mp4a_config.ext_object_type, CMmpAACTool::mpeg4audio_objtype_string(mp4a_config.ext_object_type) ));
                        MMPDEBUGMSG(1, (TEXT("AAC(Raw) ext_sample_rate : %d"),  mp4a_config.ext_sample_rate));
                        MMPDEBUGMSG(1, (TEXT("AAC(Raw) ext_channels : %d"),  mp4a_config.ext_channels));
                        MMPDEBUGMSG(1, (TEXT("AAC(Raw) frame_length_short : %d"),  mp4a_config.frame_length_short));
                    }

                    memcpy(m_codec.options.generic.reserved, &mp4a_config, sizeof(mp4a_config) );

                }
                break;

            default: m_codec.id = -1;
        }
                
        if(m_codec.id == -1) {
            mmpResult = MMP_FAILURE;
        }
        else {
            
		    m_config.fragment_size = FRAGMENT_SIZE;
		    m_config.fragments = FRAGMENT_COUNT;
	        m_config.codec = &m_codec;

            MMPDEBUGMSG(1, (TEXT("[CMmpPlayerCompress::Open] TRY : compress_open ")));
            m_compress = compress_open(0 /*card*/, 0 /*device*/, COMPRESS_IN, &m_config);
            if(m_compress == NULL) {
                mmpResult = MMP_FAILURE;
                MMPDEBUGMSG(1, (TEXT("[CMmpPlayerCompress::Open] FAIL : compress_open ")));
            }
            else {

                MMPDEBUGMSG(1, (TEXT("[CMmpPlayerCompress::Open] config.fragment_size=%d config.fragments=%d "), m_config.fragment_size, m_config.fragments ));
            }

	
        }
    }
        
#if (CMmpPlayerCompress_PCM_DUMP == 1)
    if(mmpResult == MMP_SUCCESS ) {
        m_pcm_dump_hdl = pcm_dump_write_create(CMmpPlayerCompress_PCM_FILENAME, m_pRendererAudio->get_sr(), m_pRendererAudio->get_ch(), m_pRendererAudio->get_bps() );
    }
#endif

    return mmpResult;
}


MMP_RESULT CMmpPlayerCompress::Close()
{
    CMmpPlayer::Close();

    if(m_compress != NULL) {
        compress_close(m_compress);
        m_compress = NULL;
    }

    if(m_pDemuxer != NULL) {
        CMmpDemuxer::DestroyObject(m_pDemuxer);  
        m_pDemuxer = NULL;
    }
    
    if(m_p_buf_as != NULL) { 
        mmp_buffer_mgr::get_instance()->free_media_buffer(m_p_buf_as);
        m_p_buf_as = NULL;
    }

    if(m_stream_buffer != NULL) {
        delete [] m_stream_buffer;
        m_stream_buffer = NULL;
    }
    
    return MMP_SUCCESS;
}

/*

[tinycompress::compress_open] config->fragment_size=524288 config->fragments=4 
[external/tinycompress/compress.c compress_nonblock] ln=637 
[external/tinycompress/compress.c compress_set_gapless_metadata] ln=578 
[external/tinycompress/compress.c compress_write] ln=370 size=499879 
[external/tinycompress/compress.c compress_start] ln=488 
[external/tinycompress/compress.c compress_next_track05.456001] partiail drainnig
[externalnal/tinycompress/compress.c compress_partial_drain] ln=543 
[external/tinycompress/compress.c compress_get_tstamp] ln=348 
[external/tinycompress/compress.c compress_get_tstamp] ln=348 
[external/tinycompress/compress.c compress_get_tstamp] ln=348 
[external/tinycompress/compress.c compress_get_tstamp] ln=348 
[external/tinycompress/compress.c compress_get_tstamp] ln=348 

int compress_get_tstamp(struct compress *compress,
			unsigned long *samples, unsigned int *sampling_rate)
*/
int CMmpPlayerCompress::compress_write_mme(struct compress *compress, const void *buf, unsigned int size) {

    int ret, cpsz, comp_wr_sz;

    if( (m_stream_index+size) <= m_stream_max_size) {
        memcpy(&m_stream_buffer[m_stream_index], buf, size);
        ret = size;
        m_stream_index += size;
    }
    else {

        cpsz = m_stream_max_size - m_stream_index;
        memcpy(&m_stream_buffer[m_stream_index], buf, cpsz);

        ret = cpsz;
        m_stream_index += cpsz;
        
        comp_wr_sz = compress_write(compress, m_stream_buffer, m_stream_index);
        m_stream_index = 0;
    }

    //MMPDEBUGMSG(1, (TEXT("[CMmpPlayerCompress::compress_write_mme] ln=%d size=%d ret=%d "), __LINE__, size, ret ));
    return ret;
}

void CMmpPlayerCompress::Service()
{
    MMP_RESULT mmpResult;
    MMP_U32 start_tick, before_tick, cur_tick;
    struct compr_gapless_mdata meta;
    int compr_ret;
    
    CMmpDemuxer* pDemuxer = m_pDemuxer;
        
    MMP_U32 frame_count = 0, packet_count=0, packet_sub_index = 0;
    MMP_U32 render_size = 0, render_sub_size = 0;

    MMP_TICKS packet_pts=0, last_render_pts=0;
    
    MMP_U8* aubuf;
    MMP_S32 ausize;
    MMP_S32 wrsz, wrsz1;
    MMP_U32 play_dur;

    MMP_TICKS pts;
     
    /* set nonblocking */
    compress_nonblock(m_compress, 1);
    //compress_nonblock(m_compress, 0);
    
    meta.encoder_padding = 0;
    meta.encoder_delay = 0;
    compr_ret = compress_set_gapless_metadata(m_compress, &meta);
    if(compr_ret != 0) {
        MMPDEBUGMSG(1, (TEXT("[CMmpPlayerCompress::Service] FAIL: compress_set_gapless_metadata ")));
        return;
    }
    else {
        MMPDEBUGMSG(1, (TEXT("[CMmpPlayerCompress::Service] SUCCESS: compress_set_gapless_metadata ")));
    }

    /*first feeding */
    wrsz1 = 0;
    while(m_bServiceRun == MMP_TRUE) {
        mmpResult = pDemuxer->GetNextAudioData(m_p_buf_as);
        if(mmpResult == MMP_SUCCESS) {

            aubuf = (MMP_U8*)m_p_buf_as->get_buf_vir_addr();
            ausize = m_p_buf_as->get_stream_real_size();

            wrsz = compress_write_mme(m_compress, aubuf, ausize);
            if(wrsz < ausize) {
                m_p_buf_as->inc_stream_offset(wrsz);
                break;
            }
            wrsz1 += wrsz;

            pts = m_p_buf_as->get_pts();

            //MMPDEBUGMSG(1, (TEXT("[CMmpPlayerCompress::Service] ln=%d wrsz1=%d pts=%lld "), __LINE__, wrsz1, pts));

            //if(pts > 3*1000*1000) {
             //   break;
            //}
        }
        else {

            if(m_stream_index > 0) {
                //compress_write(m_compress, m_stream_buffer, m_stream_max_size);
                compress_write(m_compress, m_stream_buffer, m_stream_index);
                m_stream_index = 0;
            }

            break;
        }
    }
    

    compress_start(m_compress);
    
    compr_ret = compress_next_track(m_compress);
    if(compr_ret != 0) {
        MMPDEBUGMSG(1, (TEXT("[CMmpPlayerCompress::Service] FAIL: compress_next_track ")));
        return;
    }

    compr_ret = compress_partial_drain(m_compress);
    if(compr_ret != 0) {
        MMPDEBUGMSG(1, (TEXT("[CMmpPlayerCompress::Service] FAIL: compress_partial_drain ")));
        return;
    }
    else {
        MMPDEBUGMSG(1, (TEXT("[CMmpPlayerCompress::Service] SUCCESS: compress_partial_drain ")));
    }

    unsigned long samples_before = 0;;
    unsigned long samples;
    unsigned int sample_rate;

    start_tick = CMmpUtil::GetTickCount();
    before_tick = start_tick;

    while(m_bServiceRun == MMP_TRUE) { 

        while(m_p_buf_as->get_stream_real_size() > 0) {

            aubuf = (MMP_U8*)m_p_buf_as->get_stream_real_addr();
            ausize = m_p_buf_as->get_stream_real_size();
            if(ausize > 0) {
                wrsz = compress_write_mme(m_compress, aubuf, ausize);
                if(wrsz > 0) {
                    m_p_buf_as->inc_stream_offset(wrsz);
                }
            }
        }

        pDemuxer->GetNextAudioData(m_p_buf_as);

        cur_tick = CMmpUtil::GetTickCount();
        if( (cur_tick - before_tick) > 1000) {

            compress_get_tstamp(m_compress, &samples, &sample_rate);

            play_dur = samples/sample_rate;

            MMPDEBUGMSG(1, (TEXT("[CMmpPlayerCompress::Service] %d.  samples=(%d %d) sample_rate=%d "), (cur_tick-start_tick)/1000 , play_dur, samples, sample_rate));

            before_tick = cur_tick;
            samples_before = samples;
        }
        CMmpUtil::Sleep(1);
    }

    
    //compress_drain(m_compress);
    compress_stop(m_compress);    
    CMmpUtil::Sleep(100);
 

}
