
#ifndef _HEADER_MmpGL_MOVIEEX1_HPP
#define _HEADER_MmpGL_MOVIEEX1_HPP

#include "MmpDefine.h"
#include "MmpGL.hpp"
#include "Matrix.h"


class CMmpGL_MovieEx1 : public CMmpGL
{
friend class CMmpGLIF;

private:

    int m_iViewWidth;
    int m_iViewHeight;

    int m_iPicWidth;
    int m_iPicHeight;
    unsigned char* m_pImageBuffer;


public:
	CMmpGL_MovieEx1(HWND hWnd, HDC hDC, int picWidth, int picHeight);
	virtual ~CMmpGL_MovieEx1(void);

	virtual MMP_RESULT Open();
	virtual MMP_RESULT Close();

    virtual void Draw_Objects();
    
public:
    virtual void Draw();
    virtual void Key_Proc(UINT nChar, UINT nRepCnt, UINT nFlags);

    unsigned char* GetImageBuffer() { return m_pImageBuffer; }
    int GetPicWidth() { return m_iPicWidth; }
    int GetPicHeight() { return m_iPicHeight; }
};

#endif