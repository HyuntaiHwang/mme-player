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

#include "mmp_db.hpp"
#include "mmp_db_ex1.hpp"

/**********************************************************
create/destroy object
**********************************************************/

class mmp_db* mmp_db::create_object(struct mmp_db::create_config *p_create_config) {

	class mmp_db* p_obj = NULL;

    p_obj = new mmp_db_ex1(p_create_config);
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

MMP_ERRORTYPE mmp_db::destroy_object(class mmp_db* p_obj) {

	if(p_obj != NULL) {
        p_obj->close();
        delete p_obj;
    }

    return MMP_ErrorNone;
}


/**********************************************************
class members
**********************************************************/

mmp_db::mmp_db(struct mmp_db::create_config *p_create_config) :
m_create_config(*p_create_config)
{


}

mmp_db::~mmp_db() {

}

/*
MMP_RESULT mmp_db::open() {
	
	return MMP_SUCCESS;
}

MMP_RESULT mmp_db::close() {
	
	return MMP_SUCCESS;
}
*/
