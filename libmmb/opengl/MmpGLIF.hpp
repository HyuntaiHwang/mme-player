
#ifndef _HEADER_MmpGLIF_HPP
#define _HEADER_MmpGLIF_HPP

#include "MmpDefine.h"

class CMmpGLIF
{
public:
    static CMmpGLIF* CreateObject(HWND hWnd, HDC hDC);
    static MMP_RESULT DestroyObject(CMmpGLIF*);

    virtual MMP_RESULT Open()=0;
	virtual MMP_RESULT Close()=0;

public:
    virtual ~CMmpGLIF() {}

    virtual void Draw()=0;

    virtual COLORREF GetBackColor()=0;
    virtual void SetBackColor(COLORREF col)=0;
    virtual void SetBackColor(BYTE r, BYTE g, BYTE b)=0;

    virtual void Resize(int cx, int cy)=0;

    virtual void Key_Proc(UINT nChar, UINT nRepCnt, UINT nFlags)=0;
};

#endif

