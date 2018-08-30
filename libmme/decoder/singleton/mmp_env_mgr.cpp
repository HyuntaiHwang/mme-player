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

#include "mmp_env_mgr.hpp"
#include "MmpUtil.hpp"

/**********************************************************
create/destroy object
**********************************************************/

class mmp_env_mgr* mmp_env_mgr::s_p_instance = NULL;

MMP_RESULT mmp_env_mgr::create_instance() {

    MMP_RESULT mmpResult;
	class mmp_env_mgr* p_obj = NULL;

	p_obj = new class mmp_env_mgr;
	if(p_obj!=NULL) {
        
		if(p_obj->open( ) != MMP_SUCCESS)
		{
			p_obj->close();
			delete p_obj;
			p_obj = NULL;
		}
	}

    if(p_obj != NULL) {
        mmp_env_mgr::s_p_instance = p_obj;
        mmpResult = MMP_SUCCESS;
    }
    else {
        mmpResult = MMP_FAILURE;
    }

    return mmpResult;
}

MMP_RESULT mmp_env_mgr::destroy_instance() {

    class mmp_env_mgr* p_obj = mmp_env_mgr::s_p_instance;

	if(p_obj != NULL) {
        p_obj->close();
        delete p_obj;

        mmp_env_mgr::s_p_instance = NULL;
    }

    return MMP_SUCCESS;
}

class mmp_env_mgr* mmp_env_mgr::get_instance() {
    return mmp_env_mgr::s_p_instance;
}

/**********************************************************
env define
**********************************************************/

/* VPU */
#if 0
#define GDM_VPU_BASE                 (0xED310000)
#define GDM_VPU_SIZE                 (SZ_64K)
#define GDM_VPU_IRQ                  (IRQ_CODEC)
#endif

#define MMP_VPU_DEVICE_NAME "/dev/vpu"
#define MMP_VPU_REG_BASE		0x17310000
#define MMP_VPU_REG_SIZE		0x10000
#define MMP_VPU_OCMEM_BASE		0x17400000
#define MMP_VPU_OCMEM_SIZE		0x20000

/* JPU */
#if 0
#define GDM_JPU_BASE                 (0xED300000)
#define GDM_JPU_SIZE                 (SZ_64K)
#define GDM_JPU_IRQ                  (IRQ_JPEG)
#endif
#define MMP_JPU_DEVICE_NAME "/dev/jpu"
#define MMP_JPU_REG_BASE		0xED300000
#define MMP_JPU_REG_SIZE		(1024*64)
#define MMP_JDI_DRAM_PHYSICAL_BASE	0x00
#define MMP_JDI_DRAM_PHYSICAL_SIZE	(128*1024*1024)

#define MMP_SHARED_MEM_VPU_KEY      0xAAAA9829
#define MMP_SHARED_MEM_JPU_KEY      0xBBBB9829
#define MMP_EXTERNAL_MUTEX_MEM_VPU_KEY      0xCCCC9829
#define MMP_EXTERNAL_MUTEX_MEM_JPU_KEY      0xDDDD9829

/**********************************************************
class members
**********************************************************/

mmp_env_mgr::mmp_env_mgr() :

m_vpu_fd(-1)
,m_vpu_instance_pool_buffer(NULL)
,m_vpu_reg_vir_addr(0)
,m_vpu_common_buffer(NULL)

{
    MMP_S32 i;

    /* int env_int */
    for(i = 0; i < ENV_UINT_MAX; i++) {
        m_env_uint[i] = 0;
    }

    /* int env_char */
    for(i = 0; i < ENV_CHAR_MAX; i++) {
        m_env_char[i][0] = '\0';
    }

    /* Init VPU Env Value */
    this->set_uint(ENV_UINT_VPU_SHM_KEY, MMP_SHARED_MEM_VPU_KEY);  /* VPU Shared Memory Key */
    this->set_char(ENV_CHAR_VPU_DRV_NAME, MMP_VPU_DEVICE_NAME);
    this->set_uint(ENV_UINT_VPU_REG_PHY_ADDR, MMP_VPU_REG_BASE); /* VPU Register Phy Addr */
    this->set_uint(ENV_UINT_VPU_REG_SIZE, MMP_VPU_REG_SIZE); /* VPU Register Size */
    this->set_uint(ENV_UINT_VPU_EXTERNAL_MUTEX_KEY, MMP_EXTERNAL_MUTEX_MEM_VPU_KEY);
    this->set_uint(ENV_UINT_VPU_OCMEM_PHY_ADDR, MMP_VPU_OCMEM_BASE);
    this->set_uint(ENV_UINT_VPU_COMEM_SIZE, MMP_VPU_OCMEM_SIZE);
    
    /* Init JPU Env Value */
    this->set_uint(ENV_UINT_JPU_SHM_KEY, MMP_SHARED_MEM_JPU_KEY);  /* JPU Shared Memory Key */
    this->set_uint(ENV_UINT_JPU_REG_PHY_ADDR, MMP_JPU_REG_BASE); /* JPU Register Phy Addr */
    this->set_uint(ENV_UINT_JPU_REG_SIZE, MMP_JPU_REG_SIZE); /* JPU Register Size */
    this->set_char(ENV_CHAR_JPU_DRV_NAME, MMP_JPU_DEVICE_NAME);
    this->set_uint(ENV_UINT_JPU_EXTERNAL_MUTEX_KEY, MMP_EXTERNAL_MUTEX_MEM_JPU_KEY);
    
}

mmp_env_mgr::~mmp_env_mgr() {

}

MMP_RESULT mmp_env_mgr::open() {
    return MMP_SUCCESS;
}
 
MMP_RESULT mmp_env_mgr::close() {
    return MMP_SUCCESS;
}


extern "C" {
    
unsigned long mmp_env_mgr_get_vpu_ocmmem_phy_addr(void) {
    return mmp_env_mgr::get_instance()->get_uint(mmp_env_mgr::ENV_UINT_VPU_OCMEM_PHY_ADDR);
}

int mmp_env_mgr_get_vpu_ocmmem_size(void) {
    return (MMP_S32)mmp_env_mgr::get_instance()->get_uint(mmp_env_mgr::ENV_UINT_VPU_COMEM_SIZE);
}

}
