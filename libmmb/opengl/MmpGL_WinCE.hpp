
#ifndef _HEADER_MmpGL_WINCE_HPP
#define _HEADER_MmpGL_WINCE_HPP

#include "MmpDefine.h"
#include "MmpGL.hpp"

#if (MMP_OS==MMP_OS_WINCE60)

#include "OpenGL_ES/Include/EGL/egl.h"
#include "OpenGL_ES/Include/GLES/gl.h"

class CMmpGL_WinCE : public CMmpGL
{
friend class CMmpGLIF;

private:
    EGLDisplay  m_eglDisplay;
    EGLContext  m_eglContext;
    EGLConfig   m_eglConfig;
    EGLConfig  *m_eglConfigArr;
    EGLSurface  m_eglSurface;

private:
	CMmpGL_WinCE(HWND hWnd, HDC hDC);
	~CMmpGL_WinCE(void);

	virtual MMP_RESULT Open();
	virtual MMP_RESULT Close();

    virtual void mglMakeCurrent(BOOL bFlag);
    virtual void mglSwapBuffers();// { eglSwapBuffers(m_eglDisplay, m_eglSurface); }

    virtual void Resize(int cx, int cy);
};

#endif //#if (MMP_OS==MMP_OS_WINCE60)
#endif