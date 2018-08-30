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

#define MME_LOGV 0
#include "mmp_singleton_mgr.hpp"

/*vpu dev singletone */
#if (MMP_HW_CODEC_VIDEO_VPU_ENABLE == 1)
#include "mmp_vpu_dev.hpp"
#endif

//#include "mmp_jpu_dev.hpp"
#include "mmp_buffer_mgr.hpp"
#include "mmp_env_mgr.hpp"
#include "MmpUtil.hpp"

#if (MMP_OS == MMP_OS_WIN32)
extern "C" void vpu_machine_start(void);
extern "C" void vpu_machine_stop(void);
extern "C" void kernel_machine_start(void);
extern "C" void kernel_machine_stop(void);
#endif

class mmp_singleton_mgr *mmp_singleton_mgr::s_pinstance = NULL;
//class mmp_singleton_mgr s_instance(0);

mmp_singleton_mgr::mmp_singleton_mgr() {

    MMP_RESULT mmpResult = MMP_SUCCESS;
    MMP_S32 i;

    MMPDEBUGMSG(1, (TEXT("[mmp_singleton_mgr::mmp_singleton_mgr] ++ ")));

    for(i = 0; i < ID_MAX; i++) {
        m_mmpResult[i] = MMP_FAILURE;    
    }

#if (MMP_OS == MMP_OS_WIN32)
    kernel_machine_start();
#endif

    /* create env_mgr */
    if(mmpResult == MMP_SUCCESS) {
        mmpResult = mmp_env_mgr::create_instance();
        m_mmpResult[ID_ENV_MGR] = mmpResult;
    }

    /* create buffer_mgr */
    if(mmpResult == MMP_SUCCESS) {
        mmpResult = mmp_buffer_mgr::create_instance();
        m_mmpResult[ID_BUFFER_MGR] = mmpResult;
    }

    /* create vpu_dev */
#if (MMP_HW_CODEC_VIDEO_VPU_ENABLE == 1)
    if(mmpResult == MMP_SUCCESS) {
        mmpResult = mmp_vpu_dev::create_instance();
        m_mmpResult[ID_VPU_DEV] = mmpResult;
    }
#endif

/* create jpu_dev */
#if 0
#if (JPU_PLATFORM_V4L2_ENABLE != 1)
    if(mmpResult == MMP_SUCCESS) {
        mmpResult = mmp_jpu_dev::create_instance();
        m_mmpResult[ID_JPU_DEV] = mmpResult;
    }
#endif
#endif

}

mmp_singleton_mgr::~mmp_singleton_mgr() {

#if 0
#if (JPU_PLATFORM_V4L2_ENABLE != 1)
    mmp_jpu_dev::destroy_instance();
#endif
#endif

#if (MMP_HW_CODEC_VIDEO_VPU_ENABLE == 1)
    mmp_vpu_dev::destroy_instance();
#endif

    mmp_buffer_mgr::destroy_instance();
    mmp_env_mgr::destroy_instance();

#if (MMP_OS == MMP_OS_WIN32)
    kernel_machine_stop();
#endif

    MMPDEBUGMSG(1, (TEXT("[mmp_singleton_mgr::mmp_singleton_mgr] -- ")));
}

MMP_RESULT mmp_singleton_mgr::get_result(int singletone_inst_id) {

    MMP_RESULT mmpResult = MMP_FAILURE;

    if(mmp_singleton_mgr::s_pinstance != NULL) {
        if( (singletone_inst_id>=0) && (singletone_inst_id < ID_MAX) ) {
            mmpResult = mmp_singleton_mgr::s_pinstance->m_mmpResult[singletone_inst_id];
        }
    }

    return mmpResult;
}

int mme_singleton_mgr_create(void) {

#ifndef WIN32
    MMPDEBUGMSG(MME_LOGV, (TEXT("[mme_singleton_mgr_create] mmp_singleton_mgr::s_pinstance=0x%x  ++++  pid=%d "), mmp_singleton_mgr::s_pinstance, getpid() ));
#endif
    if(mmp_singleton_mgr::s_pinstance == NULL) {
            mmp_singleton_mgr::s_pinstance = new class mmp_singleton_mgr;
    }
#ifndef WIN32
    MMPDEBUGMSG(MME_LOGV, (TEXT("[mme_singleton_mgr_create] mmp_singleton_mgr::s_pinstance=0x%x  ---- "), mmp_singleton_mgr::s_pinstance));
#endif
    return 0;
}

int mme_singleton_mgr_destroy(void) {

#ifndef WIN32
    MMPDEBUGMSG(MME_LOGV, (TEXT("[mme_singleton_mgr_destroy] mmp_singleton_mgr::s_pinstance=0x%x  ++++  pid=%d "), mmp_singleton_mgr::s_pinstance, getpid() ));
#endif
    if(mmp_singleton_mgr::s_pinstance != NULL) {
        delete mmp_singleton_mgr::s_pinstance;
        mmp_singleton_mgr::s_pinstance = NULL;
    }
#ifndef WIN32
    MMPDEBUGMSG(MME_LOGV, (TEXT("[mme_singleton_mgr_destroy] mmp_singleton_mgr::s_pinstance=0x%x  ---- "), mmp_singleton_mgr::s_pinstance));
#endif
    return 0;
}

int mme_singleton_mgr_get_result_HWCodec(void) {

    return mmp_singleton_mgr::get_result(mmp_singleton_mgr::ID_VPU_DEV);
}
