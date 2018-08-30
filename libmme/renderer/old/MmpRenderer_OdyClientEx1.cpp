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

#include "MmpRenderer_OdyClientEx1.hpp"

#include "MmpUtil.hpp"
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h> // for POSIX threads
#include <ctype.h>

#include <fcntl.h>
#include <unistd.h>

#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
//#include <sys/socket.h>
#include <sys/un.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <limits.h>

#include "hwcomposer.h"
#include "ion_api.h"
#include "sync.h"

#include "mmp_buffer_mgr.hpp"
#include "MmpImageTool.hpp"

#if (MMP_OS == MMP_OS_WIN32)
struct sockaddr_un {
    int sun_family;
    char sun_path[32];
};
#define bzero(mem, sz) memset(mem, 0x00, sz)
#define AF_UNIX 1

#ifdef open
#undef open
#endif

#define MSG_SEND_BUFFER 0x100

#else


#include <time.h>
#include <inttypes.h>
#include <linux/fb.h>
#include <linux/kd.h>
#include <linux/types.h>
#include <linux/stat.h>
#include <ion_api.h>
#include "gdm_fb.h"
#include <linux/fb.h>

#endif

#define UNIX_SOCKET_PATH       "/tmp/sock_msgio"
#define BUFFER_SIZE            (4096)
#define FD_COUNT               (16)


#if (MMP_OS == MMP_OS_WIN32)
#include "../MmpOpenGL/MmpGL_MovieEx1.hpp"
#include "../MmpComm/colorspace/colorspace.h"
#endif

static int dss_get_response(int sockfd);
static void dss_get_fence_fd(int sockfd, int *release_fd, struct fb_var_screeninfo *vi);
static void dss_overlay_default_config(struct gdm_dss_overlay *req,	struct ody_player *gplayer);
static int dss_overlay_set(int sockfd, struct gdm_dss_overlay *req);
static int dss_overlay_queue(int sockfd, struct gdm_dss_overlay_data *req_data);
static int dss_overlay_unset(int sockfd);

/////////////////////////////////////////////////////////////
//CMmpRenderer_OdyClientEx1 Member Functions


CMmpRenderer_OdyClientEx1::CMmpRenderer_OdyClientEx1(CMmpRendererCreateProp* pRendererProp) :  CMmpRenderer(MMP_MEDIATYPE_VIDEO, pRendererProp)
,m_sock_fd(-1)
,m_buf_idx(0)

#if (MMP_OS==MMP_OS_WIN32)
,m_msg_res(256)
#endif
{
    memset(&m_gplayer, 0x00, sizeof(struct ody_player));
    m_gplayer.release_fd = -1;

}

CMmpRenderer_OdyClientEx1::~CMmpRenderer_OdyClientEx1()
{

}

MMP_RESULT CMmpRenderer_OdyClientEx1::Open()
{
    MMP_RESULT mmpResult = MMP_SUCCESS;
    int iret;
    struct ody_player *gplayer = &m_gplayer;
    struct ody_framebuffer *pfb;
	struct ody_videofile *pvideo;

    mmpResult=CMmpRenderer::Open();

    MMPDEBUGMSG(1, (TEXT("[%s::%s] +++ W:%d H:%d "), MMP_CLASS_NAME, MMP_CLASS_FUNC, m_pRendererProp->m_iPicWidth, m_pRendererProp->m_iPicHeight));

    /* STEP0. Alloc FrameBuffer */
    if(mmpResult == MMP_SUCCESS) {

        pfb = &gplayer->fb_info;
		pvideo = &gplayer->video_info;


        switch(m_pRendererProp->pic_format) {
            case MMP_FOURCC_IMAGE_BGR888:
                pvideo->format = GDMFB_BGR888;
                pvideo->width = m_pRendererProp->m_iPicWidth;
                pvideo->height = m_pRendererProp->m_iPicHeight;
                break;

            case MMP_FOURCC_IMAGE_RGB888:
                pvideo->format = GDMFB_RGB888;
                pvideo->width = m_pRendererProp->m_iPicWidth;
                pvideo->height = m_pRendererProp->m_iPicHeight;
                break;

            case MMP_FOURCC_IMAGE_YUV444Packed:
                pvideo->format = GDMFB_YUV444I;
                pvideo->width = m_pRendererProp->m_iPicWidth;//MMP_BYTE_ALIGN(m_pRendererProp->m_iPicWidth*3,16);
                pvideo->height = m_pRendererProp->m_iPicHeight;//MMP_BYTE_ALIGN(m_pRendererProp->m_iPicHeight,16);
                break;

            case MMP_FOURCC_IMAGE_YUV444P3:
            case MMP_FOURCC_IMAGE_I420:
            default:
                pvideo->format = GDMFB_YUV420P3;
                pvideo->width = MMP_BYTE_ALIGN(m_pRendererProp->m_iPicWidth,16);
                pvideo->height = MMP_BYTE_ALIGN(m_pRendererProp->m_iPicHeight,16);
        }
    }

    /* STEP1. connect to server */
	if(mmpResult == MMP_SUCCESS) {

		struct sockaddr_un server_addr;

#if (MMP_OS == MMP_OS_WIN32)
        m_sock_fd = 0;
#else
		m_sock_fd = socket(AF_UNIX, SOCK_STREAM, 0);
#endif
		if(m_sock_fd >= 0) {

			bzero(&server_addr, sizeof(server_addr));
			server_addr.sun_family = AF_UNIX;
			strcpy(server_addr.sun_path, UNIX_SOCKET_PATH);

#if (MMP_OS == MMP_OS_WIN32)
            iret = 0;
#else
			iret = connect(m_sock_fd,(struct sockaddr *)&server_addr, sizeof(server_addr));
#endif
            if(iret != 0) {
                MMPDEBUGMSG(1, (TEXT("[CMmpRenderer_OdyClientEx1::Open] FAIL: connect ")));
                mmpResult = MMP_FAILURE;
            }
            else {
                dss_get_fence_fd(m_sock_fd, &gplayer->release_fd, &gplayer->vi);
            }
		}
		else {
			mmpResult = MMP_FAILURE;
		}
	}

    // step-02: request overlay to display
    if(mmpResult == MMP_SUCCESS) {

	    dss_overlay_default_config(&m_req, gplayer);
	    dss_overlay_set(m_sock_fd, &m_req);
		dss_get_response(m_sock_fd);
    }

	m_luma_size = m_pRendererProp->m_iPicWidth*m_pRendererProp->m_iPicHeight;
	m_chroma_size = m_luma_size/4;

#if (MMP_OS==MMP_OS_WIN32)
    m_msg_res.open(CMmpRenderer_OdyClientEx1::service_render_stub, this);
#endif

    return mmpResult;
}


MMP_RESULT CMmpRenderer_OdyClientEx1::Close()
{
    MMP_RESULT mmpResult;

#if (MMP_OS==MMP_OS_WIN32)
    m_msg_res.close();
#endif

    mmpResult=CMmpRenderer::Close();

    if(m_sock_fd >= 0) {

        // unset
	    dss_overlay_unset(m_sock_fd);

        if(m_gplayer.release_fd != -1) {
            MMP_DRIVER_CLOSE(m_gplayer.release_fd);
            m_gplayer.release_fd = -1;
        }

        MMP_DRIVER_CLOSE(m_sock_fd);
        m_sock_fd = -1;
    }

    return mmpResult;
}

void CMmpRenderer_OdyClientEx1::SetFirstRenderer() {

    if(m_sock_fd >= 0) {
        CMmpRenderer::SetFirstRenderer();
        dss_overlay_set(m_sock_fd, &m_req);
		dss_get_response(m_sock_fd);
    }
}

MMP_RESULT CMmpRenderer_OdyClientEx1::Render(class mmp_buffer_videoframe* p_buf_videoframe) {

    int iret;
	unsigned int t1, t2;
    MMP_S32 i;

    if( CMmpRenderer::s_pFirstRenderer[m_MediaType] != this ) {

        CMmpRenderer::EncodeAndMux(p_buf_videoframe);

        return MMP_SUCCESS;
    }

	memset(&m_req_data, 0x00, sizeof(struct gdm_dss_overlay_data));
	m_req_data.id = (uint32_t)this;
	m_req_data.num_plane = 3;
    for(i = 0; i < (MMP_S32)m_req_data.num_plane; i++) {
        m_req_data.data[i].memory_id = p_buf_videoframe->get_buf_shared_fd(i);
        m_req_data.data[i].offset = 0;
    }

#if (MMP_OS == MMP_OS_WIN32)
    this->dss_overlay_queue_win32(m_sock_fd, &m_req_data);
#endif

	dss_overlay_queue(m_sock_fd, &m_req_data);
    if(m_gplayer.release_fd == -1) {
        dss_get_fence_fd(m_sock_fd, &m_gplayer.release_fd, NULL);
    }

    if(m_gplayer.release_fd != -1) {
		//printf("wait frame done signal\n");

		t1 = CMmpUtil::GetTickCount();
		iret = sync_wait(m_gplayer.release_fd, 1000);
		t2 = CMmpUtil::GetTickCount();

        MMP_DRIVER_CLOSE(m_gplayer.release_fd);
        //MMPDEBUGMSG(1, (TEXT("[CMmpRenderer_OdyClientEx1::RenderYUV420Planar] sync_wait %d"), t2-t1));

        m_gplayer.release_fd = -1;
		if( (t2-t1) < 100) {
			CMmpUtil::Sleep( 100 - (t2-t1) );
		}
	}
    else {
        CMmpUtil::Sleep(100);
    }

    CMmpRenderer::EncodeAndMux(p_buf_videoframe);

	return MMP_SUCCESS;
}


MMP_RESULT CMmpRenderer_OdyClientEx1::Render(class mmp_buffer_imageframe* p_buf_imageframe) {

    MMP_RESULT mmpResult = MMP_SUCCESS;
    int iret;
	unsigned int t1, t2;
    MMP_S32 i;
    enum MMP_FOURCC fourcc;

    if( CMmpRenderer::s_pFirstRenderer[m_MediaType] != this ) {
        return MMP_SUCCESS;
    }

    memset(&m_req_data, 0x00, sizeof(struct gdm_dss_overlay_data));
    m_req_data.id = (uint32_t)this;

    fourcc = p_buf_imageframe->get_fourcc();
    m_req_data.num_plane = CMmpImageTool::GetPlaneCount(fourcc);
    for(i = 0; i < (MMP_S32)m_req_data.num_plane; i++) {
        m_req_data.data[i].memory_id = p_buf_imageframe->get_buf_shared_fd(i);
        m_req_data.data[i].offset = 0;
    }


#if (MMP_OS == MMP_OS_WIN32)
    this->dss_overlay_queue_win32(m_sock_fd, &m_req_data);
#endif

	dss_overlay_queue(m_sock_fd, &m_req_data);
    if(m_gplayer.release_fd == -1) {
        dss_get_fence_fd(m_sock_fd, &m_gplayer.release_fd, NULL);
    }

    if(m_gplayer.release_fd != -1) {
		//printf("wait frame done signal\n");

		t1 = CMmpUtil::GetTickCount();
		iret = sync_wait(m_gplayer.release_fd, 1000);
		t2 = CMmpUtil::GetTickCount();

        MMP_DRIVER_CLOSE(m_gplayer.release_fd);
        //MMPDEBUGMSG(1, (TEXT("[CMmpRenderer_OdyClientEx1::RenderYUV420Planar] sync_wait %d"), t2-t1));

        m_gplayer.release_fd = -1;
		if( (t2-t1) < 100) {
			CMmpUtil::Sleep( 100 - (t2-t1) );
		}
	}
    else {
        CMmpUtil::Sleep(100);
    }

	return mmpResult;
}

static int dss_get_response(int sockfd)
{
	int ret = 0;
	struct gdm_msghdr *msg = NULL;

	msg = gdm_recvmsg(sockfd);

	if(msg != NULL){
		if(*(int *)msg->buf == -1)
			ret = -1;
		gdm_free_msghdr(msg);
	}

	return ret;
}



static void dss_get_fence_fd(int sockfd, int *release_fd, struct fb_var_screeninfo *vi)
{
	struct gdm_msghdr *msg = NULL;

//	printf("dss_get_fence_fd - start\n");
	msg = gdm_recvmsg(sockfd);

//	printf("received msg: %0x\n", (unsigned int)msg);
	if(msg != NULL){
		if(vi) {
			memcpy(vi, msg->buf, sizeof(struct fb_var_screeninfo));
		}
//		printf("msg->fds[0]: %d\n", msg->fds[0]);
		*release_fd = msg->fds[0];
		gdm_free_msghdr(msg);
	}

}

static void dss_overlay_default_config(struct gdm_dss_overlay *req,	struct ody_player *gplayer)
{

	memset(req, 0x00, sizeof(*req));

	req->alpha = 255;
	req->blend_op = GDM_FB_BLENDING_NONE;
	req->src.width = gplayer->video_info.width;
	req->src.height = gplayer->video_info.height;
	req->src.format = gplayer->video_info.format;
	//req->src.endian = 0;
	//req->src.swap = 0;
	req->pipe_type = GDM_DSS_PIPE_TYPE_VIDEO;

	req->src_rect.x = req->src_rect.y = 0;
	req->src_rect.w = req->src.width;
	req->src_rect.h = req->src.height;

	req->dst_rect.x = req->dst_rect.y = 0;
	req->dst_rect.w = gplayer->vi.xres;
	req->dst_rect.h = gplayer->vi.yres;

	req->transp_mask = 0;
	req->flags = GDM_DSS_FLAG_SCALING;
	req->id = GDMFB_NEW_REQUEST;


}

static int dss_overlay_set(int sockfd, struct gdm_dss_overlay *req)
{
	struct gdm_hwc_msg msg_data;
	struct gdm_msghdr *msg = NULL;
    int iret = 0;

	memset(&msg_data, 0x00, sizeof(struct gdm_hwc_msg));

	msg_data.app_id = APP_ID_MULTI_SAMPLE_PLAYER;
	msg_data.hwc_cmd = GDMFB_OVERLAY_SET;

	msg = gdm_alloc_msghdr(sizeof(struct gdm_hwc_msg), 0);
    if(msg != NULL) {
	    memcpy(msg_data.data, req, sizeof(struct gdm_dss_overlay));
	    memcpy(msg->buf, &msg_data, sizeof(struct gdm_hwc_msg));

	    gdm_sendmsg(sockfd, msg);
    }
    else {
#if (MMP_OS == MMP_OS_WIN32)
        iret = 0;
#else
        iret = -1;
#endif
    }

	return iret;
}


static int dss_overlay_queue(int sockfd, struct gdm_dss_overlay_data *req_data)
{
	struct gdm_hwc_msg msg_data;
	struct gdm_msghdr *msg = NULL;
	int i = 0;
	memset(&msg_data, 0x00, sizeof(struct gdm_hwc_msg));

	msg = gdm_alloc_msghdr(sizeof(struct gdm_hwc_msg), req_data->num_plane);
    if(msg != NULL) {
	    msg_data.app_id = APP_ID_MULTI_SAMPLE_PLAYER;
	    msg_data.hwc_cmd = GDMFB_OVERLAY_PLAY;
	    memcpy(msg_data.data, req_data, sizeof(struct gdm_dss_overlay_data));
	    memcpy(msg->buf, &msg_data, sizeof(struct gdm_hwc_msg));

	    for(i = 0 ; i < (MMP_S32)req_data->num_plane ; i++)
		    msg->fds[i] = req_data->data[i].memory_id;
	    gdm_sendmsg(sockfd, msg);
    }

	return 0;
}

static int dss_overlay_unset(int sockfd)
{
	struct gdm_hwc_msg msg_data;
	struct gdm_msghdr *msg = NULL;

	memset(&msg_data, 0x00, sizeof(struct gdm_hwc_msg));
	msg_data.app_id = APP_ID_MULTI_SAMPLE_PLAYER;
	msg_data.hwc_cmd = GDMFB_OVERLAY_UNSET;

	msg = gdm_alloc_msghdr(sizeof(struct gdm_hwc_msg), 0);
	memcpy(msg->buf, &msg_data, sizeof(struct gdm_hwc_msg));
	gdm_sendmsg(sockfd, msg);

	return 0;

}

#if (MMP_OS == MMP_OS_WIN32)

int CMmpRenderer_OdyClientEx1::dss_overlay_queue_win32(int sockfd, struct gdm_dss_overlay_data *req_data) {

    class mmp_msg_packet* p_packet;

    p_packet = new class mmp_msg_packet(MSG_SEND_BUFFER);

    return 0;

}

void CMmpRenderer_OdyClientEx1::service_render_stub(void* parm) {
    CMmpRenderer_OdyClientEx1* p_obj = (CMmpRenderer_OdyClientEx1*)parm;
    p_obj->service_render();
}

void CMmpRenderer_OdyClientEx1::service_render() {

    class mmp_msg_packet* p_packet;
    MMP_RESULT mmpResult;
    MMP_S32 picWidth, picHeight;
    MMP_U8 *pImageBuffer;
    MMP_S32 shared_fd[MMP_MEDIASAMPLE_PLANE_COUNT];
    MMP_U32 offset[MMP_MEDIASAMPLE_PLANE_COUNT];
    MMP_U32 stride[MMP_MEDIASAMPLE_PLANE_COUNT];
    MMP_U8* Y, *U, *V;
    MMP_S32 y_stride, uv_stride;
    MMP_S32 i;

    class mmp_buffer* p_mmp_buf;
    class mmp_buffer_addr buf_addr[MMP_MEDIASAMPLE_PLANE_COUNT];
    CMmpGL_MovieEx1* pMmpGL;

    if(m_pRendererProp->m_hRenderWnd != NULL) {

        pMmpGL=new CMmpGL_MovieEx1((HWND)m_pRendererProp->m_hRenderWnd,
                                    ::GetDC((HWND)m_pRendererProp->m_hRenderWnd),
                                    m_pRendererProp->m_iPicWidth, m_pRendererProp->m_iPicHeight);
        if(pMmpGL==NULL)
        {
            MMPDEBUGMSG(MMPZONE_ERROR, (TEXT("[CWndOpenGL::OnCreate] FAIL: CMmpGL::CreateObject \n\r")));
            mmpResult = MMP_FAILURE;
        }

        if(mmpResult == MMP_SUCCESS) {

            if(pMmpGL->Open()!=MMP_SUCCESS)
            {
                pMmpGL->Close();
                delete pMmpGL;
                pMmpGL=NULL;
                mmpResult = MMP_FAILURE;
            }
            else {
                RECT rect;
                ::GetClientRect((HWND)m_pRendererProp->m_hRenderWnd, &rect);
                pMmpGL->Resize( (rect.right-rect.left), (rect.bottom-rect.top));
            }
        }

    }


    while(m_msg_res.is_run()) {

        mmpResult = m_msg_res.readmsg_from_queue(&p_packet);
        if(mmpResult == MMP_SUCCESS) {

            switch(p_packet->m_msg) {

                case MSG_SEND_BUFFER:

                    picWidth = pMmpGL->GetPicWidth();
                    picHeight = pMmpGL->GetPicHeight();
                    pImageBuffer= pMmpGL->GetImageBuffer();
                    //lumaSize=picWidth*picHeight;

                    shared_fd[MMP_MEDIASAMPLE_BUF_Y] = p_packet->m_int_parm[0];
                    shared_fd[MMP_MEDIASAMPLE_BUF_U] = p_packet->m_int_parm[1];
                    shared_fd[MMP_MEDIASAMPLE_BUF_V] = p_packet->m_int_parm[2];

                    offset[MMP_MEDIASAMPLE_BUF_Y] = p_packet->m_int_parm[3];
                    offset[MMP_MEDIASAMPLE_BUF_U] = p_packet->m_int_parm[4];
                    offset[MMP_MEDIASAMPLE_BUF_V] = p_packet->m_int_parm[5];

                    stride[MMP_MEDIASAMPLE_BUF_Y] = p_packet->m_int_parm[6];
                    stride[MMP_MEDIASAMPLE_BUF_U] = p_packet->m_int_parm[7];
                    stride[MMP_MEDIASAMPLE_BUF_V] = p_packet->m_int_parm[8];

                    for(i = 0; i < MMP_MEDIASAMPLE_PLANE_COUNT; i++) {
                        p_mmp_buf = mmp_buffer_mgr::get_instance()->get_buffer(shared_fd[i]);
                        if(p_mmp_buf != NULL) {
                            buf_addr[i] = p_mmp_buf->get_buf_addr();
                        }
                    }

                    Y=(MMP_U8*)(buf_addr[MMP_MEDIASAMPLE_BUF_Y].m_vir_addr + offset[MMP_MEDIASAMPLE_BUF_Y]);
                    U=(MMP_U8*)(buf_addr[MMP_MEDIASAMPLE_BUF_U].m_vir_addr + offset[MMP_MEDIASAMPLE_BUF_U]);
                    V=(MMP_U8*)(buf_addr[MMP_MEDIASAMPLE_BUF_V].m_vir_addr + offset[MMP_MEDIASAMPLE_BUF_V]);
                    y_stride = stride[MMP_MEDIASAMPLE_BUF_Y];
                    uv_stride = stride[MMP_MEDIASAMPLE_BUF_U];

                    (*yv12_to_bgr)( pImageBuffer, //uint8_t * x_ptr,
				                    picWidth*3, //int x_stride,
					                Y, //uint8_t * y_src,
					                V, //uint8_t * v_src,
					                U, //uint8_t * u_src,
					                y_stride,//int y_stride,
					                uv_stride, //int uv_stride,
					                picWidth, //int width,
					                picHeight, //int height,
					                1 //int vflip
                                    );



                   pMmpGL->Draw();

                   delete p_packet;
                   break;
            }

        }

    }

    if(pMmpGL)
    {
        pMmpGL->Close();
        delete pMmpGL;
        pMmpGL=NULL;
    }

}

#endif
