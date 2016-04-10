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

// Object renderer. Submits RenderCommands's to the RenderSystem to be executed.
class Renderer
{
public:
	virtual void setRenderSystem( RenderSystem*	pRenderSystem );
	virtual void setMesh( Mesh* pMesh );

	virtual void addLight( PointLight* pLight );
	virtual void addLight( SpotLight* pLight );
	virtual void clearLights();

	virtual void cull( XNA::Frustum* frustum, Transform* pTransform );
	virtual void cull( XNA::Sphere* sphere, Transform* pTransform );
	//virtual void cull( XNA::OrientedBox* box, Transform* pTransform );
	virtual void cullLight( PointLight* light, Transform* pTransform );
	virtual void cullLight( SpotLight* light, Transform* pTransform );
	virtual void setCulled( bool culled );

	virtual void render( Transform* pTransform );
	virtual void renderShadowmap( Transform* pTransform, Camera3D* pShadowCamera );
	virtual void renderRSM( Transform* pTransform, Camera3D* pShadowCamera, SpotLight* pLightSource )	{};
	virtual void renderOBB( Transform* pTransform );

	inline RenderSystem* getRenderSystem()	{ return m_pRenderSystem; };
	inline Mesh* getMesh()					{ return m_pMesh; };
	inline std::vector<PointLight*>* getAffectingPointLights(UINT submeshIndex)	{ return &m_PointLights[submeshIndex]; };
	inline std::vector<SpotLight*>* getAffectingSpotLights(UINT submeshIndex)	{ return &m_SpotLights[submeshIndex]; };

	inline void setTime( float t )	{ m_Time = t; };
	inline float getTime()			{ return m_Time; };

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