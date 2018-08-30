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

#include "mmp_msg_proc.hpp"
#include "MmpUtil.hpp"

/*******************************************
class mmp_msg_packet
*******************************************/
void mmp_msg_packet::init() {
    
    int i;

    m_msg = 0;
    m_data = NULL;
    m_data_size = 0;

    m_timer_expire_tick = 0;

    for(i = 0; i < MMP_MSG_PACKET_STRING_ARRAY_MAX_COUNT; i++) {
        m_string_array[i] = NULL;
        m_string_size[i] = 0;
    }
    m_string_count = 0;
}

void mmp_msg_packet::deinit() {

    int i;

    if(m_data) {
        delete [] m_data;
    }

    m_msg = 0;
    m_data = NULL;
    m_data_size = 0;

    for(i = 0; i < MMP_MSG_PACKET_STRING_ARRAY_MAX_COUNT; i++) {
        if(m_string_array[i] != NULL)  delete [] m_string_array[i];
    }
}

mmp_msg_packet::mmp_msg_packet(MMP_U32 msg) {

    this->init();
    m_msg = msg;
}

mmp_msg_packet::mmp_msg_packet(MMP_U32 msg, MMP_U8* data, MMP_S32 data_size) 
{
    this->init();
    m_msg = msg;
    m_data = new MMP_U8[data_size+16];
    memset(m_data, 0x00, data_size+16);
    memcpy(m_data, data, data_size);
    m_data_size = data_size;
}

mmp_msg_packet::~mmp_msg_packet() {

    this->deinit();
}

MMP_RESULT mmp_msg_packet::add_string(MMP_STRING string, MMP_S32 str_size) {
 
    MMP_CHAR* ptr;
    MMP_RESULT mmpResult = MMP_FAILURE;

    if(str_size > 0) {

        ptr = new MMP_CHAR[str_size+16];
        if(ptr != NULL) {
            memset(ptr, 0x00, str_size+16);
            memcpy(ptr, string, str_size);
            
            m_string_array[m_string_count] = ptr;
            m_string_size[m_string_count] = str_size;
            
            m_string_count++;
            mmpResult = MMP_SUCCESS;
        }
    }
    else if( (string==NULL) && str_size==0) {
        
        m_string_array[m_string_count] = NULL;
        m_string_size[m_string_count] = 0;
        
        m_string_count++;
        mmpResult = MMP_SUCCESS;
    }

    return mmpResult;
}

void mmp_msg_packet::set_timer(MMP_U32 dur_timer) {
    m_timer_expire_tick = CMmpUtil::GetTickCount() + dur_timer;
}

/*******************************************
class mmp_msg_res
*******************************************/
mmp_msg_res::mmp_msg_res(MMP_S32 queue_size) :

m_req_queue_size(queue_size)
,m_p_mutex(NULL)
,m_p_cond(NULL)
,m_queue(queue_size)
,m_task_run(MMP_FALSE)
,m_p_task(NULL)
{

}

mmp_msg_res::~mmp_msg_res() {

    this->close();
}

MMP_RESULT mmp_msg_res::open(void (*task_service_func)(void*), void* task_parm) {

    MMP_RESULT mmpResult = MMP_SUCCESS;
    struct mmp_oal_task_create_config oal_task_create_config;
    class mmp_msg_packet* p_msg_pack;
    MMP_S32 i;

    /* check queue status */
    if(mmpResult == MMP_SUCCESS) {
        
        for(i = 0; i < m_req_queue_size*100; i++) {
            p_msg_pack = (class mmp_msg_packet*)(0x100+i);
            m_queue.Add(p_msg_pack);
            if(m_queue.IsFull()==MMP_TRUE) {
                break;
            }
        }

        if(m_queue.GetSize() == m_req_queue_size) {
            
            for(i = 0; i < m_req_queue_size*100; i++) {
                m_queue.Delete(p_msg_pack);
                if(p_msg_pack != (class mmp_msg_packet*)(0x100+i) ) {
                    mmpResult = MMP_FAILURE;
                    break;
                }

                if(m_queue.IsEmpty()==MMP_TRUE) {
                    i++;
                    break;
                }
            }

            if(i != m_req_queue_size) {
                mmpResult = MMP_FAILURE;
            }
        }
        else {
            mmpResult = MMP_FAILURE;
        }

    }

    if(mmpResult == MMP_SUCCESS) {
        m_p_mutex = mmp_oal_mutex::create_object();
        if(m_p_mutex == NULL) mmpResult = MMP_FAILURE;
    }

    if(mmpResult == MMP_SUCCESS) {
        m_p_cond = mmp_oal_cond::create_object();
        if(m_p_cond == NULL) mmpResult = MMP_FAILURE;
    }

    m_task_run = MMP_TRUE;
    if((mmpResult == MMP_SUCCESS) && (task_service_func!=NULL) ) {
        oal_task_create_config.flag = 0;
        oal_task_create_config.service = task_service_func;
        oal_task_create_config.arg = task_parm;
        m_p_task =  mmp_oal_task::create_object(&oal_task_create_config);
        if(m_p_task == NULL) mmpResult = MMP_FAILURE;
    }
    
    return mmpResult;    
}

MMP_RESULT mmp_msg_res::close() {

    m_task_run = MMP_FALSE;
    if((m_p_task != NULL) && (m_p_cond != NULL) ) {
        m_p_cond->signal();
        CMmpUtil::Sleep(10);
        mmp_oal_task::destroy_object(m_p_task);
        m_p_task = NULL;

        mmp_oal_cond::destroy_object(m_p_cond);
        m_p_cond = NULL;
    }

    if(m_p_cond != NULL) {
        m_p_cond->signal();
        CMmpUtil::Sleep(10);
        mmp_oal_cond::destroy_object(m_p_cond);
        m_p_cond = NULL;
    }

    if(m_p_mutex != NULL) {
        mmp_oal_mutex::destroy_object(m_p_mutex);
        m_p_mutex = NULL;
    }

    return MMP_SUCCESS;    
}

MMP_RESULT mmp_msg_res::postmsg_to_queue(class mmp_msg_packet* p_packet) {

    MMP_U32 t1, t2;
    MMP_RESULT mmpResult = MMP_FAILURE;

    t1 = CMmpUtil::GetTickCount();
    t2 = t1;

    while( ((t2-t1) < 1000) && (mmpResult == MMP_FAILURE) ) {

        this->m_p_mutex->lock();
        if(this->m_queue.IsFull()) {
            mmpResult = MMP_FAILURE;
        }
        else {
            this->m_queue.Add(p_packet);
            mmpResult = MMP_SUCCESS;
        }
        this->m_p_mutex->unlock();

        t2 = CMmpUtil::GetTickCount();
        if(mmpResult == MMP_FAILURE) {
            CMmpUtil::Sleep(10);
        }
    }

    if(mmpResult == MMP_SUCCESS) {
        this->m_p_cond->signal();
    }

    return mmpResult;
}


MMP_RESULT mmp_msg_res::readmsg_from_queue(class mmp_msg_packet** p_packet) {

    MMP_RESULT mmpResult = MMP_FAILURE;
    class mmp_msg_packet* p_tmppack;
    MMP_U32 cur_tick;
    
    while(this->m_task_run == MMP_TRUE) {

        this->m_p_mutex->lock();
        if(this->m_queue.IsEmpty()) {
            this->m_p_cond->wait(this->m_p_mutex);
        }

        p_tmppack = NULL;
        if(!this->m_queue.IsEmpty()) {
            this->m_queue.Delete(p_tmppack);

            cur_tick = CMmpUtil::GetTickCount();
            if(cur_tick < p_tmppack->get_timer_expirt_tick()) {
                
                this->m_p_mutex->unlock();
                CMmpUtil::Sleep(10);
                this->m_p_mutex->lock();
                
                this->m_queue.Add(p_tmppack);
                
                p_tmppack = NULL;
            }
        }
        this->m_p_mutex->unlock();

        if(p_tmppack != NULL) {
            *p_packet =p_tmppack;
            mmpResult = MMP_SUCCESS;
            break;
        }
    }

    return mmpResult;
}

/*******************************************
class mmp_msg_proc
*******************************************/

mmp_msg_proc::mmp_msg_proc() :
m_res_read(1024*10)
,m_res_write(1024*10)

,m_task_timer_run(MMP_FALSE)
,m_p_task_timer(NULL)

,m_timer_msg(MMP_MSG_DEFAULT_TIMER_MSG)
,m_p_mutex_timer(NULL)
{

}

mmp_msg_proc::~mmp_msg_proc() {

}

MMP_RESULT mmp_msg_proc::open() {

    MMP_RESULT mmpResult = MMP_SUCCESS;
    struct mmp_oal_task_create_config oal_task_create_config;

    if(mmpResult == MMP_SUCCESS) {
        mmpResult = m_res_read.open(NULL, NULL);
    }

    if(mmpResult == MMP_SUCCESS) {
        mmpResult = m_res_write.open(mmp_msg_proc::service_write_stub, (void*)this);
    }

#if 1
    if(mmpResult == MMP_SUCCESS) {

        m_p_mutex_timer = mmp_oal_mutex::create_object();
        if(m_p_mutex_timer != NULL) {
            m_task_timer_run = MMP_TRUE;
            oal_task_create_config.flag = 0;
            oal_task_create_config.service = mmp_msg_proc::service_timerstub;
            oal_task_create_config.arg = this;
            m_p_task_timer =  mmp_oal_task::create_object(&oal_task_create_config);
        }
    }
#endif

    return mmpResult;
}

MMP_RESULT mmp_msg_proc::close() {

    if(m_p_task_timer != NULL) {
        m_task_timer_run = MMP_FALSE;
        mmp_oal_task::destroy_object(m_p_task_timer);
        m_p_task_timer = NULL;
    }

    if(m_p_mutex_timer != NULL) {
        mmp_oal_mutex::destroy_object(m_p_mutex_timer);
        m_p_mutex_timer = NULL;
    }

    m_res_read.close();
    m_res_write.close();

    return MMP_SUCCESS;
}

void mmp_msg_proc::service_read_stub(void* parm) {
    class mmp_msg_proc *p_obj = (class mmp_msg_proc*)parm;
    p_obj->service_proc(MMP_TRUE);
}

void mmp_msg_proc::service_write_stub(void* parm) {
    class mmp_msg_proc *p_obj = (class mmp_msg_proc*)parm;
    p_obj->service_proc(MMP_FALSE);
}

void mmp_msg_proc::service_proc(MMP_BOOL is_read) {

    class mmp_msg_res *p_res;
    class mmp_msg_packet* p_packet;
    MMP_RESULT mmpResult;

    if(is_read == MMP_TRUE) p_res = &m_res_read;
    else  p_res = &m_res_write;

    while(p_res->is_run() == MMP_TRUE) {
    
        p_packet = NULL;
        mmpResult = p_res->readmsg_from_queue(&p_packet);
        
        if(p_res->is_run() != MMP_TRUE) break;

        if(mmpResult == MMP_SUCCESS) {

            if(is_read == MMP_TRUE) this->service_read_proc(p_packet);
            else  this->service_write_proc(p_packet);
        }
    }

}

MMP_RESULT mmp_msg_proc::postmsg_to_readqueue(class mmp_msg_packet* p_packet) {

    return m_res_read.postmsg_to_queue(p_packet);
}

MMP_RESULT mmp_msg_proc::postmsg_to_writequeue(class mmp_msg_packet* p_packet) {

    return m_res_write.postmsg_to_queue(p_packet);
}

/*    
MMP_RESULT mmp_msg_proc::postmsg_to_queue(MMP_BOOL is_read, class mmp_msg_packet* p_packet) {

    class mmp_msg_res *p_res;
    MMP_U32 t1, t2;
    MMP_RESULT mmpResult = MMP_FAILURE;

    if(is_read == MMP_TRUE) p_res = &m_res_read;
    else  p_res = &m_res_write;

    t1 = CMmpUtil::GetTickCount();
    t2 = t1;

    while( ((t2-t1) < 1000) && (mmpResult == MMP_FAILURE) ) {

        p_res->m_p_mutex->lock();
        if(p_res->m_queue.IsFull()) {
            mmpResult = MMP_FAILURE;
        }
        else {
            p_res->m_queue.Add(p_packet);
            mmpResult = MMP_SUCCESS;
        }
        p_res->m_p_mutex->unlock();

        t2 = CMmpUtil::GetTickCount();
        if(mmpResult == MMP_FAILURE) {
            CMmpUtil::Sleep(10);
        }
    }

    if(mmpResult == MMP_SUCCESS) {
        p_res->m_p_cond->signal();
    }

    return mmpResult;
}
*/

MMP_RESULT mmp_msg_proc::readmsg_from_readqueue(class mmp_msg_packet** p_packet) {
    
    return m_res_read.readmsg_from_queue(p_packet);
}

/*
MMP_RESULT mmp_msg_proc::readmsg_from_queue(MMP_BOOL is_read, class mmp_msg_packet** p_packet) {

    class mmp_msg_res *p_res;
    MMP_RESULT mmpResult = MMP_FAILURE;
    class mmp_msg_packet* p_tmppack;
    
    if(is_read == MMP_TRUE) p_res = &m_res_read;
    else  p_res = &m_res_write;

    while(p_res->m_task_run == MMP_TRUE) {

        p_res->m_p_mutex->lock();
        if(p_res->m_queue.IsEmpty()) {
            p_res->m_p_cond->wait(p_res->m_p_mutex);
        }

        p_tmppack = NULL;
        if(!p_res->m_queue.IsEmpty()) {
            p_res->m_queue.Delete(p_tmppack);
        }
        p_res->m_p_mutex->unlock();

        if(p_tmppack != NULL) {
            *p_packet =p_tmppack;
            mmpResult = MMP_SUCCESS;
            break;
        }
    }

    return mmpResult;
}
*/

MMP_RESULT mmp_msg_proc::timer_add(MMP_U32 id, MMP_U32 dur_tick) {

    struct timer_obj to, tmpto;
    bool flag;
    MMP_BOOL install_timer = MMP_TRUE;

    m_p_mutex_timer->lock();

    to.id = id;
    to.dur_tick = dur_tick;
    to.last_expire_tick = CMmpUtil::GetTickCount();

    install_timer = MMP_TRUE;
    flag = m_list_timer.GetFirst(tmpto);
    while(flag) {
        if(tmpto.id == to.id) {
            install_timer = MMP_FALSE;
            break;
        }
        flag = m_list_timer.GetNext(tmpto);
    }
    if(install_timer == MMP_TRUE) {
        m_list_timer.Add(to);
    }

    m_p_mutex_timer->unlock();

    return MMP_SUCCESS;
}

MMP_RESULT mmp_msg_proc::timer_delete(MMP_U32 id) {

    struct timer_obj to;
    int item_index;
    bool bflag;

    m_p_mutex_timer->lock();

    item_index = 0;
    bflag = m_list_timer.GetFirst(to);
    while(bflag) {

        if(id == to.id) {
            m_list_timer.Del(item_index);
            break;
        }
        bflag = m_list_timer.GetNext(to);
        item_index ++;
    }
    m_p_mutex_timer->unlock();

    return MMP_SUCCESS;
}

void mmp_msg_proc::service_timerstub(void* parm) {

    class mmp_msg_proc* p_obj = (class mmp_msg_proc*)parm;
    p_obj->service_timer();
}

void mmp_msg_proc::service_timer() {

    bool bflag;
    struct timer_obj to;
    MMP_U32 cur_tick;
    class mmp_msg_packet* p_packet;
    int item_index;

    while(m_task_timer_run) {

        m_p_mutex_timer->lock();
        item_index = 0;
        bflag = m_list_timer.GetFirst(to);
        while(bflag) {
            
            cur_tick = CMmpUtil::GetTickCount();

            if(cur_tick > to.last_expire_tick) {
                
                if( (cur_tick - to.last_expire_tick) > to.dur_tick ) {
                    p_packet = new class mmp_msg_packet(m_timer_msg);
                    p_packet->m_int_parm[0] = to.id;
                    p_packet->m_int_parm[1] = to.dur_tick;
                    this->postmsg_to_writequeue(p_packet);

                    to.last_expire_tick = cur_tick;
                    m_list_timer.Set(item_index, to);
                }
            }

            CMmpUtil::Sleep(10);
            bflag = m_list_timer.GetNext(to);
            item_index++;
        }
        m_p_mutex_timer->unlock();
    
        CMmpUtil::Sleep(10);
    }
    
}

