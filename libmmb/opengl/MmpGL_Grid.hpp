
#ifndef _HEADER_MmpGL_GRID_HPP
#define _HEADER_MmpGL_GRID_HPP

#include "MmpDefine.h"
#include "MmpGL.hpp"
#include "Matrix.h"


class CMmpGL_Grid : public CMmpGL
{
friend class CMmpGLIF;

private:
    CMatrix m_mtxGrid;
    GLfloat* m_vertexGrid;
    GLubyte* m_colorGrid;
    GLushort* m_idxGrid;

private:
	CMmpGL_Grid(HWND hWnd, HDC hDC);
	virtual ~CMmpGL_Grid(void);

	virtual MMP_RESULT Open();
	virtual MMP_RESULT Close();

    virtual void Draw_Objects();
    void Draw_Grid();
};

#endif