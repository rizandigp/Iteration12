#pragma once

#include "FullscreenQuad.h"
#include "RenderSystem.h"
#include "Vector4.h"
#include "DX11/DX11RenderDispatcher.h"

FullscreenQuad::FullscreenQuad( RenderSystem *renderSystem )
{
	m_RenderSystem = renderSystem;

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
	m_QuadMesh = m_RenderSystem->CreatePlaneMesh( Vector2(2.0f,2.0f), Vector2(1.0f,1.0f) );//->CreateGeometryChunk( (float*)&verts, bufferLayout.GetByteSize(), bufferLayout.GetByteSize() * 4, bufferLayout, (UINT*)&indices, 6);
}

void FullscreenQuad::SetShaderset( Shaderset* shaderset )
{
	m_Shaderset = shaderset;
}

void FullscreenQuad::SetTexture( const std::string& name, Texture* texture )
{
	std::vector< std::pair< std::string, Texture* >, tbb::scalable_allocator<std::pair< std::string, Texture* >> >::iterator it = std::find_if( m_Textures.begin(), m_Textures.end(), FindFirst<std::string, Texture*>( name ) );
	if (it==m_Textures.end())
		m_Textures.push_back( std::pair< std::string, Texture* > (name, texture) );
	else
		it->second = texture;
}

void FullscreenQuad::ClearTextures()
{
	m_Textures.clear();
}

void FullscreenQuad::SetShaderParams( ShaderParamBlock params )
{
	m_ShaderParams = params;
}

void FullscreenQuad::Render( bool renderAdditive )
{
	DX11RenderCommand_Draw* command = &m_DrawCommand;

	// We would still need to set the ShaderParams even if the shader contained no parameters, but no need to fill it with anything
	RenderSystemConfig config = m_RenderSystem->GetConfig();
	ShaderParams params;
	params.initialize( m_Shaderset );
	params.assign( &m_ShaderParams );
	params.setParam( "ScreenDimensions", &XMFLOAT4(config.Width,config.Height,0.0f,0.0f) );
	command->SetShaderset( (DX11Shaderset*)m_Shaderset );
	command->SetShaderParams( &params );
	command->SetGeometryChunk( (DX11GeometryChunk*)m_QuadMesh->GetSubmesh(0)->GetGeometryChunk() );
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

	for ( std::vector< std::pair< std::string, Texture* >, tbb::scalable_allocator<std::pair< std::string, Texture* >> >::iterator it = m_Textures.begin(); it!=m_Textures.end(); ++it )
	{
		command->SetTexture( it->first, it->second );
	}

	m_RenderSystem->Submit( command ); 
	m_RenderSystem->drawcalls++;
}