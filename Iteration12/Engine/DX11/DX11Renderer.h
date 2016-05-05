#pragma once

#include "..\Renderer.h"
#include "DX11RenderCommand.h"

// Object Renderer implementation in DX11
class DX11Renderer : public Renderer
{
public:
	DX11Renderer();
	~DX11Renderer();

	void SetRenderSystem( RenderSystem*	pRenderSystem );
	void SetMesh( Mesh* pMesh );

	void Render( Transform* pTransform );
	void RenderShadowmap( Transform* pTransform, Camera3D* pShadowCamera );
	void RenderRSM( Transform* pTransform, Camera3D* pShadowCamera, SpotLight* pLightSource );
	void RenderOBB( Transform* pTransform );

protected:
	Shaderset* m_pShadowmapShader;
	Shaderset* m_pRSMShader;

	std::vector< std::vector< DX11RenderCommand_Draw* > > m_pSubmeshRenderCommands;
	std::vector< DX11RenderCommand_Draw* >	m_pDebugRenderCommands;
	std::vector< DX11RenderCommand_Draw* >	m_pShadowmapRenderCommands;
};