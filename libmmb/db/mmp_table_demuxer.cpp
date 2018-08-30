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

#include "mmp_table_demuxer.hpp"
#include "MmpUtil.hpp"

/**********************************************************
class members
**********************************************************/

mmp_table_demuxer::mmp_table_demuxer(struct mmp_table::create_config *p_create_config) : mmp_table(p_create_config, sizeof(record) ) {
	
}

mmp_table_demuxer::~mmp_table_demuxer() {

	
}

MMP_RESULT mmp_table_demuxer::open() {

	MMP_RESULT mmpResult = MMP_SUCCESS;

    mmpResult = mmp_table::open();


	return mmpResult;
}

MMP_RESULT mmp_table_demuxer::close() {
	
    MMP_RESULT mmpResult = MMP_SUCCESS;

    mmpResult = mmp_table::close();

	return mmpResult;
}

MMP_RESULT mmp_table_demuxer::add_ext(const MMP_CHAR* ext) {

    MMP_RESULT mmpResult; 
    MMP_CHAR ext1[32], ext2[32];
    MMP_BOOL is_register;
    MMP_S32 record_count;

    strcpy(ext1, ext);
    CMmpUtil::MakeLower(ext1);
    
    record_count = 0;
    is_register = MMP_FALSE;
    mmpResult = this->get_record_first((MMP_U8*)&m_record_tmp);
    while(mmpResult == MMP_SUCCESS) {
    
        record_count++;

        strcpy(ext2, m_record_tmp.extname);
        CMmpUtil::MakeLower(ext2);

        if(strcmp(ext1, ext2) == 0) {
            is_register = MMP_TRUE;
            break;
        }

        mmpResult = this->get_record_next((MMP_U8*)&m_record_tmp);
    }

    /* new extension */
    if(is_register == MMP_FALSE) {
        
        memset(&m_record_tmp, 0x00, sizeof(m_record_tmp));
        m_record_tmp.id = record_count;
        strcpy(m_record_tmp.extname, ext1);

        this->add_record((MMP_U8*)&m_record_tmp);
    }

    return MMP_SUCCESS;
}

const MMP_CHAR* mmp_table_demuxer::get_extname_first() {
    
    MMP_RESULT mmpResult;
    const MMP_CHAR* extname = NULL;

    mmpResult = this->get_record_first((MMP_U8*)&m_record_tmp);
    if(mmpResult == MMP_SUCCESS) {
        extname = m_record_tmp.extname;
    }
    
    return extname;
}
    
const MMP_CHAR* mmp_table_demuxer::get_extname_next() {

    MMP_RESULT mmpResult;
    const MMP_CHAR* extname = NULL;

    mmpResult = this->get_record_next((MMP_U8*)&m_record_tmp);
    if(mmpResult == MMP_SUCCESS) {
        extname = m_record_tmp.extname;
    }
    
    return extname;
}

