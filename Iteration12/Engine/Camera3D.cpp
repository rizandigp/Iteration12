#pragma once

#include "Camera3D.h"
#include "Ray.h"


Camera3D::Camera3D() 
{
	// Defaults
	m_Pos = Vector3( 0.0f , 0.0f , 0.0f );
	m_LookAt = Vector3( 1.0f , 0.0f , 0.0f );
	m_Up = Vector3( 0.0f , 0.0f , 1.0f );
	m_Fov = (float)XM_PI / 4.0f;		// 45 degrees
	m_ZNear = 0.1f;
	m_ZFar = 100000.0f;
	m_Aspect = 0.7853981633974483f;
	ZeroMemory( &m_View, sizeof(XMMATRIX));
	ZeroMemory( &m_Projection, sizeof(XMMATRIX));
	ZeroMemory( &m_ViewProjection, sizeof(XMMATRIX));
	m_ViewProjection = Matrix4x4::IDENTITY;
}

void Camera3D::Update( float DeltaTime )
{
	XMMATRIX view, projection, viewprojection;
	XMFLOAT4X4 xmView, xmProjection, xmViewProjection;
	// view matrix
	view = XMMatrixLookAtRH( XMVectorSet(m_Pos.x, m_Pos.y, m_Pos.z, 0.0f) ,  XMVectorSet(m_LookAt.x, m_LookAt.y, m_LookAt.z, 0.0f) , XMVectorSet(m_Up.x, m_Up.y, m_Up.z, 0.0f) );
	XMStoreFloat4x4( &xmView, view );
	// projection matrix
	projection = XMMatrixPerspectiveFovLH(m_Fov , m_Aspect , m_ZNear , m_ZFar );
	ComputeFrustumFromProjection( &m_Frustum, &projection );
	m_Frustum.Origin = m_Pos.intoXMFLOAT3();

	projection = XMMatrixPerspectiveFovRH(m_Fov , m_Aspect , m_ZNear , m_ZFar );
	XMStoreFloat4x4( &xmProjection, projection );
	// view * projection
	viewprojection = XMMatrixMultiply( view, projection );
	XMStoreFloat4x4( &xmViewProjection, viewprojection );

	// Store matrices
	m_ViewProjection = Matrix4x4(xmViewProjection._11, xmViewProjection._12, xmViewProjection._13, xmViewProjection._14,
								xmViewProjection._21, xmViewProjection._22, xmViewProjection._23, xmViewProjection._24,
								xmViewProjection._31, xmViewProjection._32, xmViewProjection._33, xmViewProjection._34,
								xmViewProjection._41, xmViewProjection._42, xmViewProjection._43, xmViewProjection._44);
	m_Projection =	Matrix4x4(xmProjection._11, xmProjection._12, xmProjection._13, xmProjection._14,
								xmProjection._21, xmProjection._22, xmProjection._23, xmProjection._24,
								xmProjection._31, xmProjection._32, xmProjection._33, xmProjection._34,
								xmProjection._41, xmProjection._42, xmProjection._43, xmProjection._44);
	m_View = Matrix4x4(xmView._11, xmView._12, xmView._13, xmView._14,
						xmView._21, xmView._22, xmView._23, xmView._24,
						xmView._31, xmView._32, xmView._33, xmView._34,
						xmView._41, xmView._42, xmView._43, xmView._44);

	Vector3 v = m_LookAt - GetPosition();
	v.normalise();
	Quaternion orient;
	orient.lookAt( v, Vector3::UNIT_Z );

	m_Frustum.Orientation = orient.intoXMFLOAT4();
	m_Orient = orient;
}


void Camera3D::SetPosition(const Vector3& Pos)
{
	m_Pos = Pos;
}

void Camera3D::SetLookAt(const Vector3& LookAt)
{
	m_LookAt = LookAt;
}

void Camera3D::SetUpVector(const Vector3& Up)
{
	m_Up = Up;
}

void Camera3D::SetView(const Vector3& Pos, const Vector3& LookAt)
{
	m_Pos = Pos;
	m_LookAt = LookAt;
}

void Camera3D::SetProjection(float Fov, float Aspect, float NearPlane, float FarPlane)
{
	m_Fov = Fov;
	m_Aspect = Aspect;
	m_ZNear = NearPlane;
	m_ZFar = FarPlane;
}

void Camera3D::SetOrientation( const Quaternion& quaternion )
{
	m_Orient = quaternion;
}

void Camera3D::GetProjectionParameters( float &Fov , float &Aspect , float &NearPlane , float &FarPlane ) const
{
	Fov = m_Fov;
	Aspect = m_Aspect;
	NearPlane = m_ZNear;
	FarPlane = m_ZFar;
}

Vector3 Camera3D::GetDirectionVector()
{
	return (m_LookAt-m_Pos).normalisedCopy();
}



// kelas CameraFPS
CameraFPS::CameraFPS()
{
	Camera3D::Camera3D();
	m_Motion = Vector3( 0.0f , 0.0f , 0.0f );
	m_Orient = Quaternion();
	m_Dir = Vector3( 0.0f , 0.0f , 0.0f );
}

void CameraFPS::SetLookAt(const Vector3& LookAt)
{
	Camera3D::SetLookAt( LookAt );
	XMFLOAT3 temp = XMFLOAT3(m_LookAt.x - m_Pos.x, m_LookAt.y - m_Pos.y, m_LookAt.z - m_Pos.z);
	m_Dir.x = atan( temp.y/temp.x );
	m_Dir.y = atan( temp.z/sqrt(temp.x*temp.x + temp.y*temp.y) );
}

Vector3 CameraFPS::GetDirectionVector()
{
	return (m_LookAt-m_Pos);
}

void CameraFPS::Update(float DeltaTime)
{
	//
	// SEMENTARA
	//
	XMFLOAT3 vCamMovement( 0.0f , 0.0f , 0.0f );

	if ( KEYDOWN( 'A' ) )
		vCamMovement.x -= 2.0f*DeltaTime;
	if ( KEYDOWN( 'D' ) )
		vCamMovement.x += 2.0f*DeltaTime;
	if ( KEYDOWN( 'W' ) )
		vCamMovement.y -= 2.0f*DeltaTime;
	if ( KEYDOWN( 'S' ) )
		vCamMovement.y += 2.0f*DeltaTime;

	m_Pos.x += vCamMovement.y*cos(m_Dir.x)*cos(m_Dir.y);			//
	m_Pos.y += vCamMovement.y*sin(m_Dir.x)*cos(m_Dir.y);			// Maju-mundur
	m_Pos.z += vCamMovement.y*sin(m_Dir.y);							//

	m_Pos.x += vCamMovement.x*sin(-m_Dir.x);						// Kiri-Kanan
	m_Pos.y += vCamMovement.x*cos(-m_Dir.x);						//



	POINT ptMousePos;
	XMFLOAT2 ptMousePosDelta;

	GetCursorPos( &ptMousePos );


	if ( KEYDOWN( VK_LBUTTON ) && ( m_bMouseDragged == false ))
		{
		m_bMouseDragged = true;
		m_ptLastMousePos = ptMousePos; 
		}

	if ( KEYDOWN( VK_LBUTTON ) )
		{
		m_bMouseDragged = true;

		ptMousePosDelta.x = float(m_ptLastMousePos.x - ptMousePos.x);		// Yaw
		ptMousePosDelta.y = float(m_ptLastMousePos.y - ptMousePos.y);		// Pitch
		ptMousePosDelta.x /= 200.0f;
		ptMousePosDelta.y /= 200.0f;

		m_Dir.x += ptMousePosDelta.x;
		m_Dir.y += ptMousePosDelta.y;

		SetCursorPos( m_ptLastMousePos.x , m_ptLastMousePos.y );
		}
	else
		m_bMouseDragged = false;

	m_LookAt.x = m_Pos.x + cos(m_Dir.x)*cos(m_Dir.y);
	m_LookAt.y = m_Pos.y + sin(m_Dir.x)*cos(m_Dir.y);
	m_LookAt.z = m_Pos.z + sin(m_Dir.y);

	Camera3D::Update( DeltaTime );
}

Ray Camera3D::GetRayFromScreenPosition( Vector2 xy )
{
	float WIDTH_DIV_2 = 0.5f;
	float HEIGHT_DIV_2 = 0.5f;
	float dx=tanf(m_Fov*0.5f)*(xy.x/WIDTH_DIV_2-1.0f)*m_Aspect;
	float dy=tanf(m_Fov*0.5f)*(1.0f-xy.y/HEIGHT_DIV_2);
	
	// Negating the Z component only necessary for right-handed coordinates
	Vector4 start(dx*m_ZNear, dy*m_ZNear, -m_ZNear, 1.0f);
	Vector4 end(dx*m_ZFar, dy*m_ZFar, -m_ZFar, 1.0f);

	Matrix4x4 invView = m_View.inverse();
	start = start*invView;
	end = end*invView;

	return Ray(Vector3(start.x,start.y,start.z), Vector3(end.x-start.x,end.y-start.y,end.z-start.z).normalisedCopy());
}