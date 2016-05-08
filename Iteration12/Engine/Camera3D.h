#pragma once

#include "Prerequisites.h"

#include "xnaCollision.h"
#include "RenderDispatcher.h"
#include "RenderCommand.h"
#include "Transform.h"
#include "Math.h"
#include "Buffer.h"
#include "Matrix4x4.h"
#include "Vector3.h"
#include "Quaternion.h"


// Gold
#define KEYDOWN(vk_code)  ((GetAsyncKeyState(vk_code) & 0x8000) )//? 1 : 0)
#define KEYUP(vk_code)  ((GetAsyncKeyState(vk_code) & 0x8000) )//? 0 : 1)


class Camera3D
{
public:
	Camera3D();

	virtual void SetPosition( const Vector3& Pos );
	virtual void SetLookAt( const Vector3& LookAt );
	virtual void SetUpVector( const Vector3& Up );
	virtual void SetView( const Vector3& Pos , const Vector3& LookAt );
	virtual void SetProjection( float Fov , float Aspect , float NearPlane , float FarPlane );
	virtual void SetOrientation( const Quaternion& quat );
	virtual void SetNearPlane( float ZNear )	{ m_ZNear = ZNear; }
	virtual void SetFarPlane( float ZFar )		{ m_ZFar = ZFar; }

	virtual void SetViewMatrix( const Matrix4x4& mat )				{ m_View = mat; };
	virtual void SetProjectionMatrix( const Matrix4x4& mat )		{ m_Projection = mat; };
	virtual void SetViewProjectionMatrix( const Matrix4x4& mat )	{ m_ViewProjection = mat; };
	virtual void SetFrustum( const XNA::Frustum* Frustum )			{ m_Frustum = *Frustum; };

	virtual void Update( float DeltaTime );

	virtual Vector3	GetPosition()					{return m_Pos;};
	virtual Vector3	GetLookAtPoint()				{return m_LookAt;};
	virtual Vector3	GetUp()							{return m_Up;};
	virtual float GetFov()							{return m_Fov;};
	virtual float GetAspect()						{return m_Aspect;};
	virtual float GetNearPlane()					{return m_ZNear;};
	virtual float GetFarPlane()						{return m_ZFar;};
	virtual Matrix4x4	GetViewMatrix()				{return m_View;};
	virtual Matrix4x4	GetProjectionMatrix()		{return m_Projection;};
	virtual Matrix4x4	GetViewProjectionMatrix()	{return m_ViewProjection;};
	virtual XNA::Frustum	GetFrustum()			{return m_Frustum; };
	virtual Quaternion	GetOrientation()			{return m_Orient; };
	virtual Vector3	GetDirectionVector();

	virtual void GetProjectionParameters( float &Fov , float &Aspect , float &NearPlane , float &FarPlane ) const;
	Ray Camera3D::GetRayFromScreenPosition( Vector2 xy );

protected:
	Vector3 m_Pos;
	Vector3 m_LookAt;
	Vector3 m_Dir;
	Quaternion m_Orient;
	Vector3 m_Up;

	Matrix4x4 m_View;
	Matrix4x4 m_Projection;
	Matrix4x4 m_ViewProjection;

	float m_Fov;
	float m_Aspect;
	float m_ZNear;
	float m_ZFar;

	XNA::Frustum m_Frustum;
};



class CameraFPS : public Camera3D
{
public:
	CameraFPS();

	void SetLookAt( const Vector3& pLookAt );

	void Update( float DeltaTime );

	Vector3	GetDirectionVector();

	inline Quaternion GetOrientation()	{ return m_Orient; };

protected:
	Vector3 m_Motion;
	Quaternion m_Orient;
	POINT m_ptLastMousePos;
	bool m_bMouseDragged;
};