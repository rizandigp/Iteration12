#pragma once

#include "Prerequisites.h"
#include "DX11RenderCommand.h"


class FullscreenQuad
{
public:
	FullscreenQuad( RenderSystem *ptr );

	void SetShaderset( Shaderset* ptr );
	void SetTexture( std::string const &name, Texture2D *pTexture );
	void SetShaderParams( ShaderParamBlock params );
	void ClearTextures();
	void Render( bool renderAdditive = false );

protected:
	RenderSystem* m_pRenderSystem;
	GeometryChunk* m_pQuadGeom;
	Shaderset* m_pShaderset;
	ShaderParamBlock m_ShaderParams;
	ID3D11BlendState* m_pAdditiveBlendState;
	DX11RenderCommand_Draw m_DrawCommand;
	std::vector< std::pair< std::string, DX11Texture2D* >, tbb::scalable_allocator<std::pair< std::string, DX11Texture2D* >> > m_pTextures;
};