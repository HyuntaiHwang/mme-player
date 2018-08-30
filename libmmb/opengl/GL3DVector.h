// 3DVector.h: interface for the CGL3DVector class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_3DVECTOR_H__3E0911D2_3692_4F91_89F5_BAF1E8E8D575__INCLUDED_)
#define AFX_3DVECTOR_H__3E0911D2_3692_4F91_89F5_BAF1E8E8D575__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//�� Ŭ������ ���� ��ǥ�踦 �̿��� �����̴�. 
//�⺻�� �̷���  CAD/CAM p65 �� �����Ѵ�.
//     x=r*sin(v)*cos(u)
//     y=r*sin(v)*sin(u)
//     z=r*cos(v)
// 

#include<math.h>
//#include<iostream.h>

const float PI =3.141592653589793238f;
const float ZEROLIMIT=0.00000001f;

class  CGL3DVector  
{
public:
	//������,  flag�� ���̸� (r,u,v)�̸� �����̸� (x,y,z)�̴�. 
	CGL3DVector( float r=0.0f, float u=0.0f, float v=0.0f, bool flag=false );
	CGL3DVector( CGL3DVector& vec);
	virtual ~CGL3DVector();

private :

    //float m_r;  //������ 
	//float m_u, m_v; //���� (radian)
    float m_x, m_y, m_z;

public :
	
	float X();// { return x; }
	float Y();// { return y; }
	float Z();// { return z; }
	float R();// { return m_r; }
	float U();// { return m_u; }
	float V();// { return m_v; }
    void Put_XYZ( float x, float y, float z );
	void Put_RUV( float r, float u, float v );
	void Put_X( float x ) { m_x=x; }
	void Put_Y( float y ) { m_y=y; }
	void Put_Z( float z ) { m_z=z; }

    CGL3DVector& operator=( CGL3DVector& vec); 
	CGL3DVector& operator+=( CGL3DVector& vec);
    CGL3DVector& operator-=( CGL3DVector& vec);
	CGL3DVector& operator*=( float value );
	CGL3DVector& operator/=( float value );

	CGL3DVector& RotateX( float sita ); //x������ ȸ�� 
    CGL3DVector& RotateY( float sita ); //y������ ȸ�� 
    CGL3DVector& RotateZ( float sita ); //z������ ȸ�� 

	void Nomalize(); //����ȭ 
	bool IsZeroVector();

	friend  CGL3DVector operator+( CGL3DVector& vec1, CGL3DVector& vec2);
    friend CGL3DVector operator-( CGL3DVector& vec1, CGL3DVector& vec2);
    friend float     operator*( CGL3DVector& vec1, CGL3DVector& vec2 ); //���� 
	friend CGL3DVector operator&( CGL3DVector& vec1, CGL3DVector& vec2 );//���� 
 	friend bool operator==( CGL3DVector& vec1, CGL3DVector& vec2 ) //�� ���Ͱ� ���� �������� 
	{
		float len;
		float flimit=ZEROLIMIT;

		len=(float)fabs( vec1.X()-vec2.X() );
		len+=(float)fabs( vec1.Y()-vec2.Y() );
		len+=(float)fabs( vec1.Z()-vec2.Z() );

		if( len > flimit ) return false;
		else return true;
			
	    //if( vec1.X() != vec2.X() ) return false;
		//if( vec1.Y() != vec2.Y() ) return false;
		//if( vec1.Z() != vec2.Z() ) return false;
		//return true;
	}
    
	static  float   GetAngle( CGL3DVector& vec1, CGL3DVector& vec2 ); //�� ������ ���հ� ���ϱ� 

    //friend ostream& operator<<( ostream& os, CGL3DVector& v );

	// vec1, vec2�� a:b�� �����ϴ� ���͸� ���� ���� 
    static CGL3DVector GetInterior( CGL3DVector& vec1, CGL3DVector& vec2, float a, float b );

	//���� ���͸� ���Ѵ�. 
	static CGL3DVector GetCrossProduct( CGL3DVector& vec1, CGL3DVector& vec2 );

	//�� ���Ͱ� ���� ���� �������� �ƴ��� �Ǵ��Ѵ�. 
	static bool  IsEqualDir( CGL3DVector& vec1, CGL3DVector& vec2 );

};

#endif // !defined(AFX_3DVECTOR_H__3E0911D2_3692_4F91_89F5_BAF1E8E8D575__INCLUDED_)
