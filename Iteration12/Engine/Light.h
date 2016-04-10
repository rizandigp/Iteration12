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

	void setColor( Vector3 color )			{ m_Color = color; };
	void setRadius( float radius )			{ m_fRadius = radius; };
	void setIntensity( float intensity )	{ m_fIntensity = intensity; };
	void enable()							{ m_bEnabled = true; };
	void disable()							{ m_bEnabled = false; };
	void setEnabled( bool enabled )			{ m_bEnabled = enabled; };

	void cull( XNA::Frustum* frustum );
	void update();

	inline Vector3 getColor()					{ return m_Color; };
	inline float getRadius()					{ return m_fRadius; };
	inline float getIntensity()					{ return m_fIntensity; };
	inline bool isVisible()						{ return m_bVisible; };
	inline bool isEnabled()						{ return m_bEnabled; };
	inline Transform* transform()				{ return &m_Transform; };

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

	void setCookie( Texture2D* ptr )		{ m_pCookie = ptr; };
	void setColor( Vector3 color )			{ m_Color = color; };
	void setRadius( float radius )			{ m_Radius = radius; };
	void setIntensity( float intensity )	{ m_Intensity = intensity; };
	void setCone( Vector2 radians )			{ m_Cone = radians; };
	void setOrientation( XMFLOAT4 orient )	{ m_Transform.setOrientation(orient); };
	void enable()							{ m_bEnabled = true; };
	void disable()							{ m_bEnabled = false; };
	void setEnabled( bool on )				{ m_bEnabled = on; };
	void setCastShadow( bool on )			{ m_bShadowCasting = on; };
	void setRSMEnabled( bool on )			{ m_bRSMEnabled = on; };

	void pointTo( Vector3 target, float roll );
	void cull( XNA::Frustum* frustum );
	void update();

	inline Texture2D* getCookie()					{ return m_pCookie; };
	inline Vector3 getColor()						{ return m_Color; };
	inline float getRadius()						{ return m_Radius; };
	inline float getIntensity()						{ return m_Intensity; };
	inline Vector2 getCone()						{ return m_Cone; };
	inline Quaternion getOrientation()				{ return m_Transform.getOrientation(); };
	inline Transform* transform()					{ return &m_Transform; };
	inline Camera3D* getProjectionCamera()			{ return m_pCamera; };
	inline XNA::Frustum getFrustum()				{ return m_pCamera->getFrustum(); };
	inline bool isEnabled()							{ return m_bEnabled; };
	inline bool isVisible()							{ return m_bVisible; };
	inline bool isCastingShadow()					{ return m_bShadowCasting; };
	inline bool isRSMEnabled()						{ return m_bRSMEnabled; };

	inline void setShadowmap( Texture2D* ptr )		{ m_pShadowmap = ptr; };
	inline void setRSMNormal( Texture2D* ptr )		{ m_pRSMNormal = ptr; };
	inline void setRSMColor( Texture2D* ptr )		{ m_pRSMColor = ptr; };
	inline Texture2D* getShadowmap()				{ return m_pShadowmap; };
	inline Texture2D* getRSMNormal()				{ return m_pRSMNormal; };
	inline Texture2D* getRSMColor()					{ return m_pRSMColor; };

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