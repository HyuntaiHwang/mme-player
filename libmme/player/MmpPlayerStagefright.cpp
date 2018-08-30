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


#include "MmpPlayerStagefright.hpp"
#include "MmpUtil.hpp"

#if (MMP_PLATFORM == MMP_PLATFORM_ANDROID || MMP_PLATFORM == MMP_PLATFORM_WIN32)

#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <media/stagefright/MetaData.h>

namespace android { 
/////////////////////////////////////////////////////////////
//CMmpPlayerStagefright Member Functions

CMmpPlayerStagefright::CMmpPlayerStagefright(CMmpPlayerCreateProp* pPlayerProp) : CMmpPlayer(pPlayerProp, "Stagefright")
,m_fd(-1)
{
    

}

CMmpPlayerStagefright::~CMmpPlayerStagefright()
{
    
    
}

MMP_RESULT CMmpPlayerStagefright::Open()
{
    MMP_RESULT mmpResult = MMP_SUCCESS;
    int status, file_len;
    Parcel parcel;
    Rect layerStackRect;
    Rect displayRect;
    
    sp<IBinder> display(SurfaceComposerClient::getBuiltInDisplay(ISurfaceComposer::eDisplayIdMain));

    //DisplayState& s(getDisplayStateLocked(display));
    //s.orientation = orientation;
    //getDisplayStateUnLocked(display)
    //SurfaceComposerClient::setOrientation(0, DISPLAY_ORIENTATION_90, 0);
    SurfaceComposerClient::getDisplayInfo(display, &m_display_info);
    MMPDEBUGMSG(1, (TEXT("[CMmpPlayerStagefright::Open] ln=%d before rotate, display  w=%d h=%d ori=%d  "), __LINE__,
                 m_display_info.w, 
                 m_display_info.h,
                 m_display_info.orientation
                 ));

    //layerStackRect = Rect(0,0,m_display_info.h,m_display_info.w);
    //displayRect = Rect(0,0, m_display_info.w, m_display_info.h);

    layerStackRect = Rect(0,0,m_display_info.w, m_display_info.h);
    displayRect = Rect(0,0, m_display_info.h, m_display_info.w);
    //displayRect = Rect(0,0, m_display_info.w, m_display_info.h);

    //SurfaceComposerClient::setDisplayProjection(display, DISPLAY_ORIENTATION_90, layerStackRect, displayRect);
    SurfaceComposerClient::getDisplayInfo(display, &m_display_info);

    MMPDEBUGMSG(1, (TEXT("[CMmpPlayerStagefright::Open] after rotate, display  w=%d h=%d ori=%d  "), 
                 m_display_info.w, 
                 m_display_info.h,
                 m_display_info.orientation
                 ));

    m_fd = open(this->m_create_config.filename, O_RDONLY);
    if(m_fd < 0) {
        mmpResult = MMP_FAILURE;
        MMPDEBUGMSG(1, (TEXT("[CMmpPlayerStagefright::Open] FAIL: file open ")));
    }
    else {
        file_len = lseek(m_fd, 0, SEEK_END);
        lseek(m_fd, 0, SEEK_SET);
        MMPDEBUGMSG(1, (TEXT("[CMmpPlayerStagefright::Open] SUCCESS: file open  len=%d "), file_len ));
    }
#if 0
      fd = open( "myfile.dat",
        O_WRONLY | O_CREAT | O_TRUNC,
        S_IRUSR | S_IWUSR );

    /* read a file that is assumed to exist */

    fd = open( "myfile.dat", O_RDONLY );
#endif

    /* alloc surface */
    if(mmpResult == MMP_SUCCESS) {
        m_SurfaceComposerClient = new SurfaceComposerClient;
        if(m_SurfaceComposerClient == NULL) {

            mmpResult = MMP_FAILURE;
            MMPDEBUGMSG(1, (TEXT("[CMmpPlayerStagefright::Open] FAIL: new SurfaceComposerClient ")));
        }
        else {

            m_SurfaceControl = m_SurfaceComposerClient->createSurface(String8("Test Surface"), 
                                                                    m_display_info.w, 
                                                                    m_display_info.h, 
                                                                    PIXEL_FORMAT_RGBA_8888, 0);
            if(m_SurfaceControl == NULL) {
                mmpResult = MMP_FAILURE;
                MMPDEBUGMSG(1, (TEXT("[CMmpPlayerStagefright::Open] FAIL: m_SurfaceComposerClient->createSurface ")));
            }
            else {
                
                SurfaceComposerClient::openGlobalTransaction();
                m_SurfaceControl->setLayer(0x7fffffff);
                m_SurfaceControl->show();
                SurfaceComposerClient::closeGlobalTransaction();

                //mSurface = pobj->m_SurfaceControl->getSurface()->getIGraphicBufferProducer();
                //pobj->iGraphicBufferProducer = pobj->mSurface->getIGraphicBufferProducer();
            }

        }

    }

    /* setup media player */
    if(mmpResult == MMP_SUCCESS) {
    
        sp<IServiceManager> sm = defaultServiceManager();
        sp<IBinder> binder = sm->getService(String16("media.player"));
        sp<IMediaPlayerService> iMediaPlayerService = interface_cast<IMediaPlayerService>(binder);

        m_iMediaPlayer = iMediaPlayerService->create(NULL);
        if(m_iMediaPlayer == NULL) {
            mmpResult = MMP_FAILURE;
            MMPDEBUGMSG(1, (TEXT("[CMmpPlayerStagefright::Open] FAIL: iMediaPlayerService->create ")));
        }
        else {
        
            //parcel.writeInt32(90);
            //m_iMediaPlayer->setParameter(kKeyRotation, parcel);

            status = m_iMediaPlayer->setDataSource(m_fd, 0, file_len);
            //status = m_iMediaPlayer->setDataSource(this->m_create_config.filename, NULL);
            if(status == 0) {

                m_iMediaPlayer->setVideoSurfaceTexture(m_SurfaceControl->getSurface()->getIGraphicBufferProducer());
                m_iMediaPlayer->setAudioStreamType(AUDIO_STREAM_MUSIC);
                
            }
            else {
                mmpResult = MMP_FAILURE;
                MMPDEBUGMSG(1, (TEXT("[CMmpPlayerStagefright::Open] FAIL: m_iMediaPlayer->setDataSource ")));
            }
        
        }
    
    }

    if(mmpResult == MMP_SUCCESS ) {
        mmpResult = CMmpPlayer::Open();
    }
    
    //m_SurfaceControl = pobj->mComposerClient->createSurface(String8("Test Surface"), display_info.w, display_info.h, PIXEL_FORMAT_RGBA_8888, 0);
    //pobj->mSurface = pobj->mSurfaceControl->getSurface();
    //pobj->iGraphicBufferProducer = pobj->mSurface->getIGraphicBufferProducer();

    return mmpResult;
}


MMP_RESULT CMmpPlayerStagefright::Close()
{
    Rect layerStackRect;
    Rect displayRect;
    

    CMmpPlayer::Close();

    MMPDEBUGMSG(1, (TEXT("[CMmpPlayerStagefright::Close] ln=%d "), __LINE__ ));

    if(m_iMediaPlayer != NULL) {
        m_iMediaPlayer->stop();
        m_iMediaPlayer.clear();
    }

    MMPDEBUGMSG(1, (TEXT("[CMmpPlayerStagefright::Close] ln=%d "), __LINE__ ));

    if(m_fd >= 0) {
        close(m_fd);
        m_fd = -1;
    }

    MMPDEBUGMSG(1, (TEXT("[CMmpPlayerStagefright::Close] ln=%d "), __LINE__ ));

    sp<IBinder> display(SurfaceComposerClient::getBuiltInDisplay(ISurfaceComposer::eDisplayIdMain));

    SurfaceComposerClient::getDisplayInfo(display, &m_display_info);
    layerStackRect = Rect(0,0,m_display_info.w, m_display_info.h);
    //displayRect = Rect(0,0, m_display_info.h, m_display_info.w);
    displayRect = Rect(0,0, m_display_info.w, m_display_info.h);
    SurfaceComposerClient::setDisplayProjection(display, DISPLAY_ORIENTATION_0, layerStackRect, displayRect);
    SurfaceComposerClient::getDisplayInfo(display, &m_display_info);
    
    MMPDEBUGMSG(1, (TEXT("[CMmpPlayerStagefright::Close] ln=%d "), __LINE__ ));

    m_SurfaceControl.clear();
    m_SurfaceComposerClient.clear();

    MMPDEBUGMSG(1, (TEXT("[CMmpPlayerStagefright::Close] display  w=%d h=%d ori=%d "), 
                 m_display_info.w, 
                 m_display_info.h,
                 m_display_info.orientation
                 ));

    return MMP_SUCCESS;

}

MMP_RESULT CMmpPlayerStagefright::PlayStart() {

    if(m_iMediaPlayer != NULL) {

        if(m_iMediaPlayer->start() == 0) {

            CMmpPlayer::PlayStart();
               //sp<ANativeWindow> mNativeWindow(m_SurfaceControl->getSurface());
                //ANativeWindowBuffer *buf;
                //GraphicBufferMapper &mapper = GraphicBufferMapper::get();
               // Rect bounds(pic_width, pic_height);
                    //android_native_rect_t odd = {0, 0, pic_width, pic_height};

            //    native_window_set_scaling_mode(mNativeWindow.get(), NATIVE_WINDOW_SCALING_MODE_SCALE_TO_WINDOW);
                // native_window_set_buffers_geometry(mNativeWindow.get(),  pic_width, pic_height, PIXEL_FORMAT_RGBA_8888);
                //native_window_set_buffers_transform(mNativeWindow.get(), HAL_TRANSFORM_ROT_90);
        }
                
    }

    

    return MMP_SUCCESS;
}

MMP_RESULT CMmpPlayerStagefright::PlayStop() {

    MMP_RESULT mmpResult = MMP_FAILURE;


    CMmpPlayer::PlayStop();

    if(m_iMediaPlayer != NULL) {

        if(m_iMediaPlayer->stop() == 0) {
            mmpResult = MMP_SUCCESS;
        }
    }

    return mmpResult;
}

MMP_RESULT CMmpPlayerStagefright::PlayPause() {

    if(m_iMediaPlayer != NULL) {

        if(m_iMediaPlayer->pause() == 0) {
            
        }
    }

    return MMP_SUCCESS;
}

MMP_RESULT CMmpPlayerStagefright::PlayResume() {

    if(m_iMediaPlayer != NULL) {

        if(m_iMediaPlayer->start() == 0) {
            
        }
    }

    return MMP_SUCCESS;
}

MMP_RESULT CMmpPlayerStagefright::Seek(MMP_S64 pts) {

    MMP_RESULT mmpResult = MMP_FAILURE;

    if(m_iMediaPlayer != NULL) {

        if(m_iMediaPlayer->seekTo((int)(pts/1000LL)) == 0) {
            mmpResult = MMP_SUCCESS;
            MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpPlayerStagefright::Seek] SUCCESS: m_iMediaPlayer->seekTo")));
        }
        else {
            MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpPlayerStagefright::Seek] FAIL: m_iMediaPlayer->seekTo")));
        }
    }
    else {
        MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpPlayerStagefright::Seek] FAIL: m_iMediaPlayer is NULL")));
    }

    return mmpResult;
}

MMP_S64 CMmpPlayerStagefright::GetDuration() {

    int dur_msec;
    MMP_S64 dur;

    m_iMediaPlayer->getDuration(&dur_msec);

    dur = (MMP_S64)dur_msec*1000;

    return dur;
}
    
MMP_S64 CMmpPlayerStagefright::GetPlayPosition() {

    int play_msec;
    MMP_S64 pos;

    m_iMediaPlayer->getCurrentPosition(&play_msec);

    pos = (MMP_S64)play_msec*1000;

    return pos;

}

MMP_U32 CMmpPlayerStagefright::GetDurationMS() {

    int dur_msec;
    MMP_U32 dur;

    m_iMediaPlayer->getDuration(&dur_msec);

    dur = (MMP_U32)dur_msec;

    return dur;
}
    
MMP_U32 CMmpPlayerStagefright::GetPlayPositionMS() {

    int play_msec;
    MMP_U32 pos;

    m_iMediaPlayer->getCurrentPosition(&play_msec);

    pos = (MMP_U32)play_msec;

    return pos;

}

void CMmpPlayerStagefright::Service() {
    
    MMP_U32 cur_tick, before_tick, start_tick;
    char szmsg[256];
    int dur_msec;
    int play_msec;

    start_tick = CMmpUtil::GetTickCount();
    before_tick = start_tick;

    MMPDEBUGMSG(1, (TEXT("[CMmpPlayerStagefright::Service] ln=%d "), __LINE__ ));
            

    while(m_bServiceRun == MMP_TRUE) {

        cur_tick = CMmpUtil::GetTickCount();

        if( (cur_tick-before_tick) > 1000) {
            
            m_iMediaPlayer->getDuration(&dur_msec);
            m_iMediaPlayer->getCurrentPosition(&play_msec);

            //sprintf(szmsg, "[MmpPlayerStagefrigh] %d. %d/%d \n", 
              //                     (cur_tick-start_tick)/1000,
              //                     play_msec/1000, dur_msec/1000 );
                                   

            //puts(szmsg);

            
            MMPDEBUGMSG(1, (TEXT("[MmpPlayerStagefrigh] %d. %d/%d,  %02d:%02d:%02d /  %02d:%02d:%02d  "), 
                                   (cur_tick-start_tick)/1000,
                                   play_msec/1000, dur_msec/1000, 
                                   CMmpUtil::Time_GetHour(play_msec), CMmpUtil::Time_GetMin(play_msec), CMmpUtil::Time_GetSec(play_msec),
                                   CMmpUtil::Time_GetHour(dur_msec), CMmpUtil::Time_GetMin(dur_msec), CMmpUtil::Time_GetSec(dur_msec)
                                   ));
            

            before_tick = cur_tick;
        }

        CMmpUtil::Sleep(50);
    }
    
}

}

#endif
