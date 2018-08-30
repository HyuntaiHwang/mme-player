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

#include "mmp_db_ex1.hpp"
#include "MmpUtil.hpp"

/**********************************************************
class members
**********************************************************/

static const MMP_CHAR *kValidExtensions[] = {
    //video
    "mp4", "m4a", "3gp", "3gpp", "3g2", "3gpp2", "ogg", 
    "rm", "rmvb",
    "ota", "mkv", "mka", "webm", "ts", "tp",
    "flac", "avi", "mpeg", 
    "mpg", "awb", "mpga","wmv", "asf", 
    "mov", "flv", "tp", "k3g", "skm",
    
    //audio 
    "mp3", "mid", "wma", "aac", "wav", "amr", "midi", "mp2", 

    //raw stream
    "h264", "264",

    //picture 
    "jpg", "jpeg", "bmp", "png", "gif"
};

mmp_db_ex1::mmp_db_ex1(struct mmp_db::create_config *p_create_config) : mmp_db(p_create_config)
{
    MMP_S32 i;

    for(i = 0; i < mmp_table::_TYPE_MAX_COUNT; i++) {
        m_p_table[i] = NULL;
    }
}

mmp_db_ex1::~mmp_db_ex1() {
    	
}

MMP_RESULT mmp_db_ex1::open() {

	MMP_RESULT mmpResult = MMP_SUCCESS;
    struct mmp_table::create_config table_create_config;
    MMP_S32 i, j;
    class mmp_table_demuxer *p_table_demuxer;

    strcpy(table_create_config.root_path, this->get_root_path());

    /* create table */
    if(mmpResult == MMP_SUCCESS) {

        for(i = 0; i < mmp_table::_TYPE_MAX_COUNT; i++) {
            table_create_config.type = (enum mmp_table::type)i;
            m_p_table[i] = mmp_table::create_object(&table_create_config);
            if(m_p_table[i] == NULL) {
                mmpResult = MMP_FAILURE;
                break;
            }
        }
    }


    /* create table_demuxer */
    if(mmpResult == MMP_SUCCESS) {
        
        p_table_demuxer = (class mmp_table_demuxer*)m_p_table[mmp_table::_DEMUXER];
        j = sizeof(kValidExtensions)/sizeof(kValidExtensions[0]);
        for(i = 0; i < j; i++) {
            p_table_demuxer->add_ext(kValidExtensions[i]);
        }
        
    }

    
	return mmpResult;
}

MMP_RESULT mmp_db_ex1::close() {

    MMP_S32 i;

    for(i = 0; i < mmp_table::_TYPE_MAX_COUNT; i++) {
        if(m_p_table[i] != NULL) {
            mmp_table::destroy_object(m_p_table[i]);
            m_p_table[i] = NULL;
        }
    }
	
	return MMP_SUCCESS;
}

MMP_RESULT mmp_db_ex1::update_table_file() {
    
    MMP_S32 file_array_max = 1024*10;
    MMP_S32 file_size_max = 1024;
    MMP_CHAR* file_array;
    class mmp_table_demuxer *p_table_demuxer = (class mmp_table_demuxer *)m_p_table[mmp_table::_DEMUXER];
    
    MMP_S32 ext_array_max = p_table_demuxer->get_record_count();
    MMP_S32 ext_size_max = 32;
    MMP_CHAR* ext_array, *ext;

    MMP_S32 ext_count, file_cnt;
    const MMP_CHAR* ext_db;
    
    // Select Media File 
    file_array = (char*)malloc(file_array_max*file_size_max);
    ext_array = (char*)malloc(ext_array_max*ext_size_max);

    ext_count = 0;
    ext_db = p_table_demuxer->get_extname_first();
    while(ext_db != NULL) {
        ext=&ext_array[ext_count*ext_size_max];  
        strcpy(ext, ext_db);
        ext_db = p_table_demuxer->get_extname_next();
        ext_count++;
    }

    file_cnt = CMmpUtil::GetFileList((char*)this->get_media_path(), 
                                     ext_array, ext_count, ext_size_max, 
                                     file_array, file_array_max, file_size_max);



    free(file_array);
    free(ext_array);

    return MMP_SUCCESS;
}
