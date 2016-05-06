#pragma once

#include "Prerequisites.h"
#include "DX11/DX11RenderCommand.h"


class FullscreenQuad
{
public:
	FullscreenQuad( RenderSystem* renderSystem );

	void SetShaderset( Shaderset* shaderset );
	void SetTexture( const std::string& name, Texture* texture );
	void SetShaderParams( ShaderParamBlock params );
	void ClearTextures();
	void Render( bool renderAdditive = false );

protected:
	RenderSystem* m_RenderSystem;
	Mesh* m_QuadMesh;
	Shaderset* m_Shaderset;
	ShaderParamBlock m_ShaderParams;
	DX11RenderCommand_Draw m_DrawCommand;
	std::vector< std::pair< std::string, Texture* >, tbb::scalable_allocator<std::pair< std::string, Texture* >> > m_Textures;
};