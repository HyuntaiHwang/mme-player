// 3DVector.cpp: implementation of the CGL3DVector class.
//
//////////////////////////////////////////////////////////////////////

#include "GL3DVector.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CGL3DVector::CGL3DVector( float r, float u, float v, bool flag )
{
   if(flag) Put_RUV(r,u,v);
   else     Put_XYZ(r,u,v);
}

CGL3DVector::CGL3DVector( CGL3DVector& vec)
{
	(*this)=vec;
}

CGL3DVector::~CGL3DVector()
{

}

void CGL3DVector::Put_XYZ( float x, float y, float z )
{
  m_x=x, m_y=y, m_z=z;
}

void CGL3DVector::Put_RUV( float r, float u, float v )
{
  m_x=(float)(r*cos(u)*sin(v));
  m_y=(float)(r*sin(u));
  m_z=(float)(r*cos(u)*cos(v));
}


float CGL3DVector::X() { return m_x; }
float CGL3DVector::Y() { return m_y; }
float CGL3DVector::Z() { return m_z; }
float CGL3DVector::R() { return sqrt( m_x*m_x+m_y*m_y+m_z*m_z ); }

float CGL3DVector::U() 
{ 
    float r, u;

	r=R();
    if( r==0.0f )  return 0.0f;

	if( m_y >= r ) u=PI/2;
	else if( m_y <= -r ) u=-PI/2;
	else  u=asin(m_y/r);
	

	return (float)u;
}

float CGL3DVector::V() 
{ 
  float r, u, v;

  r=R();
  if( r==0.0f )  return 0.0f;

  if( m_y >= r ) u=PI/2;
  else if( m_y <= -r ) u=-PI/2;
  else           u=asin(m_y/r);
  
  r*=cos(u);

  if( r==0.0f ) return 0.0f;

  if( m_z >= r ) v=0;
  else if( m_z <= -r ) v=PI;
  else v=acos(m_z/r);
  if( m_x < 0.0f ) v*=-1;

  return v;
}
    
CGL3DVector& CGL3DVector::operator =( CGL3DVector& vec )
{
  m_x=vec.m_x, m_y=vec.m_y, m_z=vec.m_z; 
  return *this;
}

CGL3DVector& CGL3DVector::operator+=( CGL3DVector& vec)
{
   Put_XYZ( X()+vec.X(), Y()+vec.Y(), Z()+vec.Z() );
   return *this;
}

CGL3DVector& CGL3DVector::operator-=( CGL3DVector& vec)
{
   Put_XYZ( X()-vec.X(), Y()-vec.Y(), Z()-vec.Z() );
   return *this;
}

CGL3DVector& CGL3DVector::operator*=( float value )
{
   Put_XYZ( X()*value, Y()*value, Z()*value );
   return *this;
}

CGL3DVector& CGL3DVector::operator/=( float value )
{
   Put_XYZ( X()/value, Y()/value, Z()/value );
   return *this;
}

CGL3DVector operator+( CGL3DVector& vec1, CGL3DVector& vec2)
{
   CGL3DVector v(vec1);
   v+=vec2;
   return v;
}

CGL3DVector operator-( CGL3DVector& vec1, CGL3DVector& vec2)
{
   CGL3DVector v(vec1);
   v-=vec2;
   return v;
}

#if 0
ostream& operator<<( ostream& os, CGL3DVector& v )
    {
      float x,y,z;
	  x=v.X(), y=v.Y(), z=v.Z();
	  //if( fabs(x) < 0.001 ) x=0;
      //if( fabs(y) < 0.001 ) y=0;
      //if( fabs(z) < 0.001 ) z=0;
	  os<<"( "<<x<<", "<<y<<", "<<z<<" )";
	  return os;
    }
#endif

//내적 
float  operator*( CGL3DVector& vec1, CGL3DVector& vec2 ) 
{
    return ( vec1.X()*vec2.X()+vec1.Y()*vec2.Y()+vec1.Z()*vec2.Z() );
}

//외적 
CGL3DVector operator&( CGL3DVector& vec1, CGL3DVector& vec2 )
{
   float a1, b1, c1;
   float a2, b2, c2;
   CGL3DVector vec;

   a1=vec1.X(), b1=vec1.Y(), c1=vec1.Z();
   a2=vec2.X(), b2=vec2.Y(), c2=vec2.Z();


   vec.Put_XYZ(   b1*c2-c1*b2, c1*a2-a1*c2 , a1*b2-a2*b1 );

   return vec;

   
}

//두 벡터의 사잇각 구하기( 단위 : radian ) 
float   CGL3DVector::GetAngle( CGL3DVector& vec1, CGL3DVector& vec2 ) 
{
   float r;
   float dot;

   r=vec1.R()*vec2.R();
   dot=vec1*vec2;

   if( r==0.0f ) return 0.0f;

   if( dot >= r )    return 0.0f;
   else if( dot<=-r) return PI;
   else          
   {
   	   return acos( dot/r ); 
   }
}

CGL3DVector& CGL3DVector::RotateX( float sita ) //x축으로 회전 
{
  float x, y, z;
  float a=-sita;
  
  x=X();
  z=cos(a)*Z()-sin(a)*Y();
  y=sin(a)*Z()+cos(a)*Y();

  Put_XYZ(x,y,z);

  return *this;
}

CGL3DVector& CGL3DVector::RotateY( float sita ) //y축으로 회전 
{
  float x, y, z;
  float a=-sita;
  
  x=cos(a)*X()-sin(a)*Z();
  y=Y();
  z=sin(a)*X()+cos(a)*Z();

  Put_XYZ(x,y,z);

  return *this;
}

CGL3DVector& CGL3DVector::RotateZ( float sita ) //z축으로 회전 
{
  float x, y, z;
  float a=sita;
  
  x=cos(a)*X()-sin(a)*Y();
  y=sin(a)*X()+cos(a)*Y();
  z=Z();

  Put_XYZ(x,y,z);

  return *this;
}

void CGL3DVector::Nomalize()
{
  float r=this->R();
  if( r==0.0f ) Put_XYZ( 0.0f, 0.0f, 0.0f );
  else Put_XYZ( m_x/r, m_y/r, m_z/r );
}

// vec1, vec2를 a:b로 내분하는 벡터를 구해 리턴 
CGL3DVector CGL3DVector::GetInterior( CGL3DVector& vec1, CGL3DVector& vec2, float a, float b )
{
  CGL3DVector vec;
  float x1,y1,z1;
  float x2,y2,z2;
  float x,y,z;
  
  x1=vec1.X(), y1=vec1.Y(), z1=vec1.Z();
  x2=vec2.X(), y2=vec2.Y(), z2=vec2.Z();
  
  x=( b*x1+a*x2)/(a+b);
  y=( b*y1+a*y2)/(a+b);
  z=( b*z1+a*z2)/(a+b);

  vec.Put_XYZ(x, y, z );

  return vec;
}


//외적 구하기 
CGL3DVector CGL3DVector::GetCrossProduct( CGL3DVector& vec1, CGL3DVector& vec2 )
{
   float a1, b1, c1;
   float a2, b2, c2;
   CGL3DVector vec;

   a1=vec1.X(), b1=vec1.Y(), c1=vec1.Z();
   a2=vec2.X(), b2=vec2.Y(), c2=vec2.Z();


   vec.Put_XYZ(   b1*c2-c1*b2, c1*a2-a1*c2 , a1*b2-a2*b1 );

   return vec;
}

bool CGL3DVector::IsZeroVector()
{
	if( m_x < -ZEROLIMIT || m_x > ZEROLIMIT ) return false;
	if( m_y < -ZEROLIMIT || m_y > ZEROLIMIT ) return false;
	if( m_z < -ZEROLIMIT || m_z > ZEROLIMIT ) return false;
	
	return true;
}


//두 벡터가 서로 같은 방향인지 아닌지 판단한다. 
//두 벡터가 평행하다고 가정한다.
bool CGL3DVector::IsEqualDir( CGL3DVector& vec1, CGL3DVector& vec2 )
{
  float k1,k2;
  float flimit=0.000000001;

  if( vec1.X() < -flimit || vec1.X() > flimit )  { k1=vec1.X(), k2=vec2.X(); }
  else if( vec1.Y() < -flimit || vec1.Y() > flimit  )  { k1=vec1.Y(), k2=vec2.Y(); }
  else if( vec1.Z() < -flimit || vec1.Z() > flimit  )  { k1=vec1.Z(), k2=vec2.Z(); }
  else return false;
 
  if( k2/k1 > 0 )  return true;  //k2가 0일 수 없다. 평행하다고 가정했으므로
  else return false;
}