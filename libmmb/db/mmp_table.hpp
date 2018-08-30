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

#ifndef MMP_TABLE_HPP__
#define MMP_TABLE_HPP__

#include "MmpDefine.h"


class mmp_table {

public:
    enum {
        _MAX_FILE_NAME_LEN = 1024,
        _MAX_EXT_NAME_LEN = 32
    };

    enum type {
        _DEMUXER=0,
        _FILE,
        _TYPE_MAX_COUNT
    };

public:
    struct create_config {
         MMP_CHAR root_path[256];
         enum mmp_table::type type;
    };

public:
	static class mmp_table* create_object(struct mmp_table::create_config *p_create_config);
	static MMP_RESULT destroy_object(class mmp_table* p_obj);

private:
    struct table_header {
        enum type type;
        MMP_S32 record_size;
        MMP_CHAR table_name[32];
        MMP_CHAR reserved[1024];
    };
private:
    struct create_config m_create_config;
    FILE* m_fp;
    MMP_S32 m_record_size;

protected:
    mmp_table(struct mmp_table::create_config *p_create_config, MMP_S32 record_size);
    virtual ~mmp_table();

  	virtual MMP_RESULT open();
	virtual MMP_RESULT close();

    MMP_RESULT add_record(MMP_U8* record);

    MMP_RESULT get_record(MMP_S32 index, MMP_U8* record);
    MMP_RESULT get_record_first(MMP_U8* record);
    MMP_RESULT get_record_next(MMP_U8* record);

public:
    MMP_S32 get_record_count();
};

#endif

