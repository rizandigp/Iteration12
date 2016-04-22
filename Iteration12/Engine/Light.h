#pragma once

#include "Prerequisites.h"

#include "Transform.h"
#include "xnaCollision.h"
#include "Math.h"
#include "Texture2D.h"
#include "Matrix4x4.h"
#include "Vector3.h"
#include "Vector2.h"


class PointLight
{
public:
	PointLight();
	PointLight( Vector3 position, Vector3 color, float radius, float intensity );

	void SetColor( Vector3 color )			{ m_Color = color; };
	void SetRadius( float radius )			{ m_fRadius = radius; };
	void SetIntensity( float intensity )	{ m_fIntensity = intensity; };
	void Enable()							{ m_bEnabled = true; };
	void Disable()							{ m_bEnabled = false; };
	void SetEnabled( bool enabled )			{ m_bEnabled = enabled; };

	void Cull( XNA::Frustum* frustum );
	void Update();

	inline Vector3 getColor()					{ return m_Color; };
	inline float GetRadius()					{ return m_fRadius; };
	inline float GetIntensity()					{ return m_fIntensity; };
	inline bool IsVisible()						{ return m_bVisible; };
	inline bool IsEnabled()						{ return m_bEnabled; };
	inline Transform* Transformation()			{ return &m_Transform; };

protected:
	Transform m_Transform;
	Vector3 m_Color;
	float m_fRadius, m_fIntensity;
	bool m_bEnabled;
	bool m_bVisible;
	bool m_bShadowCasting;
};

class SpotLight
{
public:
	SpotLight();

	void SetCookie( Texture2D* ptr )		{ m_pCookie = ptr; };
	void SetColor( Vector3 color )			{ m_Color = color; };
	void SetRadius( float radius )			{ m_Radius = radius; };
	void SetIntensity( float intensity )	{ m_Intensity = intensity; };
	void SetCone( Vector2 radians )			{ m_Cone = radians; };
	void SetOrientation( XMFLOAT4 orient )	{ m_Transform.SetOrientation(orient); };
	void Enable()							{ m_bEnabled = true; };
	void Disable()							{ m_bEnabled = false; };
	void SetEnabled( bool on )				{ m_bEnabled = on; };
	void SetCastShadow( bool on )			{ m_bShadowCasting = on; };
	void SetRSMEnabled( bool on )			{ m_bRSMEnabled = on; };

	void PointTo( Vector3 target, float roll );
	void Cull( XNA::Frustum* frustum );
	void Update();

	inline Texture2D* GetCookie()					{ return m_pCookie; };
	inline Vector3 GetColor()						{ return m_Color; };
	inline float GetRadius()						{ return m_Radius; };
	inline float GetIntensity()						{ return m_Intensity; };
	inline Vector2 GetCone()						{ return m_Cone; };
	inline Quaternion GetOrientation()				{ return m_Transform.GetOrientation(); };
	inline Transform* Transformation()				{ return &m_Transform; };
	inline Camera3D* GetProjectionCamera()			{ return m_pCamera; };
	inline XNA::Frustum GetFrustum()				{ return m_pCamera->GetFrustum(); };
	inline bool IsEnabled()							{ return m_bEnabled; };
	inline bool IsVisible()							{ return m_bVisible; };
	inline bool IsCastingShadow()					{ return m_bShadowCasting; };
	inline bool IsRSMEnabled()						{ return m_bRSMEnabled; };

	inline void SetShadowmap( Texture2D* ptr )		{ m_pShadowmap = ptr; };
	inline void SetRSMNormal( Texture2D* ptr )		{ m_pRSMNormal = ptr; };
	inline void SetRSMColor( Texture2D* ptr )		{ m_pRSMColor = ptr; };
	inline Texture2D* GetShadowmap()				{ return m_pShadowmap; };
	inline Texture2D* GetRSMNormal()				{ return m_pRSMNormal; };
	inline Texture2D* GetRSMColor()					{ return m_pRSMColor; };

protected:
	Transform m_Transform;
	Vector3 m_Color;
	Vector2 m_Cone;
	Texture2D* m_pCookie;
	Texture2D* m_pShadowmap;
	Texture2D* m_pRSMNormal;
	Texture2D* m_pRSMColor;
	float m_Radius;
	float m_Intensity;
	bool m_bEnabled;
	bool m_bVisible;
	bool m_bShadowCasting;
	bool m_bRSMEnabled;
	Camera3D* m_pCamera;
};