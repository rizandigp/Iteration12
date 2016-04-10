#pragma once

#include "Renderer.h"
#include "DX11RenderCommand.h"

// Object Renderer implementation in DX11
class DX11Renderer : public Renderer
{
public:
	DX11Renderer() 	{ m_pMesh = NULL; };

	void setRenderSystem( RenderSystem*	pRenderSystem );
	void setMesh( Mesh* pMesh );
	//void setCulled( UINT index, bool culled );

	void addLight( PointLight* pLight );
	void addLight( SpotLight* pLight );
	void clearLights();

	void cull( XNA::Frustum* frustum, Transform* pTransform );
	void cull( XNA::Sphere* sphere, Transform* pTransform );
	//void cull( XNA::OrientedBox* box, Transform* pTransform );
	void cullLight( PointLight* light, Transform* pTransform );
	void cullLight( SpotLight* light, Transform* pTransform );
	void setCulled( bool culled );

	void render( Transform* pTransform );
	void renderShadowmap( Transform* pTransform, Camera3D* pShadowCamera );
	void renderRSM( Transform* pTransform, Camera3D* pShadowCamera, SpotLight* pLightSource );
	void renderOBB( Transform* pTransform );

protected:
	std::vector< std::vector< DX11RenderCommand_Draw* > > m_pSubmeshRenderCommands;
	std::vector< DX11RenderCommand_Draw* >	m_pDebugRenderCommands;
	std::vector< DX11RenderCommand_Draw* >	m_pShadowmapRenderCommands;
};