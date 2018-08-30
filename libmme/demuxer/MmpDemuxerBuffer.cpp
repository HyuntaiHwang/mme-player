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

#include "MmpDemuxerBuffer.hpp"
#include "mmp_buffer_mgr.hpp"

CMmpDemuxerBuffer::CMmpDemuxerBuffer() {

    MMP_S32 i;

    for(i = 0; i < MT_MAX; i++) {
        this->m_queue_media[i].SetSize(30*10);
    }

}

CMmpDemuxerBuffer::~CMmpDemuxerBuffer() {
    
    this->queue_clear();
    
}

MMP_S32 CMmpDemuxerBuffer::queue_get_empty_streamindex(void) {

    MMP_S32 index = -1;
    MMP_S32 i;

    /* check if there are empty queue */
    for(i = 0; i < MT_MAX; i++) {
        if(this->m_queue_media[i].IsEmpty()) {
            index = i;
            break;
        }
    }

    return index;
}

void CMmpDemuxerBuffer::queue_add(class mmp_buffer_media* p_buf_media) {

    enum MMP_MEDIATYPE mt;
    class mmp_buffer_media* p_buf_media_tmp;

    mt = p_buf_media->get_media_type();

    if(this->m_queue_media[mt].IsFull()){
        this->m_queue_media[mt].Delete(p_buf_media_tmp);
        mmp_buffer_mgr::get_instance()->free_media_buffer(p_buf_media_tmp);
    }

    this->m_queue_media[mt].Add(p_buf_media);

}

class mmp_buffer_media* CMmpDemuxerBuffer::queue_get(MMP_BOOL bforce_get) {

    int i, media_index;
    bool bret = false;
    MMP_TICKS ts_min;
    MMP_RESULT mmpResult = MMP_FAILURE;
    class mmp_buffer_media* p_buf_media;
    class mmp_buffer_media* p_buf_media_next = NULL;

    static unsigned int s_nops_value_flag = 0;

    media_index = -1;
    ts_min = LLONG_MAX;
    for(i = 0; i < MT_MAX; i++) {
        
        if(!this->m_queue_media[i].IsEmpty()) {
            
            this->m_queue_media[i].GetFirstItem(p_buf_media);
            if(p_buf_media->get_pts() < ts_min) {
                ts_min = p_buf_media->get_pts();
                media_index = i;
            }
        }
        else if(bforce_get != MMP_TRUE) {
            break;
        }
    }

    
    if( (media_index >= 0) && (i==MT_MAX) ) {
        this->m_queue_media[media_index].Delete(p_buf_media);
        p_buf_media_next = p_buf_media;
    }
    
    return p_buf_media_next;
}


MMP_RESULT CMmpDemuxerBuffer::queue_clear() {

    MMP_S32 i;
    class mmp_buffer_media* p_buf_media;

    for(i = 0; i < MT_MAX; i++) {

        while(!this->m_queue_media[i].IsEmpty()) {
                
            this->m_queue_media[i].Delete(p_buf_media);
            if(p_buf_media != NULL) {
                mmp_buffer_mgr::get_instance()->free_media_buffer(p_buf_media);
            }
        }
    }

    return MMP_SUCCESS;
}