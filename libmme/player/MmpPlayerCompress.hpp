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

#ifndef _MMPPLAYERCOMPRESS_HPP__
#define _MMPPLAYERCOMPRESS_HPP__

#include "MmpPlayer.hpp"
#include <tinycompress/tinycompress.h>
#include <sound/compress_params.h>

#if (MMP_PLATFORM==MMP_PLATFORM_WIN32)
#define CMmpPlayerCompress_PCM_DUMP 0
#else
#define CMmpPlayerCompress_PCM_DUMP 0
#endif

#if (CMmpPlayerCompress_PCM_DUMP == 1)
#define CMmpPlayerCompress_PCM_DUMPNAME "CMmpPlayerCompress.Dump.pcm"
#if (MMP_PLATFORM==MMP_PLATFORM_WIN32)
#define CMmpPlayerCompress_PCM_FILENAME "d:\\work\\"CMmpPlayerCompress_PCM_DUMPNAME
#elif (MMP_PLATFORM==MMP_PLATFORM_ANDROID)
#define CMmpPlayerCompress_PCM_FILENAME "/data/"CMmpPlayerCompress_PCM_DUMPNAME
#endif
#endif


class CMmpPlayerCompress : public CMmpPlayer
{
friend class CMmpPlayer;

private:
    CMmpDemuxer* m_pDemuxer;

    struct compr_config m_config;
	struct snd_codec m_codec;
	struct compress *m_compress;

    unsigned char *m_stream_buffer;
    int m_stream_index;
    int m_stream_max_size;
    
    class mmp_buffer_audiostream *m_p_buf_as;
    
#if (CMmpPlayerCompress_PCM_DUMP == 1)
    void* m_pcm_dump_hdl;
#endif


protected:
    CMmpPlayerCompress(CMmpPlayerCreateProp* pPlayerProp);
    virtual ~CMmpPlayerCompress();

    virtual MMP_RESULT Open();
    virtual MMP_RESULT Close();

    virtual void Service();
    int compress_write_mme(struct compress *compress, const void *buf, unsigned int size);
};

#endif


