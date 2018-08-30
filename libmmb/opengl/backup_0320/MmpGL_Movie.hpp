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


#ifndef _HEADER_MmpGL_MOVIE_HPP
#define _HEADER_MmpGL_MOVIE_HPP

#include "../MmpGlobal/MmpDefine.h"
#include "MmpGL.hpp"
#include "Matrix.h"

#define GRID_VERTEX_COUNT 1000

class CMmpGL_Movie : public CMmpGL
{
friend class CMmpGLIF;

private:
    CMatrix m_mtxGrid;
    GLfloat* m_vertexGrid;
    GLubyte* m_colorGrid;
    GLushort* m_idxGrid;

public:
	CMmpGL_Movie(HWND hWnd, HDC hDC);
	virtual ~CMmpGL_Movie(void);

	virtual MMP_RESULT Open();
	virtual MMP_RESULT Close();

    virtual void Draw_Objects();
    
public:
    virtual void Draw();
    virtual void Key_Proc(UINT nChar, UINT nRepCnt, UINT nFlags);
};

#endif