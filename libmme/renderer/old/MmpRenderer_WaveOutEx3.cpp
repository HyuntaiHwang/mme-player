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


#include "MmpRenderer_WaveOutEx3.hpp"
#include "MmpUtil.hpp"
#include "mmp_audio_tool.hpp"

/////////////////////////////////////////////////////////////
//CMmpRenderer_WaveOutEx3 Member Functions


void CALLBACK CMmpRenderer_WaveOutEx3::waveOutProcStub(HWAVEOUT WaveOutHandle, UINT uMsg, long dwInstance, DWORD dwParam1, DWORD dwParam2 ) 
{
    CMmpRenderer_WaveOutEx3* pObj=(CMmpRenderer_WaveOutEx3*)dwInstance;
    pObj->waveOutProc(WaveOutHandle, uMsg, dwInstance, dwParam1, dwParam2 );
}

CMmpRenderer_WaveOutEx3::CMmpRenderer_WaveOutEx3(CMmpRendererAudio::create_config *p_create_config) :  CMmpRendererAudio(p_create_config)
,m_WaveOutHandle(NULL)
,m_p_sync_cs(NULL)
,m_waveBuffer(NULL)

,m_storage_buffer(NULL)
,m_storage_buffer_size(0)
{
    
    
}

CMmpRenderer_WaveOutEx3::~CMmpRenderer_WaveOutEx3()
{

}

MMP_RESULT CMmpRenderer_WaveOutEx3::Open()
{
    MMP_RESULT mmpResult;
    MMRESULT  mmresult;
    int i;

    m_p_sync_cs = mmp_oal_mutex::create_object();
    if(m_p_sync_cs==NULL)  {
        return MMP_FAILURE;
    }
    
    m_p_sync_cond = mmp_oal_cond::create_object();
    if(m_p_sync_cond==NULL)  {
        return MMP_FAILURE;
    }
    
    mmpResult=CMmpRenderer::Open();
    if(mmpResult!=MMP_SUCCESS)
    {
        return mmpResult;
    }

    WAVEFORMATEX  waveFormat;	
	waveFormat.wFormatTag = WAVE_FORMAT_PCM;
    waveFormat.nChannels = (WORD)this->get_ch();
    if(waveFormat.nChannels==0 || waveFormat.nChannels>2 )
        waveFormat.nChannels=2;
    waveFormat.nSamplesPerSec = this->get_sr();
    waveFormat.wBitsPerSample = (WORD)this->get_bps();
    if(waveFormat.wBitsPerSample==0)
        waveFormat.wBitsPerSample=16;
	waveFormat.nBlockAlign = waveFormat.nChannels * (waveFormat.wBitsPerSample/8);//1; // waveFormat.nChannels * (waveFormat.wBitsPerSample/8);
	waveFormat.nAvgBytesPerSec = waveFormat.nSamplesPerSec * waveFormat.nBlockAlign;//44100; // waveFormat.nSamplesPerSec * waveFormat.nBlockAlign;
	waveFormat.cbSize = 0;	

    mmresult = waveOutOpen(NULL, WAVE_MAPPER, &waveFormat, NULL, NULL, WAVE_FORMAT_QUERY);
	if ( mmresult != MMSYSERR_NOERROR ) 
    {
        MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpRenderer_WaveOutEx3::Open] FAIL: waveOutOpen(QUERY) \n\r")));
		m_WaveOutHandle = 0;
		return MMP_FAILURE;
	}

	mmresult = waveOutOpen(&m_WaveOutHandle, WAVE_MAPPER, &waveFormat, (DWORD)(VOID*)CMmpRenderer_WaveOutEx3::waveOutProcStub, (DWORD_PTR)this, CALLBACK_FUNCTION);
	if ( mmresult != MMSYSERR_NOERROR ) 
    {
		m_WaveOutHandle = 0;
		return MMP_FAILURE;
	}

    CMmpWaveBuffer* pWaveBuffer;
    
    m_storage_dur_tick = (MMP_S64)CMmpWaveBuffer::WAVE_BUF_SIZE*1000000/(MMP_S64)(waveFormat.nSamplesPerSec * (waveFormat.wBitsPerSample/8) * waveFormat.nChannels);
    m_storage_buffer = new unsigned char[CMmpWaveBuffer::WAVE_BUF_SIZE*10];

    m_waveBuffer=new CMmpWaveBuffer[CMmpWaveBuffer::WAVE_BUF_COUNT];
    if(m_waveBuffer==NULL)
    {
        MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpRenderer_WaveOutEx3::Open] FAIL: new CMmpWaveBuffer[CMmpWaveBuffer::WAVE_BUF_COUNT] \n\r")));
		return MMP_FAILURE;
	}

    for(i=0;i<CMmpWaveBuffer::WAVE_BUF_COUNT;i++) {

        pWaveBuffer=&m_waveBuffer[i];
        pWaveBuffer->p_buf=new unsigned char[CMmpWaveBuffer::WAVE_BUF_SIZE];
        pWaveBuffer->id=i;
        pWaveBuffer->alloc_size=CMmpWaveBuffer::WAVE_BUF_SIZE;
        

        pWaveBuffer->wave_hdr.lpData =(LPSTR)pWaveBuffer->p_buf;
	    pWaveBuffer->wave_hdr.dwBufferLength = CMmpWaveBuffer::WAVE_BUF_SIZE;
	    pWaveBuffer->wave_hdr.dwFlags = 0L;    // start position
	    pWaveBuffer->wave_hdr.dwLoops = 0L;   // loop
	    pWaveBuffer->wave_hdr.dwBytesRecorded = CMmpWaveBuffer::WAVE_BUF_SIZE;
        pWaveBuffer->wave_hdr.dwUser=(DWORD)pWaveBuffer;

        waveOutPrepareHeader(m_WaveOutHandle, &pWaveBuffer->wave_hdr, sizeof(WAVEHDR));	
    }
    
    return MMP_SUCCESS;
}

MMP_RESULT CMmpRenderer_WaveOutEx3::Close()
{
    MMP_RESULT mmpResult;
    MMP_S32 i;
    CMmpWaveBuffer* pWaveBuffer;
    WAVEHDR* pWaveHdr;
    
    if(m_WaveOutHandle) 
    {
#if 1
        while(1) {
            for(i=0;i<CMmpWaveBuffer::WAVE_BUF_COUNT;i++) {
                pWaveBuffer=&m_waveBuffer[i];
                if(pWaveBuffer->free_tick == CMmpWaveBuffer::BUSY_TICK) {
                    break;
                }
            }
            if(i < CMmpWaveBuffer::WAVE_BUF_COUNT) {
                Sleep(10);
            }
            else {
                break;
            }
        }
#endif

        for(i=0;i<CMmpWaveBuffer::WAVE_BUF_COUNT;i++) {
            pWaveBuffer=&m_waveBuffer[i];
            pWaveHdr=&pWaveBuffer->wave_hdr;
            waveOutUnprepareHeader(m_WaveOutHandle, (LPWAVEHDR)pWaveHdr, sizeof(WAVEHDR) ); 
            delete [] pWaveBuffer->p_buf;
        }
                
		waveOutClose(m_WaveOutHandle);
		m_WaveOutHandle=NULL;
	}

    mmpResult=CMmpRenderer::Close();
    if(mmpResult!=MMP_SUCCESS)
    {
        return mmpResult;
    }

    if(m_p_sync_cs) {
        mmp_oal_mutex::destroy_object(m_p_sync_cs);
        m_p_sync_cs=NULL;
    }
    
    if(m_p_sync_cond) {
        mmp_oal_cond::destroy_object(m_p_sync_cond);
        m_p_sync_cond=NULL;
    }
    
    if(m_waveBuffer) {
        delete [] m_waveBuffer;
        m_waveBuffer=NULL;
    }

    if(m_storage_buffer)
    {
        delete [] m_storage_buffer;
        m_storage_buffer = NULL;
    }

    return MMP_SUCCESS;
}

void CMmpRenderer_WaveOutEx3::waveOutProc(HWAVEOUT WaveOutHandle, UINT uMsg, long dwInstance, DWORD dwParam1, DWORD dwParam2 ) 
{
    WAVEHDR* pWaveHdr;
    CMmpWaveBuffer* pMmpWaveBuffer;
    MMP_S64 pts;

    switch (uMsg) 
	{ 
	    case MM_WOM_DONE: 
		{ 			
            m_p_sync_cs->lock();
            pWaveHdr=(WAVEHDR*)dwParam1;
            pMmpWaveBuffer = (CMmpWaveBuffer*)(pWaveHdr->dwUser);
            pts = pMmpWaveBuffer->pts;
            this->set_real_render_pts(pts);
            
            pMmpWaveBuffer->fill_size = 0;
            pMmpWaveBuffer->pts = 0;
            pMmpWaveBuffer->free_tick = CMmpUtil::GetTickCount();
            m_p_sync_cs->unlock();

            m_p_sync_cond->signal();

            MMPDEBUGMSG(0, (TEXT("[WIM_DONE] id=%d pts=%d \n\r"), pMmpWaveBuffer->id, (MMP_U32)(pts/1000) ));
			break;

		}

    	case WIM_DATA: 
		{ 
            MMPDEBUGMSG(1, (TEXT("[WIM_DATA] \n\r")));
			break;
		}

        default:
            MMPDEBUGMSG(1, (TEXT("[MM_UNKNOWN MSG] %d 0x%x\n\r"), uMsg, uMsg));
            break;
	} // end of switch   

}

CMmpRenderer_WaveOutEx3::CMmpWaveBuffer* CMmpRenderer_WaveOutEx3::GetNextWaveBufferWithLock() {

    CMmpWaveBuffer* pWaveBuffer = NULL;
    CMmpWaveBuffer* pWaveBufferTemp = NULL;
    MMP_S32 i, ret_idx = -1;
    MMP_U32 fast_free_tick = CMmpWaveBuffer::BUSY_TICK;
    MMP_S32 free_buf_count = 0;

    MMPDEBUGMSG(0, (TEXT("[CMmpRenderer_WaveOutEx3::GetNextWaveBufferWithLock] ++++++++++ \n\r")));

    while(1) {

        ret_idx = -1;
        fast_free_tick = CMmpWaveBuffer::BUSY_TICK;

        m_p_sync_cs->lock();
        for(i = 0; i < CMmpWaveBuffer::WAVE_BUF_COUNT; i++) {
            pWaveBufferTemp = &m_waveBuffer[i];
            if(pWaveBufferTemp->free_tick != CMmpWaveBuffer::BUSY_TICK) {
                if(pWaveBufferTemp->free_tick < fast_free_tick) {
                    fast_free_tick = pWaveBufferTemp->free_tick;
                    ret_idx = i;
                }
                free_buf_count++;
            }
        }
        m_p_sync_cs->unlock();
 
        if(ret_idx  == -1) {
            MMPDEBUGMSG(0, (TEXT("[CMmpRenderer_WaveOutEx3::GetNextWaveBufferWithLock] Wait..")));
            m_p_sync_cond->wait(m_p_sync_cs);
        }
        else {
            pWaveBuffer = &m_waveBuffer[ret_idx];
            break;
        }
    }

    MMPDEBUGMSG(0, (TEXT("[CMmpRenderer_WaveOutEx3::GetNextWaveBufferWithLock] ---------- id=%d buf=%d/%d free_buf_count=%d \n\r"), 
                   pWaveBuffer->id, 
                   pWaveBuffer->fill_size,
                   pWaveBuffer->alloc_size,
                   free_buf_count));

    return pWaveBuffer;
}

#if 0
MMP_RESULT CMmpRenderer_WaveOutEx3::Render(class mmp_buffer_audioframe* p_buf_af) {

    MMP_RESULT mmpResult = MMP_SUCCESS;
    CMmpWaveBuffer* pWaveBuffer;
    
    MMP_U8* p_buf_src = (MMP_U8*)p_buf_af->get_buf_vir_addr();
    MMP_S32 src_remain_size, src_write_size;
    
    MMP_U8 *p_buf_dest;
    MMP_S32 sr, ch, bps;
    MMP_S64 pts_offset;

    MMP_S32 wavebuf_remain_byte;
    MMP_S32 i;

    sr = this->get_sr();
    ch = this->get_ch();
    bps = this->get_bps();
    src_remain_size = p_buf_af->get_data_size();
    
    while(src_remain_size > 0) {

        pWaveBuffer = this->GetNextWaveBufferWithLock();
        if(pWaveBuffer->fill_size ==  0) {
            i = p_buf_af->get_data_size() - src_remain_size;
            pts_offset = mmp_audio_tool::pcm_cal_dur_us(i, sr, ch, bps);
            pWaveBuffer->pts = p_buf_af->get_pts() + pts_offset;
        }

        wavebuf_remain_byte = pWaveBuffer->alloc_size - pWaveBuffer->fill_size;
        if(wavebuf_remain_byte > src_remain_size) {
            src_write_size = src_remain_size;
        }
        else {
            src_write_size = wavebuf_remain_byte;
        }

        p_buf_dest = &pWaveBuffer->p_buf[pWaveBuffer->fill_size];
        memcpy(p_buf_dest, (void*)p_buf_src, src_write_size);
        pWaveBuffer->fill_size += src_write_size;
        src_remain_size -= src_write_size;
        p_buf_src += src_remain_size;
            
        if(pWaveBuffer->fill_size >= pWaveBuffer->alloc_size) {
            pWaveBuffer->free_tick = CMmpWaveBuffer::BUSY_TICK;
            pts_offset = mmp_audio_tool::pcm_cal_dur_us(pWaveBuffer->fill_size, sr, ch, bps);
            pWaveBuffer->pts += pts_offset;

            //pWaveBuffer->wave_hdr.lpData =(LPSTR)pWaveBuffer->p_buf;
	        //pWaveBuffer->wave_hdr.dwBufferLength = CMmpWaveBuffer::WAVE_BUF_SIZE;
	        //pWaveBuffer->wave_hdr.dwFlags = 0L;    // start position
	        //pWaveBuffer->wave_hdr.dwLoops = 0L;   // loop
	        //pWaveBuffer->wave_hdr.dwBytesRecorded = CMmpWaveBuffer::WAVE_BUF_SIZE;
            //pWaveBuffer->wave_hdr.dwUser=(DWORD)pWaveBuffer;

            MMPDEBUGMSG(1, (TEXT("[CMmpRenderer_WaveOutEx3::Render] waveOut  src_remai=%d/%d"),  src_remain_size, p_buf_af->get_data_size()));
    
            waveOutWrite(m_WaveOutHandle,  &pWaveBuffer->wave_hdr, sizeof(WAVEHDR) );         
        }
    }

    return mmpResult;
}
#else


MMP_RESULT CMmpRenderer_WaveOutEx3::Render(class mmp_buffer_audioframe* p_buf_af) {

    MMP_RESULT mmpResult = MMP_SUCCESS;
    CMmpWaveBuffer* pWaveBuffer;
    
    MMP_U8* p_buf_src = (MMP_U8*)p_buf_af->get_buf_vir_addr();
    MMP_S32 src_size;
    
    MMP_U8 *p_buf_dest;
    MMP_S32 sr, ch, bps;
    MMP_S64 pts_offset;

    MMP_S32 wavebuf_remain_byte;
    
    MMP_BOOL is_send = MMP_FALSE;

    sr = this->get_sr();
    ch = this->get_ch();
    bps = this->get_bps();
    src_size = p_buf_af->get_data_size();
    
    pWaveBuffer = this->GetNextWaveBufferWithLock();
    
    wavebuf_remain_byte = pWaveBuffer->alloc_size - pWaveBuffer->fill_size;
    if(wavebuf_remain_byte <= src_size*2) {
        is_send = MMP_TRUE;
    }
    
    p_buf_dest = &pWaveBuffer->p_buf[pWaveBuffer->fill_size];
    memcpy(p_buf_dest, (void*)p_buf_src, src_size);
    pWaveBuffer->fill_size += src_size;
                
    if(is_send == MMP_TRUE) {
        pWaveBuffer->free_tick = CMmpWaveBuffer::BUSY_TICK;
        pts_offset = 0;//mmp_audio_tool::pcm_cal_dur_us(pWaveBuffer->fill_size, sr, ch, bps);
        pWaveBuffer->pts += p_buf_af->get_pts() + pts_offset;

        //pWaveBuffer->wave_hdr.lpData =(LPSTR)pWaveBuffer->p_buf;
	    pWaveBuffer->wave_hdr.dwBufferLength = pWaveBuffer->fill_size;
	    //pWaveBuffer->wave_hdr.dwFlags = 0L;    // start position
	    //pWaveBuffer->wave_hdr.dwLoops = 0L;   // loop
	    //pWaveBuffer->wave_hdr.dwBytesRecorded = pWaveBuffer->fill_size;
        //pWaveBuffer->wave_hdr.dwUser=(DWORD)pWaveBuffer;
        src_size = p_buf_af->get_data_size();

        MMPDEBUGMSG(1, (TEXT("[CMmpRenderer_WaveOutEx3::Render] waveOut fill=%d src_remai=%d  Loop=%d dwFlag=%d "),  
                  pWaveBuffer->fill_size, src_size,
                  pWaveBuffer->wave_hdr.dwLoops,
                  pWaveBuffer->wave_hdr.dwFlags
                  ));
    
        waveOutWrite(m_WaveOutHandle,  &pWaveBuffer->wave_hdr, sizeof(WAVEHDR) );         
    }
    

    return mmpResult;
}

#endif
