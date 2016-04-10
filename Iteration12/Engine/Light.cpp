#pragma once

#include "Light.h"

PointLight::PointLight() :
	m_Color(Vector3(1.0f,1.0f,1.0f)), 
	m_fRadius(10.0f), 
	m_fIntensity(10.0f), 
	m_bEnabled(true) 
	{};

PointLight::PointLight( Vector3 position, Vector3 color, float radius, float intensity ) : 
	m_Color(color), 
	m_fRadius(radius), 
	m_fIntensity(intensity), 
	m_bEnabled(true) 
	{ 
		m_Transform.setPosition( position );
	};

void PointLight::update()
{
	m_Transform.update();
}

void PointLight::cull( XNA::Frustum* frustum )
{
	XNA::Sphere lightSphere;
	lightSphere.Center = m_Transform.getPosition().intoXMFLOAT3();
	lightSphere.Radius = m_fRadius;
	m_bVisible = ( XNA::IntersectSphereFrustum( &lightSphere, frustum ) > 0 );
}

SpotLight::SpotLight() :
	m_Color(Vector3(1.0f,1.0f,1.0f)), 
	m_Radius(10.0f), 
	m_Intensity(10.0f), 
	m_bEnabled(true), 
	m_bRSMEnabled(false), 
	m_pCookie(NULL), 
	m_pShadowmap(NULL), 
	m_Cone(XM_PIDIV4,XM_PIDIV4), 
	m_bShadowCasting(true)	
	{
		m_pCamera = new Camera3D();
	};


void SpotLight::pointTo( Vector3 target, float roll )
{
	m_Transform.lookAt( target, roll );
}
/*
// TODO : Sort this mess out
void SpotLight::update()
{
	XMFLOAT3 local( 0.0f, 0.0f, 1.0f );		// forward vector in local space
	XMFLOAT3 localup( 0.0f, 1.0f, 0.0f );	// up vector in local space
	XMMATRIX mat = XMMatrixRotationQuaternion( XMLoadFloat4(&m_Transform.getOrientation().intoXMFLOAT4()) );
	XMFLOAT3 transformed;
	XMStoreFloat3( &transformed, XMVector3Transform( XMLoadFloat3(&local), mat ) );
	XMVECTOR up = XMVector3Transform( XMLoadFloat3(&localup), mat );
	XMMATRIX view = XMMatrixLookAtLH( XMLoadFloat3(&transform()->getPosition().intoXMFLOAT3()) ,  XMVectorSet(transform()->getPosition().x+transformed.x, transform()->getPosition().y+transformed.y, transform()->getPosition().z+transformed.z, 0.0f) , up ); 
	XMMATRIX projection = XMMatrixPerspectiveFovRH(m_Cone.y , m_Cone.x/m_Cone.y , 0.1f , m_Radius );

	XMFLOAT4X4 proj, viewProj;
	XMStoreFloat4x4( &proj, projection );
	XMStoreFloat4x4( &viewProj, XMMatrixMultiply( view, projection ) );
	m_ViewProjection = Matrix4x4(viewProj._11, viewProj._12, viewProj._13, viewProj._14,
								viewProj._21, viewProj._22, viewProj._23, viewProj._24,
								viewProj._31, viewProj._32, viewProj._33, viewProj._34,
								viewProj._41, viewProj._42, viewProj._43, viewProj._44);
	m_Projection =	Matrix4x4(proj._11, proj._12, proj._13, proj._14,
								proj._21, proj._22, proj._23, proj._24,
								proj._31, proj._32, proj._33, proj._34,
								proj._41, proj._42, proj._43, proj._44);
	m_View = Matrix4x4(view._11, view._12, view._13, view._14,
						view._21, view._22, view._23, view._24,
						view._31, view._32, view._33, view._34,
						view._41, view._42, view._43, view._44);

	XNA::ComputeFrustumFromProjection( &m_Frustum, &projection );
	m_Frustum.Origin = transform()->getPosition().intoXMFLOAT3();
	m_Frustum.Orientation = m_Transform.getOrientation().intoXMFLOAT4();

	m_Transform.update();
}*/

void SpotLight::update()
{
	// Forward vector in local space
	Vector3 forward( 0.0f, 0.0f, -1.0f );
	// Up vector in local space
	Vector3 up( 0.0f, 1.0f, 0.0f );			
	// Target in local space
	Vector3 target = m_Transform.getOrientation()*forward;
	// Target in worldspace
	target = m_Transform.getPosition()-target;

	// Set view & projection
	m_pCamera->setView( m_Transform.getPosition(), target );
	m_pCamera->setProjection( m_Cone.y , m_Cone.x/m_Cone.y, 0.1f, m_Radius );
	m_pCamera->update( 0.0f );

	m_Transform.update();
	
	// Old code
	XMFLOAT3 local( 0.0f, 0.0f, 1.0f );		// forward vector in local space
	XMFLOAT3 localup( 0.0f, 1.0f, 0.0f );	// up vector in local space
	XMMATRIX mat = XMMatrixRotationQuaternion( XMLoadFloat4(&m_Transform.getOrientation().intoXMFLOAT4()) );
	XMFLOAT3 transformed;
	XMStoreFloat3( &transformed, XMVector3Transform( XMLoadFloat3(&local), mat ) );
	XMVECTOR upvec = XMVector3Transform( XMLoadFloat3(&localup), mat );
	XMMATRIX view = XMMatrixLookAtLH( XMLoadFloat3(&transform()->getPosition().intoXMFLOAT3()) ,  XMVectorSet(transform()->getPosition().x+transformed.x, transform()->getPosition().y+transformed.y, transform()->getPosition().z+transformed.z, 0.0f) , upvec ); 
	XMMATRIX projection = XMMatrixPerspectiveFovLH(m_Cone.y , m_Cone.x/m_Cone.y , 0.1f , m_Radius );

	XMFLOAT4X4 proj, vi, viewProj;
	XMStoreFloat4x4( &proj, projection );
	XMStoreFloat4x4( &vi, view );
	XMStoreFloat4x4( &viewProj, XMMatrixMultiply( view, projection ) );
	m_pCamera->setViewProjectionMatrix( Matrix4x4(viewProj._11, viewProj._12, viewProj._13, viewProj._14,
								viewProj._21, viewProj._22, viewProj._23, viewProj._24,
								viewProj._31, viewProj._32, viewProj._33, viewProj._34,
								viewProj._41, viewProj._42, viewProj._43, viewProj._44));
	m_pCamera->setProjectionMatrix( Matrix4x4(proj._11, proj._12, proj._13, proj._14,
								proj._21, proj._22, proj._23, proj._24,
								proj._31, proj._32, proj._33, proj._34,
								proj._41, proj._42, proj._43, proj._44) );
	m_pCamera->setViewMatrix( Matrix4x4(view._11, view._12, view._13, view._14,
						view._21, view._22, view._23, view._24,
						view._31, view._32, view._33, view._34,
						view._41, view._42, view._43, view._44) );
}

void SpotLight::cull( XNA::Frustum* frustum )
{
	m_bVisible = ( XNA::IntersectFrustumFrustum( frustum, &m_pCamera->getFrustum() ) > 0 );
}