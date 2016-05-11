#pragma once

#include "..\Renderer.h"
#include "DX11RenderCommand.h"

// Object Renderer implementation in DX11
class DX11Renderer : public Renderer
{
public:
	DX11Renderer();
	~DX11Renderer();

	void SetRenderSystem( RenderSystem*	renderSystem ) override;
	void SetMesh( Mesh* mesh ) override;

	void Render( Transform* transform ) override;
	void Render( Transform* transform, Material* material ) override;
	void RenderShadowmap( Transform* transform, Camera3D* shadowCamera ) override;
	void RenderRSM( Transform* transform, Camera3D* shadowCamera, SpotLight* lightSource ) override;
	void RenderOBB( Transform* transform ) override;

protected:
	Shaderset* m_ShadowmapShader;
	Shaderset* m_RSMShader;

	std::vector< std::vector< DX11RenderCommand_Draw* > > m_SubmeshRenderCommands;
	std::vector< DX11RenderCommand_Draw* >	m_DebugRenderCommands;
	std::vector< DX11RenderCommand_Draw* >	m_ShadowmapRenderCommands;
};