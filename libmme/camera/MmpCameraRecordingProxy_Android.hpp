#ifndef MMPCAMERARECORDINGPROXY_ANDROID_HPP__
#define MMPCAMERARECORDINGPROXY_ANDROID_HPP__

#include <binder/IPCThreadState.h>
#include <binder/ProcessState.h>
#include <binder/IServiceManager.h>
#include <utils/Log.h>
#include "TemplateList.hpp"
#include <camera/ICameraRecordingProxy.h>
#include "MmpCameraHW_Android.hpp"

namespace android {

class MmpCameraRecordingProxy_Android : public ICameraRecordingProxy {

private:
    sp<CMmpCameraHW_Android> m_pCameraHW;

public:
    MmpCameraRecordingProxy_Android(sp<CMmpCameraHW_Android> &pCameraHW);
    virtual ~MmpCameraRecordingProxy_Android();

    virtual status_t        startRecording(const sp<ICameraRecordingProxyListener>& listener);
    virtual void            stopRecording();
    virtual void            releaseRecordingFrame(const sp<IMemory>& mem);

    virtual IBinder*            onAsBinder() {return NULL;}
};

}

#endif
