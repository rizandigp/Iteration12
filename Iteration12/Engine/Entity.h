#pragma once

#include "Prerequisites.h"

#include <vector>
#include "Transform.h"
//#include "Renderer.h"
#include "DX11Renderer.h"
#include "RenderSystem.h"
#include "DX11Material.h"
//#include "PhysicsSystem.h"


// Base class for all objects in a scene
class Entity
{
public:
	Entity()	: m_bCastShadows(true)	{};
	virtual void setRenderSystem( RenderSystem* ptr )		{ m_pRenderSystem = ptr; };
	RenderSystem* getRenderSystem()							{ return m_pRenderSystem; };

	inline void setName( std::wstring name )	{ m_Name = name; };
	inline std::wstring getName()				{ return m_Name; };

	virtual void onAddToScene( Scene* ptr )	{ m_pScene = ptr; };
	virtual void update( float deltaTime ){};
	virtual void clearLights(){};
	virtual void cull( XNA::Frustum* frustum ){};
	virtual void setCulled( bool culled ){};
	//virtual void cull( XNA::Sphere* sphere )=0;
	//virtual void cull( XNA::OrientedBox* box )=0;
	virtual void cullLight( PointLight* light ){};
	virtual void cullLight( SpotLight* light ){};
	virtual void render(){};
	virtual void renderShadowmap( Camera3D* pShadowCamera ){};
	virtual void renderRSM( Camera3D* pShadowCamera, SpotLight* pLightSource ){};
	virtual void renderBoundingBox(){};

	inline void setCastShadows( bool cast )					{ m_bCastShadows = cast; };
	inline bool getCastShadows()							{ return m_bCastShadows; };

	Transform* transform()		{ return &m_Transform; };

protected:
	std::wstring m_Name;
	Scene* m_pScene;
	RenderSystem* m_pRenderSystem;
	//PhysicsSystem* m_pPhysicsSystem;
	Transform m_Transform;
	bool m_bCastShadows;
};

// Placeholder class with the older, slower renderer
class MyEntity : public Entity
{
public:
	MyEntity();
	MyEntity( const MyEntity& other );
	void setRenderSystem( RenderSystem* ptr );

	void setMesh( Mesh* ptr );

	void update( float deltaTime );
	void clearLights();
	void cull( XNA::Frustum* frustum );
	void cullLight( PointLight* light );
	void cullLight( SpotLight* light );
	void setCulled( bool culled );
	void render();
	void renderShadowmap( Camera3D* pShadowCamera );
	void renderBoundingBox();

protected:
	Mesh* m_pMesh;
	Renderer m_Renderer;
};

// Old controllable player class
class PlayerEntity : public Entity
{
public:
	PlayerEntity(RenderSystem* ptr);
	void onAddToScene( Scene* ptr );
	void update(float deltaTime);
	inline Camera3D* getCamera()	{ return m_pCamera; };
	inline SpotLight* getFlashlight()	{ return m_pSpotlight; };

protected:
	Camera3D* m_pCamera;
	SpotLight* m_pSpotlight;
	PointLight* m_pPointLight;
	bool m_bSpotlightOn;
	bool m_bMouseDragged;
	POINT m_ptLastMousePos;
	XMFLOAT3 m_DirEuler;
};