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

#include "mmp_csc_sw.hpp"
//#include "mmp_csc_mscaler.hpp"
//#include "mmp_csc_neon.hpp"

/**********************************************************
create/destroy object
**********************************************************/

class mmp_csc* mmp_csc::create_object(struct mmp_csc::create_config *p_create_config) {

	class mmp_csc* p_obj = NULL;

    //p_obj = new mmp_csc_sw;
	//p_obj = new mmp_csc_mscaler;
    switch(p_create_config->type) {
        //case mmp_csc::NEON :  p_obj = new mmp_csc_neon; break;
        case mmp_csc::SW :  p_obj = new mmp_csc_sw; break;
    }
	if(p_obj!=NULL) {
		if(p_obj->open() != MMP_SUCCESS) {
			p_obj->close();
			delete p_obj;
			p_obj = NULL;
		}
	}

    return p_obj;
}

MMP_RESULT mmp_csc::destroy_object(class mmp_csc* p_obj) {

	if(p_obj != NULL) {
        p_obj->close();
        delete p_obj;
    }

    return MMP_SUCCESS;
}

/**********************************************************
class mmp_csc
**********************************************************/

mmp_csc::mmp_csc() {

}

mmp_csc::~mmp_csc() {

}



