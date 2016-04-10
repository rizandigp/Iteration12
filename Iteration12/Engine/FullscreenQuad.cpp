#pragma once

#include "FullscreenQuad.h"
#include "RenderSystem.h"
#include "Vector4.h"

FullscreenQuad::FullscreenQuad( RenderSystem *ptr )
{
	m_pRenderSystem = ptr;

	// Define vertex format
	struct Vertex
	{
		Vector3 Pos;
	};

	// Define vertex buffer
	Vertex verts[] =
	{
		{ Vector3(-1.0f,-1.0f,0.0f) },
		{ Vector3(1.0f,-1.0f,0.0f) },
		{ Vector3(1.0f,1.0f,0.0f) },
		{ Vector3(-1.0f,1.0f,0.0f) },
	};

	// Define index buffer
	UINT indices[] =
	{
		3,1,0,
		2,1,3,
	};

	// Create geometry
	BufferLayout bufferLayout;
	bufferLayout.addElement( "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT );
	m_pQuadGeom = m_pRenderSystem->createGeometryChunk( (float*)&verts, bufferLayout.getByteSize(), bufferLayout.getByteSize() * 4, bufferLayout, (UINT*)&indices, 6);

	// Additive blend state
	D3D11_BLEND_DESC desc;
	desc.AlphaToCoverageEnable = false;
	desc.IndependentBlendEnable = false;
	desc.RenderTarget[0].BlendEnable = true;
	desc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
	desc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
	desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
	desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	((DX11RenderDispatcher*)m_pRenderSystem->getRenderDispatcher())->getDevice()->CreateBlendState( &desc, &m_pAdditiveBlendState );
}

void FullscreenQuad::setShaderset( Shaderset* ptr )
{
	m_pShaderset = ptr;
}

void FullscreenQuad::setTexture( std::string const &name, Texture2D *pTexture )
{
	std::vector< std::pair< std::string, DX11Texture2D* >, tbb::scalable_allocator<std::pair< std::string, DX11Texture2D* >> >::iterator it = std::find_if( m_pTextures.begin(), m_pTextures.end(), FindFirst<std::string, DX11Texture2D*>( name ) );
	if (it==m_pTextures.end())
		m_pTextures.push_back( std::pair< std::string, DX11Texture2D* > (name,(DX11Texture2D*)pTexture) );
	else
		it->second = (DX11Texture2D*)pTexture;
}

void FullscreenQuad::clearTextures()
{
	m_pTextures.clear();
}

void FullscreenQuad::setShaderParams( ShaderParamBlock params )
{
	m_ShaderParams = params;
}

void FullscreenQuad::render( bool renderAdditive )
{
	DX11RenderCommand_Draw* command = &m_DrawCommand;

	// We would still need to set the ShaderParams even if the shader contained no parameters, but no need to fill it with anything
	RenderSystemConfig config = m_pRenderSystem->getConfig();
	ShaderParams params;
	params.initialize( m_pShaderset );
	params.assign( &m_ShaderParams );
	params.setParam( "ScreenDimensions", &XMFLOAT4(config.Width,config.Height,0.0f,0.0f) );
	command->setShaderset( (D3D11Shaderset*)m_pShaderset );
	command->setShaderParams( &params );
	command->setGeometryChunk( (D3D11GeometryChunk*)m_pQuadGeom );
	command->clearTextures();

	if (renderAdditive)
	{
		command->setBlendState( m_pAdditiveBlendState );
	}

	for ( std::vector< std::pair< std::string, DX11Texture2D* >, tbb::scalable_allocator<std::pair< std::string, DX11Texture2D* >> >::iterator it = m_pTextures.begin(); it!=m_pTextures.end(); ++it )
	{
		command->setTexture( it->first, it->second );
	}

	m_pRenderSystem->submit( command ); 
	m_pRenderSystem->drawcalls++;
}