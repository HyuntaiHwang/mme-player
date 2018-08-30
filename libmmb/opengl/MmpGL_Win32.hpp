
#ifndef _HEADER_MmpGL_WIN32_HPP
#define _HEADER_MmpGL_WIN32_HPP

#include "MmpDefine.h"
#include "MmpGL.hpp"

#if (MMP_OS==MMP_OS_WIN32)

#include<gl/gl.h>

class CMmpGL_Win32 : public CMmpGL
{
friend class CMmpGLIF;

private:
    HGLRC m_hRC;

private:
	CMmpGL_Win32(HWND hWnd, HDC hDC);
	~CMmpGL_Win32(void);

	virtual MMP_RESULT Open();
	virtual MMP_RESULT Close();

    virtual void mglMakeCurrent(BOOL bFlag);
    virtual void mglSwapBuffers(); //{ SwapBuffers(m_hDC); }
    
};

#endif //#if (MMP_OS==MMP_OS_WIN32)
#endif