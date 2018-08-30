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

#ifndef MMP_MSG_PROC_HPP__
#define MMP_MSG_PROC_HPP__

#include "MmpDefine.h"
#include "mmp_oal_task.hpp"
#include "mmp_oal_mutex.hpp"
#include "mmp_oal_cond.hpp"
#include "TemplateList.hpp"

//typedef void (*mmp_msg_callbackfunc)(void*, int msg, void* data, int datalen);

#define MMP_MSG_DEFAULT_TIMER_MSG  0xFFFF1111
#define MMP_MSG_PACKET_STRING_ARRAY_MAX_COUNT 16
class mmp_msg_packet {

public:
    MMP_U32 m_msg;
    MMP_S32 m_int_parm[16];
    MMP_U8* m_data;
    MMP_S32 m_data_size;

    MMP_STRING m_string_array[MMP_MSG_PACKET_STRING_ARRAY_MAX_COUNT];
    MMP_S32 m_string_size[MMP_MSG_PACKET_STRING_ARRAY_MAX_COUNT];
    MMP_S32 m_string_count;

private:
    MMP_U32 m_timer_expire_tick;
       
public:
    mmp_msg_packet(MMP_U32 msg);
    mmp_msg_packet(MMP_U32 msg, MMP_U8* data, MMP_S32 data_size);
    virtual ~mmp_msg_packet();

    MMP_RESULT add_string(MMP_STRING string, MMP_S32 str_size);
    void set_timer(MMP_U32 dur_timer);
    inline MMP_U32 get_timer_expirt_tick() { return m_timer_expire_tick; }
    
private:
    void init();
    void deinit();
};

class mmp_msg_res {

private:
    MMP_S32 m_req_queue_size;

    class mmp_oal_mutex* m_p_mutex;
    class mmp_oal_cond* m_p_cond;
    TCircular_Queue<class mmp_msg_packet*> m_queue;

    MMP_BOOL m_task_run;
    class mmp_oal_task *m_p_task;

public:
    mmp_msg_res(MMP_S32 queue_size);
    virtual ~mmp_msg_res();

    virtual MMP_RESULT open(void (*task_service_func)(void*), void* task_parm);
    virtual MMP_RESULT close();

    inline MMP_BOOL is_run() { return m_task_run; }

    MMP_RESULT readmsg_from_queue(class mmp_msg_packet** p_packet);
    MMP_RESULT postmsg_to_queue(class mmp_msg_packet* p_packet);
};

class mmp_msg_proc {

protected:
    mmp_msg_proc();
    virtual ~mmp_msg_proc();
    
private:
    class mmp_msg_res m_res_read;
    class mmp_msg_res m_res_write;
    
    MMP_BOOL m_task_timer_run;
    class mmp_oal_task *m_p_task_timer;

    struct timer_obj {
        MMP_U32 id;
        MMP_U32 dur_tick;
        MMP_U32 last_expire_tick;
    };
    TList<struct timer_obj> m_list_timer;
    MMP_U32 m_timer_msg;
    class mmp_oal_mutex *m_p_mutex_timer;

private:
    static void service_read_stub(void* parm);
    static void service_write_stub(void* parm);

    void service_proc(MMP_BOOL is_read);

    static void service_timerstub(void*);
    void service_timer();
    
protected:

    virtual MMP_RESULT open();
    virtual MMP_RESULT close();

    
    virtual MMP_RESULT postmsg_to_readqueue(class mmp_msg_packet* p_packet);
    virtual MMP_RESULT postmsg_to_writequeue(class mmp_msg_packet* p_packet);
    //virtual MMP_RESULT postmsg_to_queue(MMP_BOOL is_read, class mmp_msg_packet* p_packet);
    
    virtual MMP_RESULT readmsg_from_readqueue(class mmp_msg_packet** p_packet);
    //virtual MMP_RESULT readmsg_from_queue(MMP_BOOL is_read, class mmp_msg_packet** p_packet);
    
    virtual MMP_RESULT service_read_proc(class mmp_msg_packet* p_packet) = 0;
    virtual MMP_RESULT service_write_proc(class mmp_msg_packet* p_packet) = 0;

    void timer_set_msg(MMP_U32 msg) { m_timer_msg = msg; }
    MMP_RESULT timer_add(MMP_U32 id, MMP_U32 dur_tick);
    MMP_RESULT timer_delete(MMP_U32 id);
    
};

#endif