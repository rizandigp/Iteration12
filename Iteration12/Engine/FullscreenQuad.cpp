#pragma once

#include "FullscreenQuad.h"
#include "RenderSystem.h"
#include "Vector4.h"
#include "DX11/DX11RenderDispatcher.h"

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
	bufferLayout.AddElement( "POSITION", 0, R32G32B32_FLOAT );
	m_pQuadGeom = m_pRenderSystem->CreateGeometryChunk( (float*)&verts, bufferLayout.GetByteSize(), bufferLayout.GetByteSize() * 4, bufferLayout, (UINT*)&indices, 6);

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
	((DX11RenderDispatcher*)m_pRenderSystem->GetRenderDispatcher())->GetDevice()->CreateBlendState( &desc, &m_pAdditiveBlendState );
}

void FullscreenQuad::SetShaderset( Shaderset* ptr )
{
	m_pShaderset = ptr;
}

void FullscreenQuad::SetTexture( std::string const &name, Texture* pTexture )
{
	std::vector< std::pair< std::string, Texture* >, tbb::scalable_allocator<std::pair< std::string, Texture* >> >::iterator it = std::find_if( m_pTextures.begin(), m_pTextures.end(), FindFirst<std::string, Texture*>( name ) );
	if (it==m_pTextures.end())
		m_pTextures.push_back( std::pair< std::string, Texture* > (name, pTexture) );
	else
		it->second = pTexture;
}

void FullscreenQuad::ClearTextures()
{
	m_pTextures.clear();
}

void FullscreenQuad::SetShaderParams( ShaderParamBlock params )
{
	m_ShaderParams = params;
}

void FullscreenQuad::Render( bool renderAdditive )
{
	DX11RenderCommand_Draw* command = &m_DrawCommand;

	// We would still need to set the ShaderParams even if the shader contained no parameters, but no need to fill it with anything
	RenderSystemConfig config = m_pRenderSystem->GetConfig();
	ShaderParams params;
	params.initialize( m_pShaderset );
	params.assign( &m_ShaderParams );
	params.setParam( "ScreenDimensions", &XMFLOAT4(config.Width,config.Height,0.0f,0.0f) );
	command->SetShaderset( (DX11Shaderset*)m_pShaderset );
	command->SetShaderParams( &params );
	command->SetGeometryChunk( (DX11GeometryChunk*)m_pQuadGeom );
	command->ClearTextures();

	if (renderAdditive)
	{
		BlendState AdditiveBlendState;
		AdditiveBlendState.BlendEnable = true;
		AdditiveBlendState.SrcBlend = BLEND_ONE;
		AdditiveBlendState.DestBlend = BLEND_ONE;
		AdditiveBlendState.BlendOp = BLEND_OP_ADD;
		AdditiveBlendState.SrcBlendAlpha = BLEND_ONE;
		AdditiveBlendState.DestBlendAlpha = BLEND_ONE;
		AdditiveBlendState.BlendOpAlpha = BLEND_OP_ADD;
		command->SetBlendState(AdditiveBlendState);

		RenderState _RenderState;
		_RenderState.EnableDepthTest = false;
		_RenderState.CullingMode = CULL_NONE;
		command->SetRenderState(_RenderState);
	}
	else
	{
		BlendState DefaultBlendState;
		command->SetBlendState(DefaultBlendState);

		RenderState _RenderState;
		_RenderState.EnableDepthTest = false;
		_RenderState.CullingMode = CULL_NONE;
		command->SetRenderState(_RenderState);
	}

	for ( std::vector< std::pair< std::string, Texture* >, tbb::scalable_allocator<std::pair< std::string, Texture* >> >::iterator it = m_pTextures.begin(); it!=m_pTextures.end(); ++it )
	{
		command->SetTexture( it->first, it->second );
	}

	m_pRenderSystem->Submit( command ); 
	m_pRenderSystem->drawcalls++;
}