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

#include "MmpDefine.h"
#include "mme_c_api.h"
#include "MmpDemuxer.hpp"


int mme_demuxer_get_audio_prop(char* mediafilename, struct mme_audio_format* p_audio_format) {
     
    CMmpDemuxer* pDemuxer;
    struct MmpDemuxerCreateConfig demuxer_create_config;
    int iret = 0;
    struct mmp_audio_format mmp_af;

    strcpy((char*)demuxer_create_config.filename, mediafilename);
    pDemuxer = CMmpDemuxer::CreateObject(&demuxer_create_config);
    if(pDemuxer == NULL) {
        iret = -1;
    }
    else {
        mmp_af = pDemuxer->get_audio_format();
        memcpy(p_audio_format, &mmp_af, sizeof(mmp_af));
        iret = 0;
        CMmpDemuxer::DestroyObject(pDemuxer);
    }

    return iret;
}