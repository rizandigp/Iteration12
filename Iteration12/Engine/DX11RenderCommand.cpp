#pragma once

#include "DX11RenderCommand.h"
#include "RenderSystem.h"
#include "GeometryChunk.h"
#include "Shaderset.h"
#include "Texture2D.h"
#include "ShaderParamBlock.h"
#include "ShaderParams.h"
#include "ConstantBufferData.h"

void DX11RenderCommand_Draw::execute( RenderDispatcher *pDispatcher )
{	
	DX11RenderDispatcher* pd3d11Dispatcher = dynamic_cast<DX11RenderDispatcher*>(pDispatcher);
	
	// Bind shaders, constant buffers, and vertex & index buffers
	m_pShaderset->bind( pDispatcher );
	Timer timer;
	pDispatcher->bindShaderParams( &m_ShaderParams );
	pd3d11Dispatcher->t_bindparams += timer.getMiliseconds();
	m_pGeometryChunk->bind( pDispatcher );

	// Bind textures
	timer.start();
	for ( std::vector< std::pair< std::string, DX11Texture2D* >, tbb::scalable_allocator<std::pair< std::string, DX11Texture2D* >> >::iterator it = getTextures()->begin(); it!=getTextures()->end(); ++it )
	{
		if ( it->second )
		it->second->bind( it->first, pDispatcher );
	}
	pd3d11Dispatcher->t_bindtextures += timer.getMiliseconds();

	// Set DX11 render states
	timer.start();
	pd3d11Dispatcher->getImmediateContext()->RSSetState( m_pRasterizerState );
	pd3d11Dispatcher->getImmediateContext()->OMSetDepthStencilState( m_pDepthStencilState, 0 );
	pd3d11Dispatcher->getImmediateContext()->OMSetBlendState( m_pBlendState, NULL, 0xffffffff );

	// Draw call
	pd3d11Dispatcher->getImmediateContext()->DrawIndexed( m_pGeometryChunk->getNumberOfVerts(), 0, 0 );
	pd3d11Dispatcher->t_drawcalls += timer.getMiliseconds();

	// Unbind textures
	for ( std::vector< std::pair< std::string, DX11Texture2D* >, tbb::scalable_allocator<std::pair< std::string, DX11Texture2D* >> >::iterator it = getTextures()->begin(); it!=getTextures()->end(); ++it )
	{
		if ( it->second )
		it->second->unbind( it->first, pDispatcher );
	}
	
	pDispatcher->numVerts += m_pGeometryChunk->getNumberOfVerts();
	pDispatcher->drawcalls++;
	//pDispatcher->t_work += timer.getMiliseconds();
}

void DX11RenderCommand_Draw::setTexture( std::string const &name, DX11Texture2D *pTexture )
{
	std::vector< std::pair< std::string, DX11Texture2D* >, tbb::scalable_allocator<std::pair< std::string, DX11Texture2D* >> >::iterator it = std::find_if( m_pTextures.begin(), m_pTextures.end(), FindFirst<std::string, DX11Texture2D*>( name ) );
	if (it==m_pTextures.end())
		m_pTextures.push_back( std::pair< std::string, DX11Texture2D* > (name,pTexture) );
	else
		it->second = pTexture;
}

void DX11RenderCommand_Draw::clone( RenderCommand* ptr )
{
	new (ptr) DX11RenderCommand_Draw(*this);
}


void DX11RenderCommand_Map::execute( RenderDispatcher *pDispatcher )
{	
	DX11RenderDispatcher* pd3d11Dispatcher = dynamic_cast<DX11RenderDispatcher*>(pDispatcher);

	D3D11_MAPPED_SUBRESOURCE resource;
    pd3d11Dispatcher->getImmediateContext()->
		Map(m_pResource, 0, D3D11_MAP_WRITE_DISCARD, 0, &resource);
    memcpy(resource.pData, m_pData, m_DataSize);
    pd3d11Dispatcher->getImmediateContext()->Unmap(m_pResource, 0);
}

void DX11RenderCommand_Map::clone( RenderCommand* ptr )
{
	new (ptr) DX11RenderCommand_Map(*this);
}