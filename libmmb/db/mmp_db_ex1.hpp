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

#ifndef MMP_DB_EX1_HPP__
#define MMP_DB_EX1_HPP__

#include "mmp_db.hpp"
#include "mmp_table_demuxer.hpp"
#include "mmp_table_file.hpp"

class mmp_db_ex1 : public mmp_db {

friend class mmp_db;
    
private:
    class mmp_table* m_p_table[mmp_table::_TYPE_MAX_COUNT];
    
protected:
    
	mmp_db_ex1(struct mmp_db::create_config *p_create_config);
	virtual ~mmp_db_ex1();

	virtual MMP_RESULT open();
	virtual MMP_RESULT close();

public:
    virtual MMP_RESULT update_table_file();
};


#endif
