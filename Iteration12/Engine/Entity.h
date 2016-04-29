#pragma once

#include "Prerequisites.h"

#include <vector>
#include "Transform.h"
//#include "Renderer.h"
#include "DX11/DX11Renderer.h"
#include "RenderSystem.h"
#include "DX11/DX11Material.h"
//#include "PhysicsSystem.h"


// Base class for all objects in a scene
class Entity
{
public:
	Entity()	: m_bCastShadows(true)	{};
	virtual void SetRenderSystem( RenderSystem* ptr )		{ m_pRenderSystem = ptr; };
	RenderSystem* GetRenderSystem()							{ return m_pRenderSystem; };

	inline void SetName( std::wstring name )	{ m_Name = name; };
	inline std::wstring GetName()				{ return m_Name; };

	virtual void OnAddToScene( Scene* ptr )	{ m_pScene = ptr; };
	virtual void Update( float deltaTime ){};
	virtual void ClearLights(){};
	virtual void Cull( XNA::Frustum* frustum ){};
	virtual void SetCulled( bool Culled ){};
	//virtual void Cull( XNA::Sphere* sphere )=0;
	//virtual void Cull( XNA::OrientedBox* box )=0;
	virtual void CullLight( PointLight* light ){};
	virtual void CullLight( SpotLight* light ){};
	virtual void Render(){};
	virtual void RenderShadowmap( Camera3D* pShadowCamera ){};
	virtual void RenderRSM( Camera3D* pShadowCamera, SpotLight* pLightSource ){};
	virtual void RenderBoundingBox(){};

	inline void SetCastShadows( bool cast )					{ m_bCastShadows = cast; };
	inline bool GetCastShadows()							{ return m_bCastShadows; };

	Transform* Transformation()		{ return &m_Transform; };

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
	void SetRenderSystem( RenderSystem* ptr );

	void SetMesh( Mesh* ptr );

	void Update( float deltaTime );
	void ClearLights();
	void Cull( XNA::Frustum* frustum );
	void CullLight( PointLight* light );
	void CullLight( SpotLight* light );
	void SetCulled( bool Culled );
	void Render();
	void RenderShadowmap( Camera3D* pShadowCamera );
	void RenderBoundingBox();

protected:
	Mesh* m_pMesh;
	Renderer m_Renderer;
};

// Old controllable player class
class PlayerEntity : public Entity
{
public:
	PlayerEntity(RenderSystem* ptr);
	void OnAddToScene( Scene* ptr );
	void Update(float deltaTime);
	inline Camera3D* GetCamera()	{ return m_pCamera; };
	inline SpotLight* GetFlashlight()	{ return m_pSpotlight; };

protected:
	Camera3D* m_pCamera;
	SpotLight* m_pSpotlight;
	PointLight* m_pPointLight;
	bool m_bSpotlightOn;
	bool m_bMouseDragged;
	POINT m_ptLastMousePos;
	XMFLOAT3 m_DirEuler;
};