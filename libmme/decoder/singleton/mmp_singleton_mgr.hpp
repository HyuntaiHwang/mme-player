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

#ifndef MMP_SINGLETON_MGR_HPP__
#define MMP_SINGLETON_MGR_HPP__

#include "MmpDefine.h"

extern "C" int mme_singleton_mgr_create(void);
extern "C" int mme_singleton_mgr_destroy(void);
extern "C" int mme_singleton_mgr_get_result_HWCodec(void);

class mmp_singleton_mgr {

friend int mme_singleton_mgr_create(void);
friend int mme_singleton_mgr_destroy(void);
public:
    enum {
        ID_ENV_MGR=0,
        ID_BUFFER_MGR,
        ID_VPU_DEV,
        ID_JPU_DEV,
        ID_MAX
    };
private:
    static class mmp_singleton_mgr *s_pinstance;
    MMP_RESULT m_mmpResult[ID_MAX];

public:
    static MMP_RESULT get_result(int singletone_inst_id);

private:
    mmp_singleton_mgr();
    ~mmp_singleton_mgr();


};

#endif


