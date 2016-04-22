#pragma once

#include "Prerequisites.h"

#include "RenderSystem.h"
#include "Mesh.h"
#include "Light.h"

// Render data (visibility, affecting lights) per submesh
struct SubmeshRenderData
{
	bool bVisible;
	// Affecting lights
	std::vector<PointLight*> AffectingPointLights;
	std::vector<SpotLight*>	AffectingSpotLights;
};

// Object renderer. Handles culling, lighting configuration, etc and submits RenderCommands's to the RenderSystem to be executed.
class Renderer
{
public:
	virtual void SetRenderSystem( RenderSystem*	pRenderSystem );
	virtual void SetMesh( Mesh* pMesh );

	virtual void AddLight( PointLight* pLight );
	virtual void AddLight( SpotLight* pLight );
	virtual void ClearLights();

	virtual void Cull( XNA::Frustum* frustum, Transform* pTransform );
	virtual void Cull( XNA::Sphere* sphere, Transform* pTransform );
	//virtual void Cull( XNA::OrientedBox* box, Transform* pTransform );
	virtual void CullLight( PointLight* light, Transform* pTransform );
	virtual void CullLight( SpotLight* light, Transform* pTransform );
	virtual void SetCulled( bool Culled );

	virtual void Render( Transform* pTransform );
	virtual void RenderShadowmap( Transform* pTransform, Camera3D* pShadowCamera );
	virtual void RenderRSM( Transform* pTransform, Camera3D* pShadowCamera, SpotLight* pLightSource )	{};
	virtual void RenderOBB( Transform* pTransform );

	inline RenderSystem* GetRenderSystem()	{ return m_pRenderSystem; };
	inline Mesh* GetMesh()					{ return m_pMesh; };
	inline std::vector<PointLight*>* GetAffectingPointLights(UINT submeshIndex)	{ return &m_PointLights[submeshIndex]; };
	inline std::vector<SpotLight*>* GetAffectingSpotLights(UINT submeshIndex)	{ return &m_SpotLights[submeshIndex]; };

	inline void SetTime( float t )	{ m_Time = t; };
	inline float GetTime()			{ return m_Time; };

protected:
	RenderSystem*	m_pRenderSystem;
	Mesh* m_pMesh;
	std::vector<SubmeshRenderData>	m_SubmeshRenderData;
	Shaderset* m_pShadowmapShader;
	Shaderset* m_pRSMShader;
	float m_Time;

private:
	std::vector< bool > m_bVisible;
	std::vector< std::vector< D3D11RenderCommand_Draw* > > m_pSubmeshRenderCommands;
	std::vector< D3D11RenderCommand_Draw* >	m_pDebugRenderCommands;
	std::vector< D3D11RenderCommand_Draw* > m_pShadowmapRenderCommands;
	// Affecting lights for every submesh
	std::vector< std::vector<PointLight*> > m_PointLights;
	std::vector< std::vector<SpotLight*> > m_SpotLights;
};