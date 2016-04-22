#pragma once

#include "Renderer.h"
#include "DX11RenderCommand.h"

// Object Renderer implementation in DX11
class DX11Renderer : public Renderer
{
public:
	DX11Renderer() 	{ m_pMesh = NULL; };

	void SetRenderSystem( RenderSystem*	pRenderSystem );
	void SetMesh( Mesh* pMesh );
	//void SetCulled( UINT index, bool culled );

	void AddLight( PointLight* pLight );
	void AddLight( SpotLight* pLight );
	void ClearLights();

	void Cull( XNA::Frustum* frustum, Transform* pTransform );
	void Cull( XNA::Sphere* sphere, Transform* pTransform );
	//void cull( XNA::OrientedBox* box, Transform* pTransform );
	void CullLight( PointLight* light, Transform* pTransform );
	void CullLight( SpotLight* light, Transform* pTransform );
	void SetCulled( bool culled );

	void Render( Transform* pTransform );
	void RenderShadowmap( Transform* pTransform, Camera3D* pShadowCamera );
	void RenderRSM( Transform* pTransform, Camera3D* pShadowCamera, SpotLight* pLightSource );
	void RenderOBB( Transform* pTransform );

protected:
	std::vector< std::vector< DX11RenderCommand_Draw* > > m_pSubmeshRenderCommands;
	std::vector< DX11RenderCommand_Draw* >	m_pDebugRenderCommands;
	std::vector< DX11RenderCommand_Draw* >	m_pShadowmapRenderCommands;
};