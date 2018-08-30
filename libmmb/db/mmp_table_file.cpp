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

#include "mmp_table_file.hpp"

/**********************************************************
class members
**********************************************************/

mmp_table_file::mmp_table_file(struct mmp_table::create_config *p_create_config) : mmp_table(p_create_config, sizeof(record) ) {
	
}

mmp_table_file::~mmp_table_file() {

	
}

MMP_RESULT mmp_table_file::open() {

	MMP_RESULT mmpResult = MMP_SUCCESS;

    mmpResult = mmp_table::open();


	return mmpResult;
}

MMP_RESULT mmp_table_file::close() {
	
    MMP_RESULT mmpResult = MMP_SUCCESS;

    mmpResult = mmp_table::close();

	return mmpResult;
}
