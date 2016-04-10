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
#include "DX11Material.h"
#include "PhysicsSystem.h"
#include "FullscreenQuad.h"

class Scene
{
public:
	Scene() : m_pRenderSystem(NULL), m_pPhysicsSystem(NULL), m_pActiveCamera(NULL), m_Time(NULL)	{};
	void init( RenderSystem* pRendering, PhysicsSystem* pPhysics );
	void setRenderSystem( RenderSystem* pRenderSystem );
	void setPhysicsSystem( PhysicsSystem* ptr );
	void addEntity( Entity* ent );
	void addPointLight( PointLight* light )	{m_pPointLights.push_back(light);};
	void addSpotLight( SpotLight* light )	{m_pSpotLights.push_back(light);};
	void setCamera( Camera3D* camera );

	void setDeltaTime( float value )		{ m_DeltaTime = value; };
	void update( float deltaTime );
	void render();
	void parallelRender();
	void renderDeferred();
	void parallelRenderDeferred();
	RenderSystem* getRenderSystem()		{ return m_pRenderSystem; };
	PhysicsSystem* getPhysicsSystem()	{ return m_pPhysicsSystem; };

	void loadFromFile( std::wstring filename );
	void loadCustomObjects();	// Programmatically create objects

	double	t_cull, t_scenerender, t_physics, t_ocean;

public:
	std::map<std::wstring, Mesh*> m_pMeshes;
	std::map<std::wstring, Entity_Prop*> m_pPrefabs;

protected:
	void loadMeshes();
	void loadPrefabs();
	hkpRigidBody* createBoxRigidBody( Vector3 halfExtents, float mass );

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
