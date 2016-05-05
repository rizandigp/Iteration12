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

// Object renderer. Handles culling, lighting configuration, and submits
// API-specific RenderCommand's to the RenderSystem to be executed.
class Renderer
{
public:
	virtual void SetRenderSystem( RenderSystem*	renderSystem );
	virtual void SetMesh( Mesh* mesh );

	virtual void AddLight( PointLight* light );
	virtual void AddLight( SpotLight* light );
	virtual void ClearLights();

	virtual void Cull( XNA::Frustum* frustum, Transform* pTransform );
	virtual void Cull( XNA::Sphere* sphere, Transform* pTransform );
	//virtual void Cull( XNA::OrientedBox* box, Transform* pTransform );
	virtual void CullLight( PointLight* light, Transform* pTransform );
	virtual void CullLight( SpotLight* light, Transform* pTransform );
	virtual void SetCulled( bool IsCulled );

	virtual void Render( Transform* pTransform ) = 0;
	virtual void RenderShadowmap( Transform* pTransform, Camera3D* pShadowCamera ) = 0;
	virtual void RenderRSM( Transform* pTransform, Camera3D* pShadowCamera, SpotLight* pLightSource ) = 0;
	virtual void RenderOBB( Transform* pTransform ) = 0;

	RenderSystem* GetRenderSystem();

	void SetTime( float t );
	float GetTime();

protected:
	RenderSystem*	m_RenderSystem;
	Mesh* m_Mesh;
	std::vector<SubmeshRenderData>	m_SubmeshRenderData;
	float m_Time;
};