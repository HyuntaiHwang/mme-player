#include "MmpCameraRecordingProxy_Android.hpp"

namespace android {


MmpCameraRecordingProxy_Android::MmpCameraRecordingProxy_Android(sp<CMmpCameraHW_Android> &pCameraHW) :
m_pCameraHW(pCameraHW)
{

}

MmpCameraRecordingProxy_Android::~MmpCameraRecordingProxy_Android() {

}

status_t MmpCameraRecordingProxy_Android::startRecording(const sp<ICameraRecordingProxyListener>& listener) {

    return m_pCameraHW->startRecording();
}

void MmpCameraRecordingProxy_Android::stopRecording() {
    
    m_pCameraHW->stopRecording();
}

void MmpCameraRecordingProxy_Android::releaseRecordingFrame(const sp<IMemory>& mem) {

    m_pCameraHW->releaseRecordingFrame(mem);
}


}