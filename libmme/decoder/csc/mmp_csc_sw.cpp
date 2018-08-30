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
#include "MmpImageTool.hpp"
#include "MmpUtil.hpp"


/**********************************************************
class mmp_csc_sw
**********************************************************/

mmp_csc_sw::mmp_csc_sw() {

}

mmp_csc_sw::~mmp_csc_sw() {

}

MMP_RESULT mmp_csc_sw::open() {

    return MMP_SUCCESS;
}

MMP_RESULT mmp_csc_sw::close() {

    return MMP_SUCCESS;
}
    
MMP_RESULT mmp_csc_sw::convert(class mmp_buffer_videoframe* p_buf_vf_src, class mmp_buffer_videoframe* p_buf_vf_dst) {

    MMP_U8 *Y, *U, *V;
    MMP_S32 luma_stride, chroma_stride;

    MMP_U32 t1, t2;

    Y = (MMP_U8 *)p_buf_vf_dst->get_buf_vir_addr_y();
    U = (MMP_U8 *)p_buf_vf_dst->get_buf_vir_addr_u();
    V = (MMP_U8 *)p_buf_vf_dst->get_buf_vir_addr_v();

    luma_stride = p_buf_vf_dst->get_stride_luma();
    chroma_stride = p_buf_vf_dst->get_stride_chroma();
            
    t1 = CMmpUtil::GetTickCount();
    CMmpImageTool::ConvertRGBtoYUV420M((MMP_U8*)p_buf_vf_src->get_buf_vir_addr(), 
                                            p_buf_vf_src->get_pic_width(), p_buf_vf_src->get_pic_height(), p_buf_vf_src->get_fourcc(),
                                            Y, U, V,
                                            luma_stride, chroma_stride, chroma_stride);
    t2 = CMmpUtil::GetTickCount();

    MMPDEBUGMSG(1, (TEXT(" convert dur : %d  "), t2-t1 ));

    return MMP_SUCCESS;
}
