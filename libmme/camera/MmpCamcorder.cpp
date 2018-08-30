#include "MmpCamcorder.hpp"
#include "MmpCamcorder_GDM7243V.hpp"


//////////////////////////////////////////////////////////////
// CMmpCamcorder CreateObject/DestroyObject

CMmpCamcorder* CMmpCamcorder::CreateObject(CMmpCamcorder::CreateConfig *pCreateConfig)
{
    CMmpCamcorder* pObj = NULL;
        
    pObj=new CMmpCamcorder_GDM7243V(pCreateConfig);
    if(pObj==NULL)
        return (CMmpCamcorder*)NULL;

    if( pObj->Open()!=MMP_SUCCESS )    
    {
        pObj->Close();
        delete pObj;
        return (CMmpCamcorder*)NULL;
    }

    return pObj;
}

MMP_RESULT CMmpCamcorder::DestroyObject(CMmpCamcorder* pObj)
{
    if(pObj)
    {
        pObj->Close();
        delete pObj;
    }
    return MMP_SUCCESS;
}

/////////////////////////////////////////////////////////////
//CMmpPlayer Member Functions

CMmpCamcorder::CMmpCamcorder(CMmpCamcorder::CreateConfig *pCreateConfig) {

}

CMmpCamcorder::~CMmpCamcorder() {

}

MMP_RESULT CMmpCamcorder::Open()
{
    
    return MMP_SUCCESS;
}


MMP_RESULT CMmpCamcorder::Close()
{
    return MMP_SUCCESS;
}

