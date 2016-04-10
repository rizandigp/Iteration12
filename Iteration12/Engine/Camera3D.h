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

// TODO : Use the new matrix & vector classes
class Camera3D
{
public:
	Camera3D();

	virtual void setPosition( const Vector3& pPos );
	virtual void setLookAt( const Vector3& pLookAt );
	virtual void setUpVector( const Vector3& pUp );
	virtual void setView( const Vector3& pPos , const Vector3& pLookAt );
	virtual void setProjection( float Fov , float Aspect , float NearPlane , float FarPlane );
	virtual void setOrientation( const Quaternion& quaternion );
	virtual void setNearPlane( float ZNear )	{ m_ZNear = ZNear; }
	virtual void setFarPlane( float ZFar )		{ m_ZFar = ZFar; }

	virtual void setViewMatrix( const Matrix4x4& mat )				{ m_View = mat; };
	virtual void setProjectionMatrix( const Matrix4x4& mat )		{ m_Projection = mat; };
	virtual void setViewProjectionMatrix( const Matrix4x4& mat )	{ m_ViewProjection = mat; };
	virtual void setFrustum( XNA::Frustum* frustum )				{ m_Frustum = *frustum; };

	virtual void update( float DeltaTime );

	virtual Vector3	getPosition()					{return m_Pos;};
	virtual Vector3	getLookAtPoint()				{return m_LookAt;};
	virtual Vector3	getUp()							{return m_Up;};
	virtual float getFov()							{return m_Fov;};
	virtual float getAspect()						{return m_Aspect;};
	virtual float getNearPlane()					{return m_ZNear;};
	virtual float getFarPlane()						{return m_ZFar;};
	virtual Matrix4x4	getViewMatrix()				{return m_View;};
	virtual Matrix4x4	getProjectionMatrix()		{return m_Projection;};
	virtual Matrix4x4	getViewProjectionMatrix()	{return m_ViewProjection;};
	virtual XNA::Frustum	getFrustum()			{return m_Frustum; };
	virtual Quaternion	getOrientation()			{return m_Orient; };
	virtual Vector3	getDirectionVector();

	virtual void getProjectionParameters( float &Fov , float &Aspect , float &NearPlane , float &FarPlane ) const;
	Ray Camera3D::getRayFromScreenPosition( Vector2 xy );

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

	void setLookAt( const Vector3& pLookAt );

	void update( float DeltaTime );

	Vector3	getDirectionVector();

	//inline XMFLOAT4 getOrientation()	{ return m_Orient; };

protected:
	Vector3 m_Motion;
	Quaternion m_Orient;
	POINT m_ptLastMousePos;
	bool m_bMouseDragged;
};