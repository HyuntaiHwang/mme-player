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


#ifndef _HEADER_MmpGLIF_HPP
#define _HEADER_MmpGLIF_HPP

#include "../MmpGlobal/MmpDefine.h"

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

