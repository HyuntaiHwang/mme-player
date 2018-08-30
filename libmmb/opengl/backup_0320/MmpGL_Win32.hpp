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


#ifndef _HEADER_MmpGL_WIN32_HPP
#define _HEADER_MmpGL_WIN32_HPP

#include "../MmpGlobal/MmpDefine.h"
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