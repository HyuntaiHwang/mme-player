// Matrix.cpp: implementation of the CMatrix class.
//
//////////////////////////////////////////////////////////////////////

#include "Matrix.h"
#include<memory.h>


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMatrix::CMatrix( int col, int row)
{
  m_Array=0;
  SetSize(col,row);
}

CMatrix::CMatrix( CMatrix& mtx)
{
  *this=mtx;
}

CMatrix::~CMatrix()
{
  DeleteArray(); 
} 

BOOL CMatrix::CreateArray()
{
  if(m_nCol==0 || m_nRow==0 ) { m_nCol=m_nRow=0; m_Array=0; return FALSE;}

  int col, row, i;
  m_Array=new float*[m_nCol];
  if(!m_Array) 
  {
	  m_nCol=m_nRow=0;
	  return FALSE;
  }
  for(  col=0; col<m_nCol; col++) m_Array[col]=0;

  for(  col=0; col<m_nCol; col++)
  {
	  m_Array[col]=new float[m_nRow];
	  if(!m_Array[col]  )
	  {
		  for( i=0; i<col; i++)  
		  {
			  if(m_Array[i])  delete [] m_Array[i];
			  m_Array[i]=0;
		  }
		  delete [] m_Array;
		  m_Array=0;
          m_nCol=m_nRow=0;
		  return FALSE;
	  }
  }

  for( col=0; col<m_nCol; col++) for( row=0; row<m_nRow; row++)
	  m_Array[col][row]=0.0f;

  return TRUE;
}

void CMatrix::DeleteArray()
{
  int col;
  if(m_Array)
  {
    for( col=0; col<m_nCol; col++) 	
	{
		if(m_Array) delete [] m_Array[col];
		m_Array[col]=0;
	}
    delete [] m_Array; 
  }
  m_Array=0;
}

float CMatrix::Get( int col, int row )// { return m_Array[col][row]; }
{
   if( col<0 || col >= GetCol() || row<0 || row >= GetRow() ) return 0.0;
   return m_Array[col][row];
}

BOOL CMatrix::SetSize(int col, int row )
{
  DeleteArray();
  m_nRow=row, m_nCol=col;
  return CreateArray();
}


CMatrix& CMatrix::operator=(CMatrix& mtx)
{
  this->DeleteArray();
  m_nCol=mtx.GetCol();
  m_nRow=mtx.GetRow();
  this->CreateArray();

  int col, row;
  for( col=0; col<m_nCol; col++) for( row=0; row<m_nRow; row++) 
	  m_Array[col][row]=mtx.m_Array[col][row];
  return *this;
}

CMatrix& CMatrix::operator+=(CMatrix& mtx)
{
  if( this->GetRow() != mtx.GetRow() ) return *this;
  if( this->GetCol() != mtx.GetCol() ) return *this;
  
  int col, row;
  for( col=0; col<m_nCol; col++) for(row=0; row<m_nRow; row++)
	  m_Array[col][row]+=mtx.m_Array[col][row];
  return *this;
}

CMatrix& CMatrix::operator-=(CMatrix& mtx)
{
  if( this->GetRow() != mtx.GetRow() ) return *this;
  if( this->GetCol() != mtx.GetCol() ) return *this;
  
  int col, row;
  for( col=0; col<m_nCol; col++) for(row=0; row<m_nRow; row++)
	  m_Array[col][row]-=mtx.m_Array[col][row];
  return *this;
}

CMatrix& CMatrix::operator*=(CMatrix& mtx)
{
  if( this->GetRow() != mtx.GetCol() ) return *this;
  
  CMatrix M( this->GetCol(), mtx.GetRow() );
  int col, row, i;
  float v;

  for( col=0; col<M.GetCol(); col++) for( row=0; row<M.GetRow(); row++)
  {
     for( v=0.0f, i=0; i<this->GetRow(); i++)
         v+=this->Get(col, i)*mtx.Get(i, row);
	 M.Put( col, row, v );
  }
  *this=M;
  return *this;
}

CMatrix& CMatrix::operator*=(float v)
{
  int col, row;

  for( col=0; col<GetCol(); col++) for( row=0; row<GetRow(); row++)
  {
     m_Array[col][row]*=v;
  }
  return *this;
}

CMatrix& CMatrix::Add(CMatrix& mtx1, CMatrix& mtx2)
{
    *this=mtx1;
	*this+=mtx2;
	return *this;
}

CMatrix& CMatrix::Sub(CMatrix& mtx1, CMatrix& mtx2)
{
   *this=mtx1;
   *this-=mtx2;
    return *this;
}
 
CMatrix& CMatrix::Multi(CMatrix& mtx1, CMatrix& mtx2)
{
   *this=mtx1;
   (*this)*=mtx2;
    return *this;
}


float CMatrix::GetDet()
{
  if( m_nRow != m_nCol ) return 0;

  if( m_nRow ==1 ) return Get(0,0);
  else if( m_nRow<=0 ) return 0;

  float value;
  value=CalDet(*this);

  return value;
}

float CMatrix::CalDet( CMatrix& mtx)
{
	int i;
	int col,row;
	float value;
	CMatrix tempmtx( mtx.GetCol()-1, mtx.GetRow()-1 );

	if( mtx.GetCol() == 1 ) return mtx.Get(0,0);

 	for(value=0, i=0;i<mtx.GetRow();i++)
	 {
	   for(col=0; col< tempmtx.GetCol(); col++)
		  for(row=0; row< tempmtx.GetRow(); row++)
	         {
		       if( row<i)   tempmtx.Put( col, row, mtx.Get( col+1, row ) );
			   else 	  tempmtx.Put( col, row, mtx.Get( col+1, row+1 ) );
	         }


	   if( i%2==0) value+=mtx.Get(0, i)*CalDet(tempmtx);
	   else        value-=mtx.Get(0, i)*CalDet(tempmtx);
	}
	

	return value;
}