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

#include "mmp_table.hpp"
#include "mmp_table_file.hpp"
#include "mmp_table_demuxer.hpp"

/**********************************************************
create/destroy object
**********************************************************/

class mmp_table* mmp_table::create_object(struct mmp_table::create_config *p_create_config) {

	class mmp_table* p_obj = NULL;

    switch(p_create_config->type) {
        case mmp_table::_FILE :  p_obj = new class mmp_table_file(p_create_config); break;
        case mmp_table::_DEMUXER :  p_obj = new class mmp_table_demuxer(p_create_config); break;
    }

	if(p_obj!=NULL) {
        
		if(p_obj->open( ) != MMP_ErrorNone)    
		{
			p_obj->close();
			delete p_obj;
			p_obj = NULL;
		}
	}

    return p_obj;
}

MMP_ERRORTYPE mmp_table::destroy_object(class mmp_table* p_obj) {

	if(p_obj != NULL) {
        p_obj->close();
        delete p_obj;
    }

    return MMP_ErrorNone;
}

/**********************************************************
class members
**********************************************************/

mmp_table::mmp_table(struct mmp_table::create_config *p_create_config, MMP_S32 record_size) :
m_fp(NULL)
,m_record_size(record_size)
{

    memcpy(&m_create_config, p_create_config, sizeof(m_create_config));
}

mmp_table::~mmp_table() {

}


MMP_RESULT mmp_table::open() {

    MMP_RESULT mmpResult = MMP_SUCCESS;
    MMP_CHAR tablefilename[256];
    MMP_CHAR tablename[256];
    MMP_S32 i;
    MMP_BOOL is_init;
    struct mmp_table::table_header table_header;
    FILE* fp;

    /* composite table file name */
    strcpy(tablefilename, this->m_create_config.root_path);
    strcat(tablefilename, MMP_FILE_DEVIDER_STRING);
    switch(this->m_create_config.type) {
        
        case _DEMUXER: strcpy(tablename, "demuxer.tbl"); break;
        case _FILE:  strcpy(tablename, "file.tbl"); break;
        default: strcpy(tablename, "unknown.tbl"); break;
    }
    strcat(tablefilename, tablename);

    fp = fopen(tablefilename, "rb");
    if(fp == NULL) {
        fp = fopen(tablefilename, "wb");
        if(fp == NULL) {
            mmpResult = MMP_FAILURE;
        }
        else {
            fclose(fp);
        }
    }
    else {
        fclose(fp);
    }

    if(mmpResult == MMP_SUCCESS) {

        m_fp = fopen(tablefilename, "rb+");
	    if(m_fp == NULL) {
            mmpResult = MMP_FAILURE;
        }
        else {
            fseek(m_fp, 0, SEEK_END);
            i = (MMP_S32)ftell(m_fp);
            fseek(m_fp, 0, SEEK_SET);
            
            is_init = MMP_TRUE;

            /* check initial file */
            if(i >= sizeof(mmp_table::table_header) ) {
                fread(&table_header, 1, sizeof(mmp_table::table_header), m_fp);
                if(table_header.type != this->m_create_config.type) is_init = MMP_FALSE;
                if(table_header.record_size != m_record_size) is_init = MMP_FALSE;
                if(strcmp(table_header.table_name,  tablename) != 0) is_init = MMP_FALSE;
            }
            else {
                is_init = MMP_FALSE;
            }
        
            /* initial file */
            if(is_init == MMP_FALSE) {
                memset(&table_header, 0x00, sizeof(mmp_table::table_header));
                table_header.type = this->m_create_config.type;
                table_header.record_size = m_record_size;
                strcpy(table_header.table_name, tablename);

                fseek(m_fp, 0, SEEK_SET);
                fwrite(&table_header, 1, sizeof(mmp_table::table_header), m_fp);
            }
        }

    }

	return mmpResult;
}

MMP_RESULT mmp_table::close() {
	
    if(m_fp!=NULL) {
        fclose(m_fp);
    }

	return MMP_SUCCESS;
}

MMP_S32 mmp_table::get_record_count() {
    
    MMP_S32 sz, i;
    MMP_S32 cur_pos;
    MMP_S32 cnt = 0;

    cur_pos = (MMP_S32)ftell(m_fp);

    fseek(m_fp, 0, SEEK_END);
    sz = ftell(m_fp);
    i = sizeof(mmp_table::table_header);
    sz -= i;
    cnt = sz / m_record_size;

    fseek(m_fp, cur_pos, SEEK_SET);

    return cnt;
}

MMP_RESULT mmp_table::add_record(MMP_U8* record) {

    MMP_RESULT mmpResult = MMP_FAILURE;
    MMP_S32 sz;

    fseek(m_fp, 0, SEEK_END);

    sz = fwrite(record, 1, m_record_size, m_fp);
    if(sz == m_record_size) {
        mmpResult = MMP_SUCCESS;
    }

    return mmpResult;
}

MMP_RESULT mmp_table::get_record(MMP_S32 index, MMP_U8* record) {

    MMP_RESULT mmpResult = MMP_FAILURE;
    MMP_S32 sz;

    fseek(m_fp, sizeof(mmp_table::table_header) + index*m_record_size, SEEK_SET);

    sz = fread(record, 1, m_record_size, m_fp);
    if(sz == m_record_size) {
        mmpResult = MMP_SUCCESS;
    }

    return mmpResult;
}
    
MMP_RESULT mmp_table::get_record_first(MMP_U8* record) {

    MMP_RESULT mmpResult = MMP_FAILURE;
    MMP_S32 sz;

    fseek(m_fp, sizeof(mmp_table::table_header), SEEK_SET);

    sz = fread(record, 1, m_record_size, m_fp);
    if(sz == m_record_size) {
        mmpResult = MMP_SUCCESS;
    }

    return mmpResult;
}
    
MMP_RESULT mmp_table::get_record_next(MMP_U8* record) {

    MMP_RESULT mmpResult = MMP_FAILURE;
    MMP_S32 sz;

    sz = fread(record, 1, m_record_size, m_fp);
    if(sz == m_record_size) {
        mmpResult = MMP_SUCCESS;
    }

    return mmpResult;
}

