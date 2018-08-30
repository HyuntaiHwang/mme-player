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

#include "MmpGL_Grid.hpp"

CMmpGL_Grid::CMmpGL_Grid(HWND hWnd, HDC hDC) : CMmpGL(hWnd, hDC)
,m_mtxGrid(3,3)//GRID_VERTEX_COUNT,GRID_VERTEX_COUNT)
,m_vertexGrid(NULL)
,m_colorGrid(NULL)
{
}

CMmpGL_Grid::~CMmpGL_Grid(void)
{
	
}

MMP_RESULT CMmpGL_Grid::Open()
{
    float sinvalue[GRID_VERTEX_COUNT];
    int i, iCol, iRow, iv, ic, iidx;
    float sita,xg, fv;
    BYTE r, g, b;

    xg=2.0f*PI/(float)GRID_VERTEX_COUNT;
    sita=0;
    for(i=0;i<GRID_VERTEX_COUNT;i++)
    {
        sinvalue[i]=(float)sin(sita)*50.0f;
        sita+=xg;
    }

    m_vertexGrid=new GLfloat[ (GRID_VERTEX_COUNT*3)*(GRID_VERTEX_COUNT*3) ];
    m_colorGrid=new GLubyte[ (GRID_VERTEX_COUNT*4)*(GRID_VERTEX_COUNT*4) ];
    m_idxGrid=new GLushort[ GRID_VERTEX_COUNT*GRID_VERTEX_COUNT ];

    iv=0;
    ic=0;
    iidx=0;
    for(iCol=0; iCol<GRID_VERTEX_COUNT; iCol++)
    {
        for(iRow=0; iRow<GRID_VERTEX_COUNT; iRow++)
        {
            fv=sinvalue[iRow];
            fv+=sinvalue[GRID_VERTEX_COUNT-1-iCol];
            //m_mtxGrid.Put(iCol, iRow, fv);

            m_vertexGrid[iv++]=(iRow-(GRID_VERTEX_COUNT/2))*1.0f;
            m_vertexGrid[iv++]=(iCol-(GRID_VERTEX_COUNT/2))*1.0f;
            m_vertexGrid[iv++]=fv;

            if(fv>=0)
            {
               r=255*fv/50;
               g=0;
               b=0;
            }
            else
            {
               r=0;
               g=0;
               b=255*(-fv)/50;
            }

            m_colorGrid[ic++]=r;
            m_colorGrid[ic++]=g;
            m_colorGrid[ic++]=b;
            m_colorGrid[ic++]=255;

            m_idxGrid[iidx]=iidx;
            iidx++;
        }
    }

    
    if( CMmpGL::Open()!=MMP_SUCCESS )
    {
        return MMP_FAILURE;
    }

	return MMP_SUCCESS;
}

MMP_RESULT CMmpGL_Grid::Close()
{
    if( CMmpGL::Close()!=MMP_SUCCESS )
    {
        return MMP_FAILURE;
    }

    if(m_vertexGrid)
    {
        delete [] m_vertexGrid;
        m_vertexGrid=NULL;
    }

    if(m_colorGrid)
    {
        delete [] m_colorGrid;
        m_colorGrid=NULL;
    }

    if(m_idxGrid)
    {
        delete [] m_idxGrid;
        m_idxGrid=NULL;
    }

	return MMP_SUCCESS;
}

void CMmpGL_Grid::Draw_Objects()
{
    this->Draw_Grid();
}

#if 0
void CMmpGL_Grid::Draw_Grid()
{
    int iv, ic,iCol, iRow;
    float fv;
    BYTE r, g, b;
    GLushort LineIdx[GRID_VERTEX_COUNT];

    glVertexPointer(3, GL_FLOAT, 0, m_vertexGrid);
    glColorPointer(4, GL_UNSIGNED_BYTE, 0, m_colorGrid);
         
    for(iCol=0; iCol<GRID_VERTEX_COUNT; iCol++)
    {
        iv=0;
        ic=0;
        for(iRow=0; iRow<GRID_VERTEX_COUNT; iRow++)
        {
            fv=m_mtxGrid.Get(iCol, iRow);
            m_vertexGrid[iv++]=(float)iCol-(GRID_VERTEX_COUNT>>1);
            m_vertexGrid[iv++]=(float)iRow-(GRID_VERTEX_COUNT>>1);
            m_vertexGrid[iv++]=fv;

            if(fv>=0)
            {
               r=255*fv/50;
               g=0;
               b=0;
            }
            else
            {
               r=0;
               g=0;
               b=255*(-fv)/50;
            }

            m_colorGrid[ic++]=r;
            m_colorGrid[ic++]=g;
            m_colorGrid[ic++]=b;
            m_colorGrid[ic++]=255;

            LineIdx[iRow]=iRow;
        }

         glDrawElements(GL_LINES, GRID_VERTEX_COUNT, GL_UNSIGNED_SHORT, LineIdx);
    }
}

#else

void CMmpGL_Grid::Draw_Grid()
{
    glVertexPointer(3, GL_FLOAT, 0, m_vertexGrid);
    glColorPointer(4, GL_UNSIGNED_BYTE, 0, m_colorGrid);
    glDrawElements(GL_LINES, GRID_VERTEX_COUNT*GRID_VERTEX_COUNT, GL_UNSIGNED_SHORT, m_idxGrid);
    //glDrawElements(GL_TRIANGLE_FAN, GRID_VERTEX_COUNT*GRID_VERTEX_COUNT, GL_UNSIGNED_SHORT, m_idxGrid);

    
}

#endif