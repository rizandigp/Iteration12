#pragma once

#include "DX11RenderCommand.h"
#include "RenderSystem.h"
#include "GeometryChunk.h"
#include "Shaderset.h"
#include "Texture2D.h"
#include "ShaderParamBlock.h"
#include "ShaderParams.h"
#include "ConstantBufferData.h"

void DX11RenderCommand_Draw::Execute( RenderDispatcher *pDispatcher )
{	
	DX11RenderDispatcher* pd3d11Dispatcher = dynamic_cast<DX11RenderDispatcher*>(pDispatcher);
	
	// Bind shaders, constant buffers, and vertex & index buffers
	m_pShaderset->Bind( pDispatcher );
	Timer timer;
	pDispatcher->BindShaderParams( &m_ShaderParams );
	pd3d11Dispatcher->t_bindparams += timer.GetMiliseconds();
	m_pGeometryChunk->Bind( pDispatcher );

	// Bind textures
	timer.Start();
	for ( std::vector< std::pair< std::string, DX11Texture2D* >, tbb::scalable_allocator<std::pair< std::string, DX11Texture2D* >> >::iterator it = GetTextures()->begin(); it!=GetTextures()->end(); ++it )
	{
		if ( it->second )
		it->second->Bind( it->first, pDispatcher );
	}
	pd3d11Dispatcher->t_bindtextures += timer.GetMiliseconds();

	// Set DX11 render states
	timer.Start();
	pd3d11Dispatcher->GetImmediateContext()->RSSetState( m_pRasterizerState );
	pd3d11Dispatcher->GetImmediateContext()->OMSetDepthStencilState( m_pDepthStencilState, 0 );
	pd3d11Dispatcher->GetImmediateContext()->OMSetBlendState( m_pBlendState, NULL, 0xffffffff );

	// Draw call
	pd3d11Dispatcher->GetImmediateContext()->DrawIndexed( m_pGeometryChunk->GetNumberOfVerts(), 0, 0 );
	pd3d11Dispatcher->t_drawcalls += timer.GetMiliseconds();

	// Unbind textures
	for ( std::vector< std::pair< std::string, DX11Texture2D* >, tbb::scalable_allocator<std::pair< std::string, DX11Texture2D* >> >::iterator it = GetTextures()->begin(); it!=GetTextures()->end(); ++it )
	{
		if ( it->second )
		it->second->Unbind( it->first, pDispatcher );
	}
	
	pDispatcher->numVerts += m_pGeometryChunk->GetNumberOfVerts();
	pDispatcher->drawcalls++;
	//pDispatcher->t_work += timer.GetMiliseconds();
}

void DX11RenderCommand_Draw::SetTexture( std::string const &name, DX11Texture2D *pTexture )
{
	std::vector< std::pair< std::string, DX11Texture2D* >, tbb::scalable_allocator<std::pair< std::string, DX11Texture2D* >> >::iterator it = std::find_if( m_pTextures.begin(), m_pTextures.end(), FindFirst<std::string, DX11Texture2D*>( name ) );
	if (it==m_pTextures.end())
		m_pTextures.push_back( std::pair< std::string, DX11Texture2D* > (name,pTexture) );
	else
		it->second = pTexture;
}

void DX11RenderCommand_Draw::Clone( RenderCommand* ptr )
{
	new (ptr) DX11RenderCommand_Draw(*this);
}


void DX11RenderCommand_Map::Execute( RenderDispatcher *pDispatcher )
{	
	DX11RenderDispatcher* pd3d11Dispatcher = dynamic_cast<DX11RenderDispatcher*>(pDispatcher);

	D3D11_MAPPED_SUBRESOURCE resource;
    pd3d11Dispatcher->GetImmediateContext()->
		Map(m_pResource, 0, D3D11_MAP_WRITE_DISCARD, 0, &resource);
    memcpy(resource.pData, m_pData, m_DataSize);
    pd3d11Dispatcher->GetImmediateContext()->Unmap(m_pResource, 0);
}

void DX11RenderCommand_Map::Clone( RenderCommand* ptr )
{
	new (ptr) DX11RenderCommand_Map(*this);
}

void DX11RenderCommand_CopySubresourceRegion::Execute( RenderDispatcher *pDispatcher )
{	
	DX11RenderDispatcher* pd3d11Dispatcher = dynamic_cast<DX11RenderDispatcher*>(pDispatcher);

	pd3d11Dispatcher->GetImmediateContext()->CopySubresourceRegion( 
		m_pDestination, m_DestinationSubresourceIndex, 0, 0, 0,
		m_pSource, m_SourceSubresourceIndex, NULL );
}

void DX11RenderCommand_CopySubresourceRegion::Clone( RenderCommand* ptr )
{
	new (ptr) DX11RenderCommand_CopySubresourceRegion(*this);
}