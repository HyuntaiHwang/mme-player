#ifndef MMPCAMCORDER_GDM7243V_HPP__
#define MMPCAMCORDER_GDM7243V_HPP__

#include "MmpCamcorder.hpp"
#include "GDMBuffer.h"
#include "mmp_buffer.hpp"
#include "mmp_buffer_mgr.hpp"
#include "gisp-dxo.h"

class CMmpCamcorder_GDM7243V : public CMmpCamcorder {

friend class CMmpCamcorder;

protected:
    CMmpCamcorder_GDM7243V(CMmpCamcorder::CreateConfig *pCreateConfig);
    virtual ~CMmpCamcorder_GDM7243V();

    virtual MMP_RESULT Open();
    virtual MMP_RESULT Close();

private:
    struct DXO *mDXO;

    int mSensorWidth;
    int mSensorHeight;

    int mPreviewFd;
    int mPreviewWidth;
    int mPreviewHeight;
    unsigned int mPreviewFormat;
    int mPreviewBufferCount;
    struct GDMBuffer *mPreviewBuffers;
    class mmp_buffer_videoframe* m_p_buf_vf_arr[10];
    
public:
    virtual MMP_RESULT Preview_Start();
};


#endif

