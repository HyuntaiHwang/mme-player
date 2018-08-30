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

#include "MmpRenderer_AndroidSurfaceEx2.hpp"
#include "MmpUtil.hpp"
//#include "colorspace/colorspace.h"

/////////////////////////////////////////////////////////////
//CMmpRenderer_AndroidSurfaceEx2 Member Functions

#define RENDER_FOURCC_IN MMP_FOURCC_IMAGE_YVU420
//#define RENDER_FOURCC_IN MMP_FOURCC_IMAGE_ABGR8888

CMmpRenderer_AndroidSurfaceEx2::CMmpRenderer_AndroidSurfaceEx2(struct CMmpRendererVideo::create_config* p_create_config) :  
CMmpRendererVideo(p_create_config, RENDER_FOURCC_IN)

{
    

}

CMmpRenderer_AndroidSurfaceEx2::~CMmpRenderer_AndroidSurfaceEx2()
{

}

MMP_RESULT CMmpRenderer_AndroidSurfaceEx2::Open()
{
    MMP_RESULT mmpResult = MMP_SUCCESS;

    mmpResult=CMmpRendererVideo::Open();
    

    sp<IBinder> display(SurfaceComposerClient::getBuiltInDisplay(ISurfaceComposer::eDisplayIdMain));
    
    //DisplayID display_id  =0;
    SurfaceComposerClient::getDisplayInfo(display, &m_display_info);

    MMPDEBUGMSG(MMPZONE_INFO, (TEXT("[CMmpRenderer_AndroidSurfaceEx2::Open] DisplayInfo Resol(%d, %d) DPI(%3.1f %3.1f) density=%3.1f orientation=%d "), 
            m_display_info.w, m_display_info.h,
            m_display_info.xdpi, m_display_info.ydpi,
            m_display_info.density,
            m_display_info.orientation
            ));

    /* Create Composer Client */
    if(mmpResult == MMP_SUCCESS) {
    
        m_ComposerClient = new SurfaceComposerClient;
        if(m_ComposerClient->initCheck() != NO_ERROR) {
            MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpRenderer_AndroidSurfaceEx2::Open] FAIL: mComposerClient = new SurfaceComposerClient")));
            mmpResult = MMP_SUCCESS;
        }
    }

    /* Create Surface */
    if(mmpResult == MMP_SUCCESS) {
        //m_SurfaceControl = m_ComposerClient->createSurface(String8("CMmpRenderer_AndroidSurfaceEx2"), m_display_info.w, m_display_info.h, PIXEL_FORMAT_RGBA_8888, 0);
        m_SurfaceControl = m_ComposerClient->createSurface(String8("CMmpRenderer_AndroidSurfaceEx2"), m_display_info.w, m_display_info.h, PIXEL_FORMAT_BGRA_8888, 0);
        if(m_SurfaceControl == NULL) {
            mmpResult = MMP_FAILURE;
            MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpRenderer_AndroidSurfaceEx2::Open] FAIL: m_ComposerClient->createSurface")));
        }
        else { 
            if(m_SurfaceControl->isValid() != 1) {
                mmpResult = MMP_FAILURE;
                MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpRenderer_AndroidSurfaceEx2::Open] ERROR : m_SurfaceControl->isValid() ")));
            }
        }
    }

    /* Set Surface Prop */
    if(mmpResult == MMP_SUCCESS) {
        SurfaceComposerClient::openGlobalTransaction();
        m_SurfaceControl->setLayer(0x7fffffff);
        //m_SurfaceControl->setLayer(0xffffffff);
        m_SurfaceControl->show();
        SurfaceComposerClient::closeGlobalTransaction();

        m_Surface = m_SurfaceControl->getSurface();
        if(m_Surface == NULL) {
            mmpResult = MMP_FAILURE;
            MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CMmpRenderer_AndroidSurfaceEx2::Open] FAIL: m_SurfaceControl->getSurface ")));
        }
        else {
            sp<ANativeWindow> wnd(m_Surface);
            m_NativeWindow = wnd;//new ANativeWindow(m_Surface);
    
        }
    }

    
    return mmpResult;
}


MMP_RESULT CMmpRenderer_AndroidSurfaceEx2::Close()
{
    MMP_RESULT mmpResult;

    if(m_ComposerClient != NULL) {
        m_ComposerClient->dispose();
    }

    mmpResult=CMmpRendererVideo::Close();
    

    return mmpResult;
}

MMP_RESULT CMmpRenderer_AndroidSurfaceEx2::vf_config_internal(MMP_S32 bufcnt, MMP_S32 pic_width, MMP_S32 pic_height, struct mmp_rect display_crop_rect) {
    
    MMP_S32 i;
    class mmp_buffer_videoframe* p_buf_vf;
    enum MMP_FOURCC fourcc;
    int pixel_format;
    int  err;
    unsigned int usage=0;
    MMP_S32 ion_fd[3] = {-1, -1 -1};
    MMP_S32 ion_mem_offset[3] = {0, 0, 0};
    
    m_buf_idx = 0;
    m_buf_count = bufcnt;

    fourcc = this->get_fourcc_in();

    //native_window_api_connect(m_NativeWindow.get(),  NATIVE_WINDOW_API_CPU);
    
    //native_window_set_active_rect(m_NativeWindow.get(), &a_rect);
    native_window_set_buffer_count(m_NativeWindow.get(), m_buf_count);
    native_window_set_scaling_mode(m_NativeWindow.get(), NATIVE_WINDOW_SCALING_MODE_SCALE_TO_WINDOW);
    //native_window_set_scaling_mode(m_NativeWindow.get(), NATIVE_WINDOW_SCALING_MODE_SCALE_CROP);
    switch(fourcc) {
        case MMP_FOURCC_IMAGE_ARGB8888:  pixel_format = HAL_PIXEL_FORMAT_RGBA_8888; break;
        case MMP_FOURCC_IMAGE_ABGR8888:  pixel_format = HAL_PIXEL_FORMAT_BGRA_8888; break;
        case MMP_FOURCC_IMAGE_YVU420: 
        case MMP_FOURCC_IMAGE_YUV420: 
               //pixel_format = 0x13;  //#define OMX_COLOR_FormatYUV420Planar 0x13
               pixel_format = HAL_PIXEL_FORMAT_YV12; 
               break;
        default:
            pixel_format = fourcc;
        
    }
    native_window_set_buffers_format(m_NativeWindow.get(),  pixel_format /*HAL_PIXEL_FORMAT_YV12*/ );
    //native_window_set_buffers_dimensions(m_NativeWindow.get(), pic_width, pic_height ); /* note : Don't set 'buf_width/buf_height',  dimenstion means 'pic_width/pic_height'  */
    //native_window_set_buffers_dimensions(m_NativeWindow.get(), buf_width, buf_height ); /* note : Don't set 'buf_width/buf_height',  dimenstion means 'pic_width/pic_height'  */

    native_window_set_buffers_geometry(m_NativeWindow.get(), pic_width, pic_height, pixel_format);
    native_window_set_usage(m_NativeWindow.get(), usage | GRALLOC_USAGE_HW_TEXTURE | GRALLOC_USAGE_EXTERNAL_DISP);
    
#if 0 // don't use ROTATE in case of 1280x720 LCD 
    native_window_set_buffers_transform(m_NativeWindow.get(), HAL_TRANSFORM_ROT_90);
#endif

    android_native_rect_t a_rect;

    a_rect.left = display_crop_rect.left;
    a_rect.top = display_crop_rect.top;
    a_rect.right = display_crop_rect.right;
    a_rect.bottom = display_crop_rect.bottom;

    native_window_set_crop(m_NativeWindow.get(), &a_rect);

    for(i = 0; i < bufcnt; i++) {

            ANativeWindowBuffer *buf = NULL;

            err = native_window_dequeue_buffer_and_wait(m_NativeWindow.get(), &buf);
            if( (buf != NULL) && (err==0) )  {
                
               m_buf_native[i] = buf;

                MMPDEBUGMSG(1, (TEXT("[CMmpRenderer_AndroidSurfaceEx2::vf_config_internal] buffer(%d/%d) pic(w:%d h:%d) d_rect(%d %d %d %d)  buf(w:%d h:%d stride:%d) ver:%d numfd:%d numInts:%d data:0x%08x fd:%d int(%d %d %d,,,) "), 
                        i, m_buf_count, 
                        pic_width, pic_height, 
                        display_crop_rect.left, display_crop_rect.top, display_crop_rect.right, display_crop_rect.bottom,
                        buf->width, buf->height, buf->stride,
                        buf->handle->version, buf->handle->numFds, buf->handle->numInts, buf->handle->data,
                        buf->handle->data[0], //fd
                        buf->handle->data[1], buf->handle->data[2], buf->handle->data[3]
                        ));

                ion_fd[0] = buf->handle->data[0];
                p_buf_vf = mmp_buffer_mgr::get_instance()->attach_media_videoframe((MMP_MEDIA_ID)this, ion_fd, ion_mem_offset, mmp_buffer::ION_HEAP_CARVEOUT,
                                                                                   pic_width, pic_height, fourcc);
                if(p_buf_vf != NULL) {
                   this->vf_add(p_buf_vf);
                }
            }
    }
        
    return MMP_SUCCESS;
}

#if 0

        typedef struct native_handle
        {
            int version;        /* sizeof(native_handle_t) */
            int numFds;         /* number of file-descriptors at &data[0] */
            int numInts;        /* number of ints at &data[numFds] */
            int *data;        /* numFds + numInts ints */
        } native_handle_t;
        typedef const native_handle_t* buffer_handle_t;

            typedef struct ANativeWindowBuffer
            {
            #ifdef __cplusplus
                ANativeWindowBuffer() {
                    common.magic = ANDROID_NATIVE_BUFFER_MAGIC;
                    common.version = sizeof(ANativeWindowBuffer);
                    memset(common.reserved, 0, sizeof(common.reserved));
                }

                // Implement the methods that sp<ANativeWindowBuffer> expects so that it
                // can be used to automatically refcount ANativeWindowBuffer's.
                void incStrong(const void* id) const {
                    common.incRef(const_cast<android_native_base_t*>(&common));
                }
                void decStrong(const void* id) const {
                    common.decRef(const_cast<android_native_base_t*>(&common));
                }
            #endif

                struct android_native_base_t common;

                int width;
                int height;
                int stride;
                int format;
                int usage;

                void* reserved[2];

                buffer_handle_t handle;

                void* reserved_proc[8];
            } ANativeWindowBuffer_t;
#endif

#define RENDER_DEBUG 0
MMP_RESULT CMmpRenderer_AndroidSurfaceEx2::Render(class mmp_buffer_videoframe* p_buf_vf, MMP_MEDIA_ID decoder_id) {
    
    MMP_S32 i;
    int fd1, fd2, fd3;

    GraphicBufferMapper &mapper = GraphicBufferMapper::get();
    void* dst[3] = { NULL, NULL, NULL };
    int err;

    ANativeWindowBuffer *buf;

    MMPDEBUGMSG(RENDER_DEBUG, (TEXT("[AKDEBUG %s] ln=%d  m_buf_count=%d "), __func__, __LINE__, m_buf_count ));

    for(i = 0; i < m_buf_count; i++) {
        
        buf = m_buf_native[i];
        fd1 = buf->handle->data[0];
        fd2 = p_buf_vf->get_buf_shared_fd();
        MMPDEBUGMSG(RENDER_DEBUG, (TEXT("[AKDEBUG %s] ln=%d "), __func__, __LINE__ ));
        if(fd1 == fd2) {

            //p_buf_vf->set_busy(MMP_TRUE);

            MMPDEBUGMSG(RENDER_DEBUG, (TEXT("[AKDEBUG %s] ln=%d "), __func__, __LINE__ ));

            /*
            android_native_rect_t a_rect;

    a_rect.left = 0;
    a_rect.top = 0;
    a_rect.right = 720/2;
    a_rect.bottom = 404;

    native_window_set_crop(m_NativeWindow.get(), &a_rect);
    //native_window_set_post_transform_crop(m_NativeWindow.get(), &a_rect);
    */
            err = m_NativeWindow->queueBuffer(m_NativeWindow.get(), buf,  -1);
            if(err==0) {
                p_buf_vf->set_own((MMP_MEDIA_ID)this);
            }

            //CMmpUtil::Sleep(30);
            MMPDEBUGMSG(RENDER_DEBUG, (TEXT("[AKDEBUG %s] ln=%d "), __func__, __LINE__ ));
            buf = NULL;
            int err = native_window_dequeue_buffer_and_wait(m_NativeWindow.get(), &buf);
        
            if( (err==0) && (buf != NULL) ) {

                MMPDEBUGMSG(RENDER_DEBUG, (TEXT("[AKDEBUG %s] ln=%d "), __func__, __LINE__ ));
                fd3 = buf->handle->data[0];
                this->vf_set_own(fd3, decoder_id);
            }

            break;
        }
    }

    
    return MMP_SUCCESS;
}

