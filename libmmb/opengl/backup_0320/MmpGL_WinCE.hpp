/*
 *******************************************************************************
 *  (C) Copyright MtekVision Ltd. 2007 All right reserved.
 *    Confidential Information
 *  
 *  All parts of the MtekVision Program Source are protected by copyright law 
 *  and all rights are reserved. 
 *  This documentation may not, in whole or in part, be copied, photocopied, 
 *  reproduced, translated, or reduced to any electronic medium or machine 
 *  readable form without prior consent, in writing, from MtekVision. 
 *******************************************************************************
 *  Project     : OpenOS1
 *  Description :
 *      
 *******************************************************************************
 *  Version control information
 *  $Id$
 *
 *******************************************************************************
 *  Modification History
 *  $Log$
 *
 *******************************************************************************
 */


#ifndef _HEADER_MmpGL_WINCE_HPP
#define _HEADER_MmpGL_WINCE_HPP

#include "../MmpGlobal/MmpDefine.h"
#include "MmpGL.hpp"

#if (MMP_OS==MMP_OS_WINCE60)

#include "../MmpGlobal/OpenGL_ES/Include/EGL/egl.h"
#include "../MmpGlobal/OpenGL_ES/Include/GLES/gl.h"

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