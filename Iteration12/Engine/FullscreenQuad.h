#pragma once

#include "Prerequisites.h"
#include "DX11RenderCommand.h"


class FullscreenQuad
{
public:
	FullscreenQuad( RenderSystem *ptr );

	void setShaderset( Shaderset* ptr );
	void setTexture( std::string const &name, Texture2D *pTexture );
	void setShaderParams( ShaderParamBlock params );
	void clearTextures();
	void render( bool renderAdditive );

protected:
	RenderSystem* m_pRenderSystem;
	GeometryChunk* m_pQuadGeom;
	Shaderset* m_pShaderset;
	ShaderParamBlock m_ShaderParams;
	ID3D11BlendState* m_pAdditiveBlendState;
	DX11RenderCommand_Draw m_DrawCommand;
	std::vector< std::pair< std::string, DX11Texture2D* >, tbb::scalable_allocator<std::pair< std::string, DX11Texture2D* >> > m_pTextures;
};