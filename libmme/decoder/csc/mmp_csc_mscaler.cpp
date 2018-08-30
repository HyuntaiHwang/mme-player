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

#include "mmp_csc_mscaler.hpp"


/**********************************************************
class mmp_csc_mscaler
**********************************************************/

mmp_csc_mscaler::mmp_csc_mscaler() :
m_hScaler(NULL)
{

}

mmp_csc_mscaler::~mmp_csc_mscaler() {

}

MMP_RESULT mmp_csc_mscaler::open() {

    m_hScaler = MScalerOpen("/dev/video8");
    if(m_hScaler == NULL) {
        MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[mmp_csc_mscaler::open] FAIL: MScalerOpen")));
        return MMP_FAILURE;
    }

    return MMP_SUCCESS;
}

MMP_RESULT mmp_csc_mscaler::close() {

    if(m_hScaler != NULL) {
        MScalerClose(m_hScaler);
    }
        
    return MMP_SUCCESS;
}
 
MMP_RESULT mmp_csc_mscaler::start() {

    return MMP_SUCCESS;
}
	
MMP_RESULT mmp_csc_mscaler::stop() {

    if(m_hScaler!=NULL) MScalerStop(m_hScaler);
        
    return MMP_SUCCESS;
}
    
MMP_RESULT mmp_csc_mscaler::convert(class mmp_buffer_videoframe* p_buf_vf_src, class mmp_buffer_videoframe* p_buf_vf_dst) {

    MScalerHandle hScaler = this->m_hScaler;
    struct MScalerImageFormat srcFormat={0}, dstFormat={0} ; 
    struct MScalerImageData srcData, dstData;
    MMP_S32 i;
        
    srcFormat.width = p_buf_vf_src->get_pic_width();
    srcFormat.height = p_buf_vf_src->get_pic_height();
    srcFormat.pixelformat = p_buf_vf_src->get_fourcc();
    srcFormat.crop.left=0;
    srcFormat.crop.top=0;
    srcFormat.crop.width = srcFormat.width;
    srcFormat.crop.height = srcFormat.height;
    
    srcData.planes = p_buf_vf_src->get_plane_count();
    for(i = 0; i < srcData.planes; i++) {
        srcData.plane[i].fd = p_buf_vf_src->get_buf_shared_fd(i);
        srcData.plane[i].length = p_buf_vf_src->get_buf_size(i);
        srcData.plane[i].base = (void*)p_buf_vf_src->get_buf_vir_addr(i);
    }
    srcData.acquireFenceFd = -1;
    srcData.releaseFenceFd = -1;
    srcData.priv = NULL;
    
    dstFormat.width = p_buf_vf_dst->get_pic_width();
    dstFormat.height = p_buf_vf_dst->get_pic_height();
    dstFormat.pixelformat = p_buf_vf_dst->get_fourcc();
    dstFormat.crop.left=0;
    dstFormat.crop.top=0;
    dstFormat.crop.width = dstFormat.width;
    dstFormat.crop.height = dstFormat.height;
    
    dstData.planes = p_buf_vf_dst->get_plane_count();
    for(i = 0; i < dstData.planes; i++) {
        dstData.plane[i].fd = p_buf_vf_dst->get_buf_shared_fd(i);
        dstData.plane[i].length = p_buf_vf_dst->get_buf_size(i);
        dstData.plane[i].base = (void*)p_buf_vf_dst->get_buf_vir_addr();
    }
    dstData.acquireFenceFd = -1;
    dstData.releaseFenceFd = -1;
    dstData.priv = NULL;
    
    MScalerLock(hScaler, -1);

    MScalerSetImageFormat(hScaler, &srcFormat, &dstFormat);
    MScalerSetImageData(hScaler, &srcData, &dstData);
    
    MScalerRun(hScaler);
    MScalerWaitDone(hScaler, -1);
    
    MScalerUnlock(hScaler);

    return MMP_SUCCESS;
}
