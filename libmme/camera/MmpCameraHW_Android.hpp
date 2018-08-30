#ifndef MMPCAMERAHW_ANDROID_HPP__
#define MMPCAMERAHW_ANDROID_HPP__

#include <binder/IPCThreadState.h>
#include <binder/ProcessState.h>
#include <binder/IServiceManager.h>
#include <utils/Log.h>
#include "TemplateList.hpp"
#include <camera/ICamera.h>
#include <binder/IMemory.h>

namespace android {

class CameraHWMemory;

class CameraHWHeap : public IMemoryHeap {

friend class CameraHWMemory;

private:
    int m_ID;
    unsigned char* m_buffer;
    unsigned int m_offset;
public:
    CameraHWHeap();
    virtual ~CameraHWHeap();

    virtual int         getHeapID() const;
    virtual void*       getBase() const;
    virtual size_t      getSize() const;
    virtual uint32_t    getFlags() const;
    virtual uint32_t    getOffset() const;

    virtual IBinder*            onAsBinder() {return NULL;}
};

class CameraHWMemory : public IMemory {

private:
    sp<IMemoryHeap> m_MemoryHeap;
    int m_nPicWidth;
    int m_nPicHeight;
    int m_nPicSize;
    bool m_bstoreMetaDataInBuffers;
    
    unsigned char *m_CamBuffer;

protected:
    virtual sp<IMemoryHeap> getMemory(ssize_t* offset=0, size_t* size=0) const;

public:


    CameraHWMemory(int picwidth, int picheight, bool bstoreMetaDataInBuffers);
    ~CameraHWMemory();

    virtual IBinder*            onAsBinder() {return NULL;}



};

class CMmpCameraHW_Android : public ICamera {

private:
    sp<ICameraClient> m_camera_client;
    String8 m_strCameraParam;
    bool m_bstoreMetaDataInBuffers;

    //HANDLE m_Thread;
    bool m_bStartRec;
    bool m_bThreadRun;

    int m_nPicWidth;
    int m_nPicHeight;
    
    static void ServiceStub(void* param);
    void Service();

public:
    CMmpCameraHW_Android();
    virtual ~CMmpCameraHW_Android();

    virtual void            disconnect();

    // connect new client with existing camera remote
    virtual status_t        connect(const sp<ICameraClient>& client);

    // prevent other processes from using this ICamera interface
    virtual status_t        lock();

    // allow other processes to use this ICamera interface
    virtual status_t        unlock();

    // pass the buffered IGraphicBufferProducer to the camera service
    virtual status_t        setPreviewTarget(const sp<IGraphicBufferProducer>& bufferProducer);

    // set the preview callback flag to affect how the received frames from
    // preview are handled. Enabling preview callback flags disables any active
    // preview callback surface set by setPreviewCallbackTarget().
    virtual void            setPreviewCallbackFlag(int flag);
    // set a buffer interface to use for client-received preview frames instead
    // of preview callback buffers. Passing a valid interface here disables any
    // active preview callbacks set by setPreviewCallbackFlag(). Passing NULL
    // disables the use of the callback target.
    virtual status_t        setPreviewCallbackTarget(const sp<IGraphicBufferProducer>& callbackProducer);

    // start preview mode, must call setPreviewTarget first
    virtual status_t        startPreview();

    // stop preview mode
    virtual void            stopPreview();

    // get preview state
    virtual bool            previewEnabled();

    // start recording mode
    virtual status_t        startRecording();

    // stop recording mode
    virtual void            stopRecording();

    // get recording state
    virtual bool            recordingEnabled();

    // release a recording frame
    virtual void            releaseRecordingFrame(const sp<IMemory>& mem);

    // auto focus
    virtual status_t        autoFocus();

    // cancel auto focus
    virtual status_t        cancelAutoFocus();

    /*
     * take a picture.
     * @param msgType the message type an application selectively turn on/off
     * on a photo-by-photo basis. The supported message types are:
     * CAMERA_MSG_SHUTTER, CAMERA_MSG_RAW_IMAGE, CAMERA_MSG_COMPRESSED_IMAGE,
     * and CAMERA_MSG_POSTVIEW_FRAME. Any other message types will be ignored.
     */
    virtual status_t        takePicture(int msgType);

    // set preview/capture parameters - key/value pairs
    virtual status_t        setParameters(const String8& params);

    // get preview/capture parameters - key/value pairs
    virtual String8         getParameters() const;

    // send command to camera driver
    virtual status_t        sendCommand(int32_t cmd, int32_t arg1, int32_t arg2);

    // tell the camera hal to store meta data or real YUV data in video buffers.
    virtual status_t        storeMetaDataInBuffers(bool enabled);


    //virtual IBinder*            onAsBinder() {return NULL;}
};

}

#endif
