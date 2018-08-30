#include "MmpCameraHW_Android.hpp"

namespace android {

////////////////////////////////////////////////
//
#define HEAP_SIZE (640*480*3)
CameraHWHeap::CameraHWHeap() :
m_ID(0x9829)
,m_buffer(new unsigned char[HEAP_SIZE])
,m_offset(0)
{

}

CameraHWHeap::~CameraHWHeap() {
    
    delete [] m_buffer;
}

int CameraHWHeap::getHeapID() const {
    return m_ID;
}
void* CameraHWHeap::getBase() const {
    return (void*)m_buffer;
}

size_t CameraHWHeap::getSize() const {
    return (size_t)HEAP_SIZE;
}

uint32_t CameraHWHeap::getFlags() const {
    return 0;
}

uint32_t CameraHWHeap::getOffset() const {
    return m_offset;
}


////////////////////////////////////////////////
//

CameraHWMemory::CameraHWMemory(int picwidth, int picheight, bool bstoreMetaDataInBuffers) :
m_MemoryHeap(new CameraHWHeap) 
,m_nPicWidth(picwidth)
,m_nPicHeight(picheight)
,m_nPicSize(picwidth*picheight*3/2)
,m_bstoreMetaDataInBuffers(bstoreMetaDataInBuffers)
,m_CamBuffer(NULL)
{
    unsigned char* base;
    unsigned int addr;
    
    if(m_bstoreMetaDataInBuffers == true) {
    
#if 0  //Normal 
        m_CamBuffer = new unsigned char[m_nPicWidth*m_nPicHeight*2];

        addr = (unsigned int)m_CamBuffer;

        base = (unsigned char*)m_MemoryHeap->getBase();
        memcpy(base, &addr, sizeof(unsigned int));
        memcpy(&base[sizeof(unsigned int)], &addr, sizeof(unsigned int));
#else //Samsung yuv420sp buffer stored

        /*
 * meta data contains the following data format.
 * payload depends on the MetadataBufferType
 * --------------------------------------------------------------
 * | MetadataBufferType                         |          payload                           |
 * --------------------------------------------------------------
 *
 * If MetadataBufferType is kMetadataBufferTypeCameraSource, then
 * --------------------------------------------------------------
 * | kMetadataBufferTypeCameraSource  | physical addr. of Y |physical addr. of CbCr |
 * --------------------------------------------------------------
 *
 * If MetadataBufferType is kMetadataBufferTypeGrallocSource, then
 * --------------------------------------------------------------
 * | kMetadataBufferTypeGrallocSource    | buffer_handle_t |
 * --------------------------------------------------------------
 */

//kMetadataBufferTypeCameraSource  = 0,
//kMetadataBufferTypeGrallocSource = 1,

        unsigned int type = 0; //kMetadataBufferTypeCameraSource
        unsigned int yaddr, cbcraddr;
        unsigned int offset = 0;

        m_CamBuffer = new unsigned char[m_nPicWidth*m_nPicHeight*2];

        type = 0;//kMetadataBufferTypeCameraSource
        yaddr = (unsigned int)m_CamBuffer;
        cbcraddr = yaddr + m_nPicWidth*m_nPicHeight;
        offset= 0;

        base = (unsigned char*)m_MemoryHeap->getBase();

        memcpy(&base[offset], &type, sizeof(unsigned int));     offset+=sizeof(unsigned int);
        memcpy(&base[offset], &yaddr, sizeof(unsigned int));    offset+=sizeof(unsigned int);
        memcpy(&base[offset], &cbcraddr, sizeof(unsigned int)); offset+=sizeof(unsigned int);


#endif
    
    }
     
}

CameraHWMemory::~CameraHWMemory() {

    if(m_CamBuffer) {
        delete [] m_CamBuffer;
    }
}

sp<IMemoryHeap> CameraHWMemory::getMemory(ssize_t* offset, size_t* size) const {
      
    size_t sz;
    if(m_bstoreMetaDataInBuffers == true) sz = 64;
    else sz = m_nPicSize;

    if(size) *size = sz;
    if(offset) *offset = 0;
    return m_MemoryHeap;
}

////////////////////////////////////////////////
//

//"video-frame-format=yuv420sp;"\   semi planer Samsung 

#define CAMERA_PARAM \
"video-frame-format=yuv420sp;"\
"video-size-values=320x240,640x480;"\
"preview-frame-rate=24;"\
"preview-frame-rate-values=15,24,30;"\


CMmpCameraHW_Android::CMmpCameraHW_Android() :
m_strCameraParam(CAMERA_PARAM)
,m_bstoreMetaDataInBuffers(false)
,m_bStartRec(false)

,m_nPicWidth(320)
,m_nPicHeight(240)
{

}

CMmpCameraHW_Android::~CMmpCameraHW_Android() {

}

void   CMmpCameraHW_Android::disconnect() {

}

// connect new client with existing camera remote
status_t        CMmpCameraHW_Android::connect(const sp<ICameraClient>& client) {
    return 0;
}

// prevent other processes from using this ICamera interface
status_t        CMmpCameraHW_Android::lock() {
    return 0;
}

// allow other processes to use this ICamera interface
status_t        CMmpCameraHW_Android::unlock() {
    return 0;
}

// pass the buffered IGraphicBufferProducer to the camera service
status_t        CMmpCameraHW_Android::setPreviewTarget(const sp<IGraphicBufferProducer>& bufferProducer) {
    return 0;
}

// set the preview callback flag to affect how the received frames from
// preview are handled. Enabling preview callback flags disables any active
// preview callback surface set by setPreviewCallbackTarget().
void            CMmpCameraHW_Android::setPreviewCallbackFlag(int flag) {

}
// set a buffer interface to use for client-received preview frames instead
// of preview callback buffers. Passing a valid interface here disables any
// active preview callbacks set by setPreviewCallbackFlag(). Passing NULL
// disables the use of the callback target.
status_t        CMmpCameraHW_Android::setPreviewCallbackTarget(const sp<IGraphicBufferProducer>& callbackProducer) {
    return 0;
}

// start preview mode, must call setPreviewTarget first
status_t        CMmpCameraHW_Android::startPreview() {
    return 0;
}

// stop preview mode
void            CMmpCameraHW_Android::stopPreview() {

}

// get preview state
bool            CMmpCameraHW_Android::previewEnabled() {
    return false;
}

// start recording mode
status_t        CMmpCameraHW_Android::startRecording() {
    return 0;
}

// stop recording mode
void            CMmpCameraHW_Android::stopRecording() {

}

// get recording state
bool            CMmpCameraHW_Android::recordingEnabled() {
    return false;
}

// release a recording frame
void            CMmpCameraHW_Android::releaseRecordingFrame(const sp<IMemory>& mem) {

}

// auto focus
status_t        CMmpCameraHW_Android::autoFocus() {
    return 0;
}

// cancel auto focus
status_t        CMmpCameraHW_Android::cancelAutoFocus() {
    return 0;
}

/*
    * take a picture.
    * @param msgType the message type an application selectively turn on/off
    * on a photo-by-photo basis. The supported message types are:
    * CAMERA_MSG_SHUTTER, CAMERA_MSG_RAW_IMAGE, CAMERA_MSG_COMPRESSED_IMAGE,
    * and CAMERA_MSG_POSTVIEW_FRAME. Any other message types will be ignored.
    */
status_t        CMmpCameraHW_Android::takePicture(int msgType) {
    return 0;
}

// set preview/capture parameters - key/value pairs
status_t        CMmpCameraHW_Android::setParameters(const String8& params) {
    return 0;
}

// get preview/capture parameters - key/value pairs
String8         CMmpCameraHW_Android::getParameters() const {
    return String8("");
}

// send command to camera driver
status_t        CMmpCameraHW_Android::sendCommand(int32_t cmd, int32_t arg1, int32_t arg2) {
    return 0;
}

// tell the camera hal to store meta data or real YUV data in video buffers.
status_t        CMmpCameraHW_Android::storeMetaDataInBuffers(bool enabled) {
    return 0;
}



}