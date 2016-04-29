#pragma once

#include "DX11RenderCommand.h"
#include "..\RenderSystem.h"
#include "..\GeometryChunk.h"
#include "..\Shaderset.h"
#include "..\Texture2D.h"
#include "..\ShaderParamBlock.h"
#include "..\ShaderParams.h"
#include "..\ConstantBufferData.h"
#include "DX11Texture2D.h"
#include "DX11Texture3D.h"
#include "DX11TextureCube.h"
#include "DX11RenderDispatcher.h"
#include "DX11Shaderset.h"
#include "DX11GeometryChunk.h"


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
	for ( std::vector< std::pair< std::string, Texture* >, tbb::scalable_allocator<std::pair< std::string, Texture* >> >::iterator it = GetTextures()->begin(); it!=GetTextures()->end(); ++it )
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
	pd3d11Dispatcher->GetImmediateContext()->DrawIndexed( m_pGeometryChunk->GetNumberOfVertices(), 0, 0 );
	pd3d11Dispatcher->t_drawcalls += timer.GetMiliseconds();

	// Unbind textures
	for ( std::vector< std::pair< std::string, Texture* >, tbb::scalable_allocator<std::pair< std::string, Texture* >> >::iterator it = GetTextures()->begin(); it!=GetTextures()->end(); ++it )
	{
		if ( it->second )
		it->second->Unbind( it->first, pDispatcher );
	}
	
	pDispatcher->numVerts += m_pGeometryChunk->GetNumberOfVertices();
	pDispatcher->drawcalls++;
	//pDispatcher->t_work += timer.GetMiliseconds();
}

void DX11RenderCommand_Draw::SetTexture( const std::string& name, Texture* pTexture )
{
	std::vector< std::pair< std::string, Texture* >, tbb::scalable_allocator<std::pair< std::string, Texture* >> >::iterator it = std::find_if( m_pTextures.begin(), m_pTextures.end(), FindFirst<std::string, Texture*>( name ) );
	if (it==m_pTextures.end())
		m_pTextures.push_back( std::pair< std::string, Texture* > (name,pTexture) );
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

void D3D11RenderCommand_Draw::Execute( RenderDispatcher *pDispatcher )
{
	//pDispatcher->drawcalls++;
	
	//Timer timer;
	
	DX11RenderDispatcher* pd3d11Dispatcher = static_cast<DX11RenderDispatcher*>(pDispatcher);
	/*
	//pDispatcher->drawcalls += 1;
	if (pDispatcher->drawcalls==0)
	{
		pDispatcher->t_starve = pDispatcher->timer2.GetMiliseconds();
	}*/
	
	m_pShaderset->Bind( pDispatcher );
	Timer timer;
	pDispatcher->BindShaderParams( &m_ShaderParamBlock );
	pd3d11Dispatcher->t_bindparams += timer.GetMiliseconds();
	m_pGeometryChunk->Bind( pDispatcher );

	timer.Start();
	for ( std::vector< std::pair< std::string, DX11Texture2D* >, tbb::scalable_allocator<std::pair< std::string, DX11Texture2D* >> >::iterator it = GetTextures()->begin(); it!=GetTextures()->end(); ++it )
	{
		if ( it->second )
		it->second->Bind( it->first, pDispatcher );
	}
	pd3d11Dispatcher->t_bindtextures += timer.GetMiliseconds();

	timer.Start();
	pd3d11Dispatcher->GetImmediateContext()->RSSetState( m_pRasterizerState );
	pd3d11Dispatcher->GetImmediateContext()->OMSetDepthStencilState( m_pDepthStencilState, 0 );
	pd3d11Dispatcher->GetImmediateContext()->OMSetBlendState( m_pBlendState, NULL, 0xffffffff );
	pd3d11Dispatcher->GetImmediateContext()->DrawIndexed( m_pGeometryChunk->GetNumberOfVertices(), 0, 0 );
	pd3d11Dispatcher->t_drawcalls += timer.GetMiliseconds();
	
	for ( std::vector< std::pair< std::string, DX11Texture2D* >, tbb::scalable_allocator<std::pair< std::string, DX11Texture2D* >> >::iterator it = GetTextures()->begin(); it!=GetTextures()->end(); ++it )
	{
		if ( it->second )
		it->second->Unbind( it->first, pDispatcher );
	}
	
	pDispatcher->numVerts += m_pGeometryChunk->GetNumberOfVertices();
	pDispatcher->drawcalls++;
	//pDispatcher->t_work += timer.GetMiliseconds();
}

void D3D11RenderCommand_Draw::SetTexture( std::string name, DX11Texture2D *pTexture )
{
	std::vector< std::pair< std::string, DX11Texture2D* >, tbb::scalable_allocator<std::pair< std::string, DX11Texture2D* >> >::iterator it = std::find_if( m_pTextures.begin(), m_pTextures.end(), FindFirst<std::string, DX11Texture2D*>( name ) );
	if (it==m_pTextures.end())
		m_pTextures.push_back( std::pair< std::string, DX11Texture2D* > (name,pTexture) );
	else
		it->second = pTexture;
}

void D3D11RenderCommand_BeginFrame::Execute( RenderDispatcher *pDispatcher )
{
	Timer timer;

	pDispatcher->frameFinished = false;
	pDispatcher->renderTimer.Start();
	pDispatcher->t_work = 0.0f;
	pDispatcher->t_renderloop = 0.0f;
	pDispatcher->t_bindparams = 0.0f;
	pDispatcher->t_1 = 0.0f;
	pDispatcher->t_2 = 0.0f;
	pDispatcher->t_bindtextures = 0.0f;
	pDispatcher->t_drawcalls = 0.0f;
	pDispatcher->numVerts = 0.0f;
	pDispatcher->drawcalls = 0;
	pDispatcher->ClearBackbuffer( m_ClearColor );
	pDispatcher->ClearDepthStencil( m_Depth, m_Stencil );

	pDispatcher->t_BeginFrame = timer.GetMiliseconds();

	//pDispatcher->t_work += timer2.GetMiliseconds();
}

void D3D11RenderCommand_ClearTexture::Execute( RenderDispatcher *pDispatcher )
{
	Texture* texture;

	texture = dynamic_cast<DX11Texture2D*>(m_pTexture);
	if ( texture )
	{
		((DX11RenderDispatcher*)pDispatcher)->GetImmediateContext()->ClearRenderTargetView( ((DX11Texture2D*)m_pTexture)->GetRenderTargetView(), m_ClearColor );
		((DX11RenderDispatcher*)pDispatcher)->GetImmediateContext()->ClearDepthStencilView( ((DX11Texture2D*)m_pTexture)->GetDepthStencilView(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0 );
	}

	texture = dynamic_cast<DX11TextureCube*>(m_pTexture);
	if ( texture )
	{
		((DX11RenderDispatcher*)pDispatcher)->GetImmediateContext()->ClearRenderTargetView( ((DX11TextureCube*)m_pTexture)->GetRenderTargetView(), m_ClearColor );
		((DX11RenderDispatcher*)pDispatcher)->GetImmediateContext()->ClearDepthStencilView( ((DX11TextureCube*)m_pTexture)->GetDepthStencilView(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0 );
	}

	/*texture = dynamic_cast<DX11Texture3D*>(m_pTexture);
	if ( texture )
	{
		((DX11RenderDispatcher*)pDispatcher)->GetImmediateContext()->ClearRenderTargetView( ((DX11Texture3D*)m_pTexture)->GetRenderTargetView(), m_ClearColor );
		((DX11RenderDispatcher*)pDispatcher)->GetImmediateContext()->ClearDepthStencilView( ((DX11Texture3D*)m_pTexture)->GetDepthStencilView(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0 );
	}*/
}

void D3D11RenderCommand_EndFrame::Execute( RenderDispatcher *pDispatcher )
{
	Timer timer;

	pDispatcher->Present( 0 );
	//double dt = timer.GetMiliseconds();
	//pDispatcher->t_renderloop += dt;
	pDispatcher->t_renderthread = pDispatcher->renderTimer.GetMiliseconds();
	//pDispatcher->t_work += dt;
	//pDispatcher->frameFinished = true;
	pDispatcher->m_pRenderSystem->SignalFrameFinished();
	
	pDispatcher->t_EndFrame = timer.GetMiliseconds();
}

void D3D11RenderCommand_RenderTarget::Execute( RenderDispatcher* pDispatcher )
{
	if( m_pRenderTargetTexture )
	{
		pDispatcher->SetRenderTarget( m_pRenderTargetTexture );
		
		/*
		// Setting default viewports already done by RenderDispatcher::SetRenderTarget()
		D3D11_VIEWPORT vp;
		vp.Width = (FLOAT)m_pRenderTargetTexture->GetWidth();
		vp.Height = (FLOAT)m_pRenderTargetTexture->GetHeight();
		vp.MinDepth = 0.0f;
		vp.MaxDepth = 1.0f;
		vp.TopLeftX = 0;
		vp.TopLeftY = 0;
		((DX11RenderDispatcher*)pDispatcher)->GetImmediateContext()->RSSetViewports( 1, &vp );*/
	}
	else
		pDispatcher->SetBackbufferAsRenderTarget();	// Default
}

void D3D11RenderCommand_MRT::SetRenderTargetTextures( UINT numRenderTargets, DX11Texture2D** pTextureArray )	
{ 
	m_numRenderTargets = numRenderTargets; 
	for(int i=0; i<numRenderTargets; i++)
		m_pRenderTargetTextureArray.push_back(pTextureArray[i]); 
}

void D3D11RenderCommand_MRT::Execute( RenderDispatcher* pDispatcher )
{
	if( m_pRenderTargetTextureArray.size() )
	{
		pDispatcher->SetMultipleRenderTargets( m_numRenderTargets, (Texture2D**)&m_pRenderTargetTextureArray[0] );
		
		/*
		// Setting default viewports already done by RenderDispatcher::SetRenderTarget()
		D3D11_VIEWPORT vp;
		vp.Width = (FLOAT)m_pRenderTargetTexture->GetWidth();
		vp.Height = (FLOAT)m_pRenderTargetTexture->GetHeight();
		vp.MinDepth = 0.0f;
		vp.MaxDepth = 1.0f;
		vp.TopLeftX = 0;
		vp.TopLeftY = 0;
		((DX11RenderDispatcher*)pDispatcher)->GetImmediateContext()->RSSetViewports( 1, &vp );*/
	}
	else
		pDispatcher->SetBackbufferAsRenderTarget();	// Default
}

void D3D11RenderCommand_ResolveMSAA::Execute( RenderDispatcher* pDispatcher )
{
	pDispatcher->ResolveMSAA( m_pDestination, m_pSource );
}

void D3D11RenderCommand_Draw::Clone( RenderCommand* ptr )
{
	new (ptr) D3D11RenderCommand_Draw(*this);
}

void D3D11RenderCommand_BeginFrame::Clone( RenderCommand* ptr )
{
	new (ptr) D3D11RenderCommand_BeginFrame(*this);
}

void D3D11RenderCommand_EndFrame::Clone( RenderCommand* ptr )
{
	new (ptr) D3D11RenderCommand_EndFrame(*this);
}

void D3D11RenderCommand_ClearTexture::Clone( RenderCommand* ptr )
{
	new (ptr) D3D11RenderCommand_ClearTexture(*this);
}

void D3D11RenderCommand_RenderTarget::Clone( RenderCommand* ptr )
{
	new (ptr) D3D11RenderCommand_RenderTarget(*this);
}

void D3D11RenderCommand_MRT::Clone( RenderCommand* ptr )
{
	new (ptr) D3D11RenderCommand_MRT(*this);
}

void D3D11RenderCommand_ResolveMSAA::Clone( RenderCommand* ptr )
{
	new (ptr) D3D11RenderCommand_ResolveMSAA(*this);
}