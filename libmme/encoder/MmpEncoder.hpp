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

#ifndef MMPENCODER_HPP__
#define MMPENCODER_HPP__

#include "MmpDefine.h"
#include "MmpPlayerDef.h"
#include "mmp_buffer_mgr.hpp"


class CMmpEncoder
{
public:
    static MMP_RESULT DestroyObject(CMmpEncoder* pObj);

protected:
    MMP_U32 m_nClassStartTick;

private:
    enum MMP_FOURCC m_fourcc_in;
    enum MMP_FOURCC m_fourcc_out;

    MMP_CHAR m_fourcc_in_name[8];
    MMP_CHAR m_fourcc_out_name[8];
    MMP_CHAR m_class_name[16];

    const enum MMP_FOURCC *m_support_fourcc_in_list;
    MMP_S32 m_support_fourcc_in_count;
    
    const enum MMP_FOURCC *m_support_fourcc_out_list;
    MMP_S32 m_support_fourcc_out_count;
        
protected:
    CMmpEncoder(enum MMP_FOURCC fourcc_in, enum MMP_FOURCC fourcc_out, MMP_CHAR* class_name,
                const enum MMP_FOURCC *support_fourcc_in_list, MMP_S32 support_fourcc_in_count,
                const enum MMP_FOURCC *support_fourcc_out_list, MMP_S32 support_fourcc_out_count);
    virtual ~CMmpEncoder();

    virtual MMP_RESULT Open();
    virtual MMP_RESULT Close();

public:
	inline enum MMP_FOURCC get_fourcc_in() { return m_fourcc_in; }
    inline enum MMP_FOURCC get_fourcc_out() { return m_fourcc_out; }

    MMP_RESULT set_fourcc_in(enum MMP_FOURCC fourcc);
    MMP_RESULT set_fourcc_out(enum MMP_FOURCC fourcc);
    
    inline const MMP_CHAR* get_fourcc_in_name() { return m_fourcc_in_name; }
    inline const MMP_CHAR* get_fourcc_out_name() { return m_fourcc_out_name; }
    inline const MMP_CHAR* get_class_name() { return m_class_name; }

    MMP_BOOL is_support_fourcc_in(enum MMP_FOURCC fourcc);
    inline MMP_S32  get_support_fourcc_in_count() { return m_support_fourcc_in_count; }
    inline enum MMP_FOURCC  get_support_fourcc_in(MMP_S32 idx) { return m_support_fourcc_in_list[idx]; }
    
    MMP_BOOL is_support_fourcc_out(enum MMP_FOURCC fourcc);
    inline MMP_S32  get_support_fourcc_out_count() { return m_support_fourcc_out_count; }
    inline enum MMP_FOURCC  get_support_fourcc_out(MMP_S32 idx) { return m_support_fourcc_out_list[idx]; }
      
};

#endif

