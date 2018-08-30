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


#include "MmpPlayerService.hpp"
#include "MmpUtil.hpp"


CMmpPlayerService::CMmpPlayerService(MMP_CHAR* player_name) :
m_bServiceRun(MMP_FALSE)
,m_p_task(NULL)
{
    strcpy(this->m_player_name, player_name);
    this->mon_reset();

    memset(&m_mon, 0x00, sizeof(m_mon));
}

CMmpPlayerService::~CMmpPlayerService() {

}

MMP_RESULT CMmpPlayerService::Open() {

    return MMP_SUCCESS;
}

MMP_RESULT CMmpPlayerService::Close() {

    return MMP_SUCCESS;
}

/* monitor function */
void CMmpPlayerService::mon_reset() {
    memset(&m_mon, 0x00, sizeof(m_mon));
}

void CMmpPlayerService::mon_reset_every_1sec() {
    
    m_mon.vdec.fps = 0;
    m_mon.vdec.dur_sum = 0;
    m_mon.vdec.dur_sum_dec = 0;
    
    m_mon.vren.fps = 0;
    m_mon.vren.dur_sum = 0;
    
}

void CMmpPlayerService::mon_vdec_begin() {
    m_mon.vdec.t.start_tick = CMmpUtil::GetTickCount();
}

void CMmpPlayerService::mon_vdec_end(CMmpDecoderVideo* pDecoderVideo, class mmp_buffer_videostream *p_buf_vs, class mmp_buffer_videoframe* p_buf_vf) {

    m_mon.vdec.t.end_tick = CMmpUtil::GetTickCount();
    if(p_buf_vf != NULL) {
        m_mon.vdec.pic_width = p_buf_vf->get_pic_width();
        m_mon.vdec.pic_height = p_buf_vf->get_pic_height();
    
        m_mon.vdec.t.pts = p_buf_vf->get_pts();
        m_mon.vdec.fps++;
        m_mon.vdec.fps_t++;
        m_mon.vdec.dur_sum +=  m_mon.vdec.t.end_tick-m_mon.vdec.t.start_tick;
        m_mon.vdec.dur_sum_dec += p_buf_vf->get_coding_dur();
        m_mon.vdec.dur_sum_dec_t += p_buf_vf->get_postproc_dur();//>get_coding_dur();
        m_mon.vdec.stream_size_t += (MMP_S64)p_buf_vs->get_stream_size();
        if(pDecoderVideo != NULL) {
            m_mon.vdec.fourcc_in = pDecoderVideo->get_fourcc_in();
            m_mon.vdec.fourcc_out = pDecoderVideo->get_fourcc_out();
        }
        else {
            m_mon.vdec.fourcc_in = MMP_FOURCC_VIDEO_UNKNOWN;
            m_mon.vdec.fourcc_out = MMP_FOURCC_VIDEO_UNKNOWN;
        }
    }
}

void CMmpPlayerService::mon_vren_begin() {
    m_mon.vren.t.start_tick = CMmpUtil::GetTickCount();
}

void CMmpPlayerService::mon_vren_end(class mmp_buffer_videoframe* p_buf_vf) {
    
    m_mon.vren.t.end_tick = CMmpUtil::GetTickCount();
    if(p_buf_vf!=NULL) {

        m_mon.vren.pic_width = p_buf_vf->get_pic_width();
        m_mon.vren.pic_height = p_buf_vf->get_pic_height();
    
        m_mon.vren.t.pts = p_buf_vf->get_pts();
        m_mon.vren.fps++;
        m_mon.vren.dur_sum +=  m_mon.vren.t.end_tick-m_mon.vren.t.start_tick;
    }
}

void CMmpPlayerService::mon_print_every_1sec_video(const MMP_CHAR* codec_name) {
    mon_print_every_1sec(codec_name, MMP_FALSE, MMP_TRUE);
}

void CMmpPlayerService::mon_print_every_1sec_audio(const MMP_CHAR* codec_name) {
    mon_print_every_1sec(codec_name, MMP_TRUE, MMP_FALSE);
}
    
void CMmpPlayerService::mon_print_every_1sec(const MMP_CHAR* codec_name, MMP_BOOL is_audio, MMP_BOOL is_video) {

    MMP_CHAR szmsg[512];
    MMP_CHAR szbuf[128];
    struct mmp_system_meminfo meminfo;
    MMP_S64 avg_bitrate;
    MMP_U32 avg_dur, avg_fps;

    szmsg[0] = (MMP_CHAR)NULL;

    CMmpUtil::system_meminfo(&meminfo);

    if(is_audio) {
        
    }

    
    if(is_video) {

        avg_dur = (m_mon.vdec.fps_t==0)?0:m_mon.vdec.dur_sum_dec_t/m_mon.vdec.fps_t;
        avg_fps = (avg_dur==0)?0:(1000/avg_dur);
        avg_bitrate = (m_mon.vdec.fps_t==0)?0:(MMP_S32)(m_mon.vdec.stream_size_t*8L*24L/(MMP_S64)m_mon.vdec.fps_t);

        if (m_mon.vdec.fps > 20) {
            system("reboot");
        }

        sprintf(szbuf, "VD=(%dx%d %c%c%c%c %d dur=(%d %d %d) %dkbps pts=%d) ", 
                         m_mon.vdec.pic_width, m_mon.vdec.pic_height,
                         MMPGETFOURCCARG(m_mon.vdec.fourcc_in),
                         m_mon.vdec.fps, 
                         //m_mon.vdec.t.end_tick - m_mon.vdec.t.start_tick,
                         (m_mon.vdec.fps==0)?0:m_mon.vdec.dur_sum/m_mon.vdec.fps,
                         (m_mon.vdec.fps==0)?0:m_mon.vdec.dur_sum_dec/m_mon.vdec.fps,
                         avg_dur,
                         (MMP_S32)avg_bitrate/1000,
                         (MMP_U32)(m_mon.vdec.t.pts/1000)
                         );
        strcat(szmsg, szbuf);

        sprintf(szbuf, "VR=(%d dur=%d pts=%d) ", 
                         m_mon.vren.fps, 
                         m_mon.vren.t.end_tick - m_mon.vren.t.start_tick,
                         (MMP_U32)(m_mon.vren.t.pts/1000)
                         );
        strcat(szmsg, szbuf);
    }

    MMPDEBUGMSG(1, (TEXT("[Player-%s] %s %s  MemFree(%d/%d MByte)"), this->GetPlayerName(), codec_name, szmsg,  meminfo.MemFree/1024, meminfo.MemTotal/1024 ));
}


MMP_S64 CMmpPlayerService::GetDuration(MMP_S32 *hour, MMP_S32* min, MMP_S32* sec, MMP_S32* msec) {

    MMP_S64 dur64;
    MMP_U32 dur32;

    dur64 = this->GetDuration();
    dur32 = (MMP_U32)(dur64/1000LL);

    if(hour) *hour = CMmpUtil::Time_GetHour(dur32);
    if(min) *min = CMmpUtil::Time_GetMin(dur32);
    if(sec) *sec = CMmpUtil::Time_GetSec(dur32);
    if(msec) *msec = CMmpUtil::Time_GetMileSec(dur32);

    return dur64;
}

MMP_S64 CMmpPlayerService::GetPlayPosition(MMP_S32 *hour, MMP_S32* min, MMP_S32* sec, MMP_S32* msec) {

    MMP_S64 pos64;
    MMP_U32 pos32;

    pos64 = this->GetPlayPosition();
    pos32 = (MMP_U32)(pos64/1000LL);

    if(hour) *hour = CMmpUtil::Time_GetHour(pos32);
    if(min) *min = CMmpUtil::Time_GetMin(pos32);
    if(sec) *sec = CMmpUtil::Time_GetSec(pos32);
    if(msec) *msec = CMmpUtil::Time_GetMileSec(pos32);

    return pos64;
}

void CMmpPlayerService::ServiceStub(void* parm)
{
    CMmpPlayerService* pObj=(CMmpPlayerService*)parm;
    pObj->Service();
}

void CMmpPlayerService::ServiceStub_VideoDec(void* parm) {
    CMmpPlayerService* pObj=(CMmpPlayerService*)parm;
    pObj->Service_VideoDec();
}
 
void CMmpPlayerService::ServiceStub_AudioDec(void* parm) {
    CMmpPlayerService* pObj=(CMmpPlayerService*)parm;
    pObj->Service_AudioDec();
}

void CMmpPlayerService::ServiceStub_AudioRender(void* parm) {
    CMmpPlayerService* pObj=(CMmpPlayerService*)parm;
    pObj->Service_AudioRender();
}

void CMmpPlayerService::ServiceStub_VideoRender(void* parm) {
    CMmpPlayerService* pObj=(CMmpPlayerService*)parm;
    pObj->Service_VideoRender();
}

MMP_RESULT CMmpPlayerService::PlayStart()
{
    MMP_RESULT mmpResult = MMP_FAILURE;

    if(m_p_task == NULL)
    {
        m_bServiceRun=MMP_TRUE;
        m_p_task = mmp_oal_task::create_object(CMmpPlayerService::ServiceStub, this);
        if(m_p_task!=NULL)
        {
            mmpResult = MMP_SUCCESS;
        }
    }

    return mmpResult;
}

MMP_RESULT CMmpPlayerService::PlayStop()
{
    if(m_p_task != NULL)
    {
        m_bServiceRun=MMP_FALSE;
        mmp_oal_task::destroy_object(m_p_task);
        m_p_task=NULL;
    }

    return MMP_SUCCESS;
}


void CMmpPlayerService::Service_Audio_Only(CMmpDemuxer* pDemuxer, CMmpDecoderAudio* pDecoderAudio, CMmpRenderer* pRendererAudio)
{
#if 0
    MMP_U32 start_tick, before_tick, cur_tick;
    MMP_RESULT mmpResult;
    CMmpMediaSample *pMediaSample = &m_MediaSampleObj;
    CMmpMediaSampleDecodeResult* pDecResult = &m_DecResultObj;
    
    MMP_U32 stream_buf_size;
    MMP_U32 stream_buf_max_size = 1024*1024;
    MMP_U8* stream_buf, *pbuf;

    MMP_U8* pcm_buf;
    MMP_U32 pcm_buf_max_size = 1024*1024;

    MMP_U32 frame_count = 0;
    MMP_U32 render_size = 0;

    MMP_S64 packet_pts;

    stream_buf = (MMP_U8*)malloc(stream_buf_max_size);
    if(stream_buf == NULL) {
        m_bServiceRun = MMP_FALSE;
    }

    pcm_buf = (MMP_U8*)malloc(pcm_buf_max_size);
    if(pcm_buf == NULL) {
        m_bServiceRun = MMP_FALSE;
    }
    
    start_tick = CMmpUtil::GetTickCount();
    before_tick = start_tick;
    while(m_bServiceRun == MMP_TRUE) {
    
        mmpResult = pDemuxer->GetNextAudioData(stream_buf, stream_buf_max_size, &stream_buf_size, &packet_pts);
        if(mmpResult == MMP_SUCCESS) {
        
            pbuf = stream_buf;
            while(stream_buf_size > 0) {

                pMediaSample->pAu = pbuf;
                pMediaSample->uiAuSize = stream_buf_size;
                pMediaSample->uiSampleNumber = 0;
                pMediaSample->uiTimeStamp = 0;
                pMediaSample->uiFlag = 0;

                pDecResult->uiDecodedBufferLogAddr[MMP_DECODED_BUF_PCM] = (MMP_U32)pcm_buf;
                pDecResult->uiDecodedBufferMaxSize = pcm_buf_max_size;
                        
                pDecoderAudio->DecodeAu(pMediaSample, pDecResult);
                if(pDecResult->uiDecodedSize > 0) {
                    pRendererAudio->RenderPCM(pcm_buf, pDecResult->uiDecodedSize);

                    frame_count++;
                    render_size += pDecResult->uiDecodedSize;

                    CMmpUtil::Sleep(1);
                }

                if(stream_buf_size >= pDecResult->uiAuUsedByte) {
                    stream_buf_size -= pDecResult->uiAuUsedByte;
                    pbuf += pDecResult->uiAuUsedByte;
                }
                else {
                    stream_buf_size = 0;
                }
            }
        }

        CMmpUtil::Sleep(1);

        cur_tick = CMmpUtil::GetTickCount();
        if( (cur_tick - before_tick) > 1000) {
            MMPDEBUGMSG(1, (TEXT("[MmpPlayerService Audio] %d. frc=%d rnsz=%d "),
                           (cur_tick-start_tick)/1000,
                           frame_count, render_size
                ));

            before_tick = cur_tick;
            frame_count = 0;
            render_size = 0;
        }

    } /* endo fo while(m_bServiceRun == MMP_TRUE) { */

    if(stream_buf != NULL) {
        free(stream_buf);
    }

    if(pcm_buf != NULL) {
        free(pcm_buf);
    }
#endif
}


void CMmpPlayerService::Service_AV_Simple(CMmpDemuxer* pDemuxer, 
                           CMmpDecoderAudio* pDecoderAudio, CMmpDecoderVideo* pDecoderVideo,
                           CMmpRenderer* pRendererAudio, CMmpRenderer* pRendererVideo) {

#if 0
    MMP_RESULT mmpResult;
    CMmpMediaSample MediaSampleObj;
    CMmpMediaSampleDecodeResult DecResultObj;
    CMmpMediaSample *pMediaSample = &MediaSampleObj;
    CMmpMediaSampleDecodeResult* pDecResult = &DecResultObj;
    
    MMP_U32 stream_buf_max_size = 1024*1024;
    MMP_U8* stream_buf;

    MMP_U8* decoded_buf;
    MMP_U32 decoded_buf_max_size = 1024*1024*3;
    MMP_U8 *Y= NULL, *U= NULL, *V= NULL;

    MMP_U32 frame_count = 0;
    MMP_U32 buffer_width, buffer_height;

    memset(pMediaSample, 0x00, sizeof(CMmpMediaSample));
    memset(pDecResult, 0x00, sizeof(CMmpMediaSampleDecodeResult));

    buffer_width = MMP_BYTE_ALIGN(pDemuxer->GetVideoPicWidth(), 16);
    buffer_height = MMP_BYTE_ALIGN(pDemuxer->GetVideoPicHeight(), 16);

    stream_buf = (MMP_U8*)malloc(stream_buf_max_size);
    if(stream_buf == NULL) {
        m_bServiceRun = MMP_FALSE;
    }

    decoded_buf = (MMP_U8*)malloc(decoded_buf_max_size);
    if(decoded_buf == NULL) {
        m_bServiceRun = MMP_FALSE;
    }
    else {
        Y = decoded_buf;
        U = Y + buffer_width*buffer_height;
        V = U + (buffer_width*buffer_height)/4;
    }
    
    while(m_bServiceRun == MMP_TRUE) {
    
        pMediaSample->pAu = stream_buf;
        pMediaSample->uiAuMaxSize = stream_buf_max_size;

        mmpResult = pDemuxer->GetNextData(pMediaSample);
        if(mmpResult == MMP_SUCCESS) {
            
            pDecResult->uiDecodedSize = 0;

            if(pMediaSample->uiMediaType == MMP_MEDIATYPE_AUDIO) {
                pDecResult->uiDecodedBufferLogAddr[MMP_DECODED_BUF_PCM] = (MMP_U32)decoded_buf;
                pDecResult->uiDecodedBufferMaxSize = decoded_buf_max_size;
                pDecoderAudio->DecodeAu(pMediaSample, pDecResult);
            }
            else if(pMediaSample->uiMediaType == MMP_MEDIATYPE_VIDEO) {
                pDecResult->uiDecodedBufferLogAddr[MMP_DECODED_BUF_Y] = (MMP_U32)Y;
                pDecResult->uiDecodedBufferLogAddr[MMP_DECODED_BUF_U] = (MMP_U32)U;
                pDecResult->uiDecodedBufferLogAddr[MMP_DECODED_BUF_V] = (MMP_U32)V;
                pDecResult->uiDecodedBufferStride[MMP_DECODED_BUF_Y] = buffer_width;
                pDecResult->uiDecodedBufferStride[MMP_DECODED_BUF_U] = buffer_width/2;
                pDecResult->uiDecodedBufferStride[MMP_DECODED_BUF_V] = buffer_width/2;
                pDecResult->uiDecodedBufferAlignHeight[MMP_DECODED_BUF_Y] = buffer_height;
                pDecResult->uiDecodedBufferAlignHeight[MMP_DECODED_BUF_U] = buffer_height/2;
                pDecResult->uiDecodedBufferAlignHeight[MMP_DECODED_BUF_V] = buffer_height/2;
                pDecResult->uiDecodedBufferMaxSize = buffer_width*buffer_height*3/2;
            
                pDecoderVideo->DecodeAu(pMediaSample, pDecResult);
            }
      
#if 0
            if(pDecResult->uiDecodedSize > 0) {
                if(pMediaSample->uiMediaType == MMP_MEDIATYPE_AUDIO) {
                    pRendererAudio->RenderPCM(decoded_buf, pDecResult->uiDecodedSize);
                }
                else if(pMediaSample->uiMediaType == MMP_MEDIATYPE_VIDEO) {
                    pRendererVideo->RenderYUV420Planar(Y, U, V, buffer_width, buffer_height);
                }
            }
#endif
        }

        frame_count++;

        //if(frame_count > 120) {
        //    break;
        //}

        CMmpUtil::Sleep(1);
    } /* endo fo while(m_bServiceRun == MMP_TRUE) { */

    if(stream_buf != NULL) {
        free(stream_buf);
    }

    if(decoded_buf != NULL) {
        free(decoded_buf);
    }
#endif
}

