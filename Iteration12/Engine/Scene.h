#pragma once

#include "Prerequisites.h"

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <queue>
//#include "Entity.h"
#include "Camera3D.h"
#include "Light.h"
#include "xnaCollision.h"
#include "Utilities.h"
#include "Entity_Prop.h"
#include "DX11/DX11Material.h"
#include "PhysicsSystem.h"
#include "FullscreenQuad.h"

class Scene
{
public:
	Scene() : m_pRenderSystem(NULL), m_pPhysicsSystem(NULL), m_pActiveCamera(NULL), m_Time(NULL)	{};
	void Init( RenderSystem* pRendering, PhysicsSystem* pPhysics );
	void SetRenderSystem( RenderSystem* pRenderSystem );
	void SetPhysicsSystem( PhysicsSystem* ptr );
	void AddEntity( Entity* ent );
	void AddPointLight( PointLight* light )	{m_pPointLights.push_back(light);};
	void AddSpotLight( SpotLight* light )	{m_pSpotLights.push_back(light);};
	void SetCamera( Camera3D* camera );

	void SetDeltaTime( float value )		{ m_DeltaTime = value; };
	void Update( float deltaTime );
	void Render();
	void ParallelRender();
	void RenderDeferred();
	void ParallelRenderDeferred();
	RenderSystem* GetRenderSystem()		{ return m_pRenderSystem; };
	PhysicsSystem* GetPhysicsSystem()	{ return m_pPhysicsSystem; };

	void LoadFromFile( std::wstring filename );
	void LoadCustomObjects();	// Programmatically create objects

	double	t_Cull, t_scenerender, t_physics, t_ocean;

public:
	std::map<std::wstring, Mesh*> m_pMeshes;
	std::map<std::wstring, Entity_Prop*> m_pPrefabs;

protected:
	void LoadMeshes();
	void LoadPrefabs();
	hkpRigidBody* CreateBoxRigidBody( Vector3 halfExtents, float mass );

protected:
	RenderSystem* m_pRenderSystem;
	PhysicsSystem* m_pPhysicsSystem;
	Camera3D*	m_pActiveCamera;
	std::vector<Entity*>	m_pEntities;
	std::vector<PointLight*>	m_pPointLights;
	std::vector<SpotLight*>	m_pSpotLights;
	Texture2D*	m_pHDRRenderTarget;
	Texture2D*	m_pShadowmap[16];
	Texture2D*	m_pRSMNormal[16];
	Texture2D*	m_pRSMColor[16];
	Texture2D*	m_pSMEmpty;
	Texture2D*	m_pGBuffer[4];
	Texture2D*	m_pTestTexture;
	Texture2D*	m_pSSAOBuffer;
	Texture2D*	m_pHalfRes;
	FullscreenQuad* m_pFullscreenQuad;
	FullscreenQuad* m_pDepthDownsamplePass;
	FullscreenQuad* m_pSSAOPass;
	FullscreenQuad* m_pSSAOCombinePass;
	FullscreenQuad* m_pTonemappingPass;
	FullscreenQuad* m_pSunlight;
	FullscreenQuad* m_pGI;
	Shaderset*	m_pTonemappingShaderset[4];	

	float m_DeltaTime, m_Time;
	hkLoader m_HavokLoader;
	Entity_Water* m_pWater;
};

Color getSkyColor(Vector3 e);
