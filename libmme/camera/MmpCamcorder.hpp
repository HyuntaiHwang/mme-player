#ifndef MMPCAMCORDER_HPP__
#define MMPCAMCORDER_HPP__

#include "MmpDefine.h"

class CMmpCamcorder {

public:
    struct CreateConfig {
        MMP_S32 dummy;
    };

public:
    static CMmpCamcorder* CreateObject(CMmpCamcorder::CreateConfig *pCreateConfig);
    static MMP_RESULT DestroyObject(CMmpCamcorder* pObj);

protected:
    CMmpCamcorder(CMmpCamcorder::CreateConfig *pCreateConfig);
    virtual ~CMmpCamcorder();

    virtual MMP_RESULT Open();
    virtual MMP_RESULT Close();

public:
    virtual MMP_RESULT Preview_Start() = 0;
};


#endif

