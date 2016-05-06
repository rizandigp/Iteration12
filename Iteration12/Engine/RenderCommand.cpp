#pragma once

#include "RenderCommand.h"
#include "RenderSystem.h"
#include "GeometryChunk.h"
#include "Shaderset.h"
#include "Texture2D.h"
#include "Texture3D.h"
#include "TextureCube.h"
#include "ShaderParamBlock.h"
#include "ShaderParams.h"
#include "Math.h"
#include "Utilities.h"
#include "ConstantBufferData.h"
#include "DX11/DX11RenderDispatcher.h"


RenderCommand::~RenderCommand() 
{
}

/*
void* RenderCommand::operator new(size_t size)
{
	void *p = malloc(sizeof(RenderCommandBlock));
	return p; 
}

void RenderCommand::operator delete(void* p)
{
	free(p); 
}

void* RenderCommand::operator new( size_t, void* ptr )
{
   return ptr;
}*/

void GenericRenderCommand::Execute( RenderDispatcher *pDispatcher )
{
	if (!m_pRenderTargetTexture)
	{
		m_pShaderset->Bind( pDispatcher );
		pDispatcher->BindShaderParams( &m_ShaderParamBlock );

		for ( std::vector< std::pair< std::string, Texture2D* > >::iterator it = GetTextures()->begin(); it!=GetTextures()->end(); ++it )
		{
			if ( it->second )
				it->second->Bind( it->first, pDispatcher );
		}

		m_pGeometryChunk->Bind( pDispatcher );
	}
	else
	{
			static_cast<DX11RenderDispatcher*>(pDispatcher)->SetRenderTarget( GetRenderTargetTexture() );
			D3D11_VIEWPORT vp;
			vp.Width = GetRenderTargetTexture()->GetWidth();
			vp.Height = GetRenderTargetTexture()->GetHeight();
			vp.MinDepth = 0.0f;
			vp.MaxDepth = 1.0f;
			vp.TopLeftX = 0;
			vp.TopLeftY = 0;
			static_cast<DX11RenderDispatcher*>(pDispatcher)->GetImmediateContext()->RSSetViewports( 1, &vp );
	}
}