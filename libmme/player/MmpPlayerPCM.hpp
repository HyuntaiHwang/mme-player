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

#ifndef MMPPLAYERPCM_HPP__
#define MMPPLAYERPCM_HPP__

#include "MmpPlayer.hpp"

class CMmpPlayerPCM : public CMmpPlayer
{
friend class CMmpPlayer;

private:
    void* m_pcm_hdl;
    CMmpRendererAudio* m_pRendererAudio;
    class mmp_buffer_audioframe* m_p_buf_af;
    MMP_S64 m_play_duration;
    

protected:
    CMmpPlayerPCM(CMmpPlayerCreateProp* pPlayerProp);
    virtual ~CMmpPlayerPCM();

    virtual MMP_RESULT Open();
    virtual MMP_RESULT Close();

    virtual void Service();
};

#endif


