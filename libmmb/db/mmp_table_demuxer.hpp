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

#ifndef MMP_TABLE_DEMUXER_HPP__
#define MMP_TABLE_DEMUXER_HPP__

#include "mmp_table.hpp"


class mmp_table_demuxer : public mmp_table {

friend class mmp_table;

public:
    struct record {
        MMP_S32  id;
        MMP_CHAR extname[_MAX_EXT_NAME_LEN];
        MMP_CHAR reserved[2048];
    };

private:
    struct record m_record_tmp;
    
protected:
    
	mmp_table_demuxer(struct mmp_table::create_config *p_create_config);
	virtual ~mmp_table_demuxer();

	virtual MMP_RESULT open();
	virtual MMP_RESULT close();

public:
    MMP_RESULT add_ext(const MMP_CHAR* ext);
    const MMP_CHAR* get_extname_first();
    const MMP_CHAR* get_extname_next();
};


#endif
