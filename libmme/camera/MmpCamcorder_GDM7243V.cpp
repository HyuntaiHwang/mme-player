#include "MmpCamcorder_GDM7243V.hpp"
#include "v4l2_api_def.h"
#include <sys/ioctl.h>
#include "V4L2.h"
#include "gisp-dxo.h"

#define NODE_NAME_PREVIEW "/dev/video3"
#define NODE_NAME_CAPTURE "/dev/video1"
#define NODE_NAME_RECORD  "/dev/video2"
#define NODE_NAME_FACEDETECT "/dev/video4"

enum {
    SENSOR_ID_OVT2715 = 10,
    SENSOR_ID_OVT5670 = 20,		
    SENSOR_ID_VIGEN = 254,
    SENSOR_ID_INVALID = 255,    /* From test, this id is *NOT* valid */
    SENSOR_ID_COUNT = 256,
};

const enum v4l2_buf_type V4L2_BUF_TYPE = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
const enum v4l2_memory V4L2_MEM_TYPE = V4L2_MEMORY_DMABUF;

#define GISP_IN_BUF_TYPE            V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE
#define GISP_OUT_BUF_TYPE           V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE
#define GISP_FIELD_TYPE             V4L2_FIELD_NONE
#define GISP_MEM_TYPE               V4L2_MEMORY_DMABUF

#define GISP_BUF_TYPE(out)          (out ? GISP_OUT_BUF_TYPE : GISP_IN_BUF_TYPE)


static int getSensorId()
{
    return SENSOR_ID_OVT5670;
}




/////////////////////////////////////////////////////////////
//CMmpPlayer Member Functions

CMmpCamcorder_GDM7243V::CMmpCamcorder_GDM7243V(CMmpCamcorder_GDM7243V::CreateConfig *pCreateConfig) : CMmpCamcorder(pCreateConfig) 

,mSensorWidth(2592)
,mSensorHeight(1944)

,mPreviewFd(-1)
,mPreviewWidth(1280)
,mPreviewHeight(720)
,mPreviewFormat(V4L2_PIX_FMT_YVU420)
,mPreviewBufferCount(6)
,mPreviewBuffers(NULL)

    
{

}

CMmpCamcorder_GDM7243V::~CMmpCamcorder_GDM7243V() {

}

MMP_RESULT CMmpCamcorder_GDM7243V::Open()
{
    int i;
    struct GDMBuffer *b;
    struct mmp_buffer_create_config mmpbuf_createconfig;
    class mmp_buffer_videoframe* p_buf_vf;

    mPreviewBuffers = new GDMBuffer[mPreviewBufferCount];
    MMP_ASSERT(mPreviewBuffers);
    memset(mPreviewBuffers, 0, sizeof(struct GDMBuffer) * mPreviewBufferCount);

    for(i = 0; i < mPreviewBufferCount; i++) {

        m_p_buf_vf_arr[i] = mmp_buffer_mgr::get_instance()->alloc_media_videoframe((MMP_MEDIA_ID)this, mPreviewWidth, mPreviewHeight, MMP_FOURCC_IMAGE_YVU420, mmp_buffer::ION, mmp_buffer::FLAG_NONE);
        p_buf_vf = m_p_buf_vf_arr[i];

        b = &mPreviewBuffers[i];

        b->index = i;
        b->planeCount = 1;
        b->plane[0].fd = p_buf_vf->get_buf_shared_fd();
        b->plane[0].offset = 0;
        b->plane[0].length = p_buf_vf->get_buf_size();
        b->plane[0].used = 0;
        b->plane[0].base = (uint8_t*)p_buf_vf->get_buf_vir_addr();
    }

    return MMP_SUCCESS;
}


MMP_RESULT CMmpCamcorder_GDM7243V::Close()
{
    if (mPreviewBuffers){
        delete [] mPreviewBuffers;
		mPreviewBuffers	= NULL;
	}

    return MMP_SUCCESS;
}

MMP_RESULT CMmpCamcorder_GDM7243V::Preview_Start() {

    int ret;
    char sensorName[256];
    struct DXOOutputFormat dxoFmt;

    MMPDEBUGMSG(1, (TEXT("[CMmpCamcorder_GDM7243V::Preview_Start] ++ ")));

    mPreviewFd = ::open(NODE_NAME_PREVIEW, O_RDWR);
    MMP_ASSERT(mPreviewFd > 0);

    ret = v4l2_querycap(mPreviewFd, V4L2_BUF_TYPE);
    //DBG("mPreviewFd=%d, querycap, ret = %d", mPreviewFd, ret);
    MMP_ASSERT(ret == 0);

    ret = v4l2_enum_input(mPreviewFd, getSensorId(), sensorName, sizeof(sensorName));
    MMP_ASSERT(ret == 0);

    ret = v4l2_s_input(mPreviewFd, getSensorId());
    MMP_ASSERT(ret == 0);

    ret = v4l2_enum_fmt(mPreviewFd, V4L2_BUF_TYPE, mPreviewFormat);
    MMP_ASSERT(ret == 0);

    ret = v4l2_s_fmt(mPreviewFd,
		     mPreviewWidth, mPreviewHeight,
		     V4L2_BUF_TYPE, mPreviewFormat, NULL);
    MMP_ASSERT(ret == 0);

    ret = v4l2_reqbufs(mPreviewFd,
		       V4L2_BUF_TYPE, V4L2_MEM_TYPE, mPreviewBufferCount);
    MMP_ASSERT(ret > 0);

    for (int i = 0; i < ret; i++) {
        int iret;

        if (!mPreviewBuffers[i].user)
		{
			//DBG("%d, 0x%X, Preview Buf Not q", i, (unsigned int)mPreviewBuffers[i].user);
            break;
		}

        //GCamera::dumpHandle((buffer_handle_t *)(mPreviewBuffers[i].user));
        iret = v4l2_qbuf(mPreviewFd, mPreviewWidth, mPreviewHeight,
                V4L2_BUF_TYPE, V4L2_MEM_TYPE, &mPreviewBuffers[i], i);
        MMP_ASSERT(iret == 0);
    }


    memset(&dxoFmt, 0, sizeof(dxoFmt));
    dxoFmt.width = mPreviewWidth;
    dxoFmt.height = mPreviewHeight;
    dxoFmt.pixelFormat = mPreviewFormat;
    dxoFmt.crop.left = 0;
    dxoFmt.crop.top = 0;
	dxoFmt.crop.right = mSensorWidth - 1;
    dxoFmt.crop.bottom = mSensorHeight - 1;
    //DBG("dxoFmt = (%dx%d) (%d, %d, %d, %d)",
     //       dxoFmt.width, dxoFmt.height,
       //     dxoFmt.crop.left,
       //     dxoFmt.crop.top,
       //     dxoFmt.crop.right,
       //     dxoFmt.crop.bottom);

    ret = DXOSetOutputFormat(mDXO, DXO_OUTPUT_DISPLAY, &dxoFmt);
    MMP_ASSERT(ret == 0);

    DXOSetOutputEnable(mDXO, 1 << DXO_OUTPUT_DISPLAY, 1 << DXO_OUTPUT_DISPLAY);
    DXOSetOutputEnable(mDXO, 1 << DXO_OUTPUT_CAPTURE, 0);
    DXOSetOutputEnable(mDXO, 1 << DXO_OUTPUT_VIDEO, 0);
    DXOSetOutputEnable(mDXO, 1 << DXO_OUTPUT_FD, 0);

    ret = v4l2_streamon(mPreviewFd, V4L2_BUF_TYPE);


    MMP_ASSERT(ret == NO_ERROR);

    DXORunState(mDXO, DXO_STATE_PREVIEW, 0);

    MMPDEBUGMSG(1, (TEXT("[CMmpCamcorder_GDM7243V::Preview_Start] -- ")));

    return MMP_SUCCESS;
}

