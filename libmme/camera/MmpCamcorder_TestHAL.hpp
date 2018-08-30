#ifndef MMPCAMCORDER_TEST_HAL_HPP__
#define MMPCAMCORDER_TEST_HAL_HPP__

#include "MmpDefine.h"
#include "MmpCameraHW_Android.hpp"
#include "MmpCameraRecordingProxy_Android.hpp"

class CMmpCamcorder_TestHAL {

private:


public:
    CMmpCamcorder_TestHAL();
    virtual ~CMmpCamcorder_TestHAL();

    MMP_RESULT Open();
};


#endif

