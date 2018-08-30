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

#ifndef MMP_DB_HPP__
#define MMP_DB_HPP__

#include "MmpDefine.h"
#include "mmp_db_def.h"

class mmp_db {

public:
    struct create_config {
         MMP_CHAR root_path[256];
         MMP_CHAR media_path[32];
    };

public:
	static class mmp_db* create_object(struct mmp_db::create_config *p_create_config);
	static MMP_RESULT destroy_object(class mmp_db* p_obj);

private:
    struct create_config m_create_config;

protected:
    mmp_db(struct mmp_db::create_config *p_create_config);
    virtual ~mmp_db();

    virtual MMP_RESULT open() = 0;
	virtual MMP_RESULT close() = 0;
    
public:
    const char* get_root_path() { return (const char*)m_create_config.root_path; }
    const char* get_media_path() { return (const char*)m_create_config.media_path; }
    virtual MMP_RESULT update_table_file() = 0;
};

#endif