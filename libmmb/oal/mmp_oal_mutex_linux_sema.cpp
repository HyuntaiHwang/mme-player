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

#include "mmp_oal_mutex_linux_sema.hpp"

#if (MMP_OS == MMP_OS_LINUX)


#include <fcntl.h>
#include<sys/sem.h>

#define GLOABL_SEMA_TYPE_SEM_OPEN 1
#define GLOABL_SEMA_TYPE_SEM_GET  2
#define GLOABL_SEMA_TYPE  GLOABL_SEMA_TYPE_SEM_GET
/**********************************************************
class members
**********************************************************/

mmp_oal_mutex_linux_sema::mmp_oal_mutex_linux_sema(MMP_U32 key) : mmp_oal_mutex(key) 
,m_p_sem(SEM_FAILED)
,m_sem_id(-1)
{
	
}

mmp_oal_mutex_linux_sema::~mmp_oal_mutex_linux_sema() {

}


#if 0
     struct semid_ds {
                struct ipc_perm sem_perm;       /* permissions .. see ipc.h */
                time_t          sem_otime;      /* last semop time */
                time_t          sem_ctime;      /* last change time */
                struct sem      *sem_base;      /* ptr to first semaphore in array */
                struct wait_queue *eventn;
                struct wait_queue *eventz;
                struct sem_undo  *undo;         /* undo requests on this array */
                ushort          sem_nsems;      /* no. of semaphores in array */
        };
#endif


MMP_RESULT mmp_oal_mutex_linux_sema::open() {

	MMP_RESULT mmpResult = MMP_SUCCESS;
    char mutex_name[32];
    int iret;

    if(m_key == 0) {
        iret = sem_init(&m_sem, 0, 1);
        if(iret < 0 ) {
            m_p_sem = SEM_FAILED;
            mmpResult = MMP_FAILURE;
        }
        else {
            m_p_sem = &m_sem;
        }
    }
    else {

#if (GLOABL_SEMA_TYPE == GLOABL_SEMA_TYPE_SEM_GET)
        m_sem_id = semget((key_t)m_key, 1, 0666|IPC_EXCL);
        if(m_sem_id < 0) {
            m_sem_id = semget((key_t)m_key, 1, 0666|IPC_CREAT);
            if(m_sem_id >= 0) {
            
                /* new create */
                struct semid_ds sem_buf;
                int arg  =  1;             /* 세마포어 값을 1로 설정 */
                semctl(m_sem_id, 0, SETVAL, arg);
            }
        }
        else {
            
        }


        if(m_sem_id >= 0) {
            m_p_sem = (sem_t*)m_sem_id;
        }
        else {
            mmpResult = MMP_FAILURE;
        }
#else
        sprintf(mutex_name, "mme_sema-0x%08x", m_key);
        m_p_sem = sem_open(mutex_name, O_CREAT, 0777, 1);
        if(m_p_sem == SEM_FAILED) {
            mmpResult = MMP_FAILURE;
        }
#endif
    }

	return mmpResult;
}


MMP_RESULT mmp_oal_mutex_linux_sema::close() {
	
	MMP_RESULT ret = MMP_SUCCESS;
    //struct semid_ds sem_buf;

    if(m_p_sem != SEM_FAILED) {

        if(m_key == 0) {
            sem_destroy(m_p_sem);
        }
        else {
#if (GLOABL_SEMA_TYPE == GLOABL_SEMA_TYPE_SEM_GET)

            //if(m_sem_id >= 0) {
            //    semctl(m_sem_id, 0, IPC_STAT, &sem_buf);
            //}

            //printf("shm_buf.shm_nattch = %d\n",  (int)shm_buf.shm_nattch);

            //if(shm_buf.shm_nattch == 0) {
            //    shmctl(m_shm_id, IPC_RMID,  NULL);
            //    m_shm_id = -1;
            //}

#else
            sem_close(m_p_sem);
#endif
        }
        m_p_sem = SEM_FAILED;
    }

	return ret;
}

#if 0
struct sembuf {
    short sem_num;   세마포어 번호
    short sem_op;     세마포어 증감값
    short sem_flg;     옵션
} 
#endif
void mmp_oal_mutex_linux_sema::lock() {
	

    if(m_key == 0) {
    	sem_wait(m_p_sem);
    }
    else {
#if (GLOABL_SEMA_TYPE == GLOABL_SEMA_TYPE_SEM_GET)
        struct sembuf buf;
        buf.sem_num = 0;
        buf.sem_op = -1;
        buf.sem_flg = SEM_UNDO;
        semop(m_sem_id, &buf, 1);
#else
        sem_wait(m_p_sem);
#endif
    }
}
	
void mmp_oal_mutex_linux_sema::unlock() {

    if(m_key == 0) {
	    sem_post(m_p_sem);
    }
    else {

#if (GLOABL_SEMA_TYPE == GLOABL_SEMA_TYPE_SEM_GET)
        struct sembuf buf;
        buf.sem_num = 0;
        buf.sem_op = 1;
        buf.sem_flg = SEM_UNDO;
        semop(m_sem_id, &buf, 1);
#else
        sem_post(m_p_sem);
#endif
    }

}

void* mmp_oal_mutex_linux_sema::get_handle() {

	return (void*)m_p_sem;
}

#endif
