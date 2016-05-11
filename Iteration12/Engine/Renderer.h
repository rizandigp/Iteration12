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
	virtual ~Renderer();

	virtual void SetRenderSystem( RenderSystem*	renderSystem );
	virtual void SetMesh( Mesh* mesh );

	virtual void AddLight( PointLight* light );
	virtual void AddLight( SpotLight* light );
	virtual void ClearLights();

	virtual void Cull( XNA::Frustum* frustum, Transform* transform );
	virtual void Cull( XNA::Sphere* sphere, Transform* transform );
	//virtual void Cull( XNA::OrientedBox* box, Transform* transform );
	virtual void CullLight( PointLight* light, Transform* transform );
	virtual void CullLight( SpotLight* light, Transform* transform );
	virtual void SetCulled( bool IsCulled );

	virtual void Render( Transform* transform ) = 0;
	virtual void Render( Transform* transform, Material* material ) = 0;
	virtual void RenderShadowmap( Transform* transform, Camera3D* shadowCamera ) = 0;
	virtual void RenderRSM( Transform* transform, Camera3D* shadowCamera, SpotLight* lightSource ) = 0;
	virtual void RenderOBB( Transform* transform ) = 0;

	RenderSystem* GetRenderSystem();

	void SetTime( float t );
	float GetTime();

protected:
	RenderSystem*	m_RenderSystem;
	Mesh* m_Mesh;
	std::vector<SubmeshRenderData>	m_SubmeshRenderData;
	float m_Time;
};