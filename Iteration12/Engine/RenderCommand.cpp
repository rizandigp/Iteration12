#pragma once

#include "RenderCommand.h"
#include "RenderSystem.h"
#include "GeometryChunk.h"
#include "Shaderset.h"
#include "Texture2D.h"
#include "ShaderParamBlock.h"
#include "ShaderParams.h"
#include "Math.h"
#include "Utilities.h"
#include "ConstantBufferData.h"
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

void GenericRenderCommand::execute( RenderDispatcher *pDispatcher )
{
	if (!m_pRenderTargetTexture)
	{
		m_pShaderset->bind( pDispatcher );
		pDispatcher->bindShaderParams( &m_ShaderParamBlock );

		for ( std::vector< std::pair< std::string, Texture2D* > >::iterator it = getTextures()->begin(); it!=getTextures()->end(); ++it )
		{
			if ( it->second )
			it->second->bind( it->first, pDispatcher );
		}

		m_pGeometryChunk->bind( pDispatcher );
	}
	else
	{
			static_cast<DX11RenderDispatcher*>(pDispatcher)->setRenderTarget( getRenderTargetTexture() );
			D3D11_VIEWPORT vp;
			vp.Width = getRenderTargetTexture()->getWidth();
			vp.Height = getRenderTargetTexture()->getHeight();
			vp.MinDepth = 0.0f;
			vp.MaxDepth = 1.0f;
			vp.TopLeftX = 0;
			vp.TopLeftY = 0;
			static_cast<DX11RenderDispatcher*>(pDispatcher)->getImmediateContext()->RSSetViewports( 1, &vp );
	}
}

void D3D11RenderCommand_Draw::execute( RenderDispatcher *pDispatcher )
{
	//pDispatcher->drawcalls++;
	
	//Timer timer;
	
	DX11RenderDispatcher* pd3d11Dispatcher = static_cast<DX11RenderDispatcher*>(pDispatcher);
	/*
	//pDispatcher->drawcalls += 1;
	if (pDispatcher->drawcalls==0)
	{
		pDispatcher->t_starve = pDispatcher->timer2.getMiliseconds();
	}*/
	
	m_pShaderset->bind( pDispatcher );
	Timer timer;
	pDispatcher->bindShaderParams( &m_ShaderParamBlock );
	pd3d11Dispatcher->t_bindparams += timer.getMiliseconds();
	m_pGeometryChunk->bind( pDispatcher );

	
	
	timer.start();
	for ( std::vector< std::pair< std::string, DX11Texture2D* >, tbb::scalable_allocator<std::pair< std::string, DX11Texture2D* >> >::iterator it = getTextures()->begin(); it!=getTextures()->end(); ++it )
	{
		if ( it->second )
		it->second->bind( it->first, pDispatcher );
	}
	pd3d11Dispatcher->t_bindtextures += timer.getMiliseconds();

	timer.start();
	pd3d11Dispatcher->getImmediateContext()->RSSetState( m_pRasterizerState );
	pd3d11Dispatcher->getImmediateContext()->OMSetDepthStencilState( m_pDepthStencilState, 0 );
	pd3d11Dispatcher->getImmediateContext()->OMSetBlendState( m_pBlendState, NULL, 0xffffffff );
	pd3d11Dispatcher->getImmediateContext()->DrawIndexed( m_pGeometryChunk->getNumberOfVerts(), 0, 0 );
	pd3d11Dispatcher->t_drawcalls += timer.getMiliseconds();
	
	for ( std::vector< std::pair< std::string, DX11Texture2D* >, tbb::scalable_allocator<std::pair< std::string, DX11Texture2D* >> >::iterator it = getTextures()->begin(); it!=getTextures()->end(); ++it )
	{
		if ( it->second )
		it->second->unbind( it->first, pDispatcher );
	}
	
	pDispatcher->numVerts += m_pGeometryChunk->getNumberOfVerts();
	pDispatcher->drawcalls++;
	//pDispatcher->t_work += timer.getMiliseconds();
}

void D3D11RenderCommand_Draw::setTexture( std::string name, DX11Texture2D *pTexture )
{
	std::vector< std::pair< std::string, DX11Texture2D* >, tbb::scalable_allocator<std::pair< std::string, DX11Texture2D* >> >::iterator it = std::find_if( m_pTextures.begin(), m_pTextures.end(), FindFirst<std::string, DX11Texture2D*>( name ) );
	if (it==m_pTextures.end())
		m_pTextures.push_back( std::pair< std::string, DX11Texture2D* > (name,pTexture) );
	else
		it->second = pTexture;
}

void D3D11RenderCommand_BeginFrame::execute( RenderDispatcher *pDispatcher )
{
	Timer timer;

	pDispatcher->frameFinished = false;
	pDispatcher->renderTimer.start();
	pDispatcher->t_work = 0.0f;
	pDispatcher->t_renderloop = 0.0f;
	pDispatcher->t_bindparams = 0.0f;
	pDispatcher->t_1 = 0.0f;
	pDispatcher->t_2 = 0.0f;
	pDispatcher->t_bindtextures = 0.0f;
	pDispatcher->t_drawcalls = 0.0f;
	pDispatcher->numVerts = 0.0f;
	pDispatcher->drawcalls = 0;
	pDispatcher->clearBackbuffer( m_ClearColor );
	pDispatcher->clearDepthStencil( m_Depth, m_Stencil );

	pDispatcher->t_beginframe = timer.getMiliseconds();

	//pDispatcher->t_work += timer2.getMiliseconds();
}

void D3D11RenderCommand_ClearTexture::execute( RenderDispatcher *pDispatcher )
{
	((DX11RenderDispatcher*)pDispatcher)->getImmediateContext()->ClearRenderTargetView( ((DX11Texture2D*)m_pTexture)->getRenderTargetView(), m_ClearColor );
    ((DX11RenderDispatcher*)pDispatcher)->getImmediateContext()->ClearDepthStencilView( ((DX11Texture2D*)m_pTexture)->getDepthStencilView(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0 );
}

void D3D11RenderCommand_EndFrame::execute( RenderDispatcher *pDispatcher )
{
	Timer timer;

	pDispatcher->present( 0 );
	//double dt = timer.getMiliseconds();
	//pDispatcher->t_renderloop += dt;
	pDispatcher->t_renderthread = pDispatcher->renderTimer.getMiliseconds();
	//pDispatcher->t_work += dt;
	//pDispatcher->frameFinished = true;
	pDispatcher->m_pRenderSystem->signalFrameFinished();
	
	pDispatcher->t_endframe = timer.getMiliseconds();
}

void D3D11RenderCommand_RenderTarget::execute( RenderDispatcher* pDispatcher )
{
	if( m_pRenderTargetTexture )
	{
		pDispatcher->setRenderTarget( m_pRenderTargetTexture );
		
		/*
		// Setting default viewports already done by RenderDispatcher::setRenderTarget()
		D3D11_VIEWPORT vp;
		vp.Width = (FLOAT)m_pRenderTargetTexture->getWidth();
		vp.Height = (FLOAT)m_pRenderTargetTexture->getHeight();
		vp.MinDepth = 0.0f;
		vp.MaxDepth = 1.0f;
		vp.TopLeftX = 0;
		vp.TopLeftY = 0;
		((DX11RenderDispatcher*)pDispatcher)->getImmediateContext()->RSSetViewports( 1, &vp );*/
	}
	else
		pDispatcher->setBackbufferAsRenderTarget();	// Default
}

void D3D11RenderCommand_MRT::setRenderTargetTextures( UINT numRenderTargets, DX11Texture2D** pTextureArray )	
{ 
	m_numRenderTargets = numRenderTargets; 
	for(int i=0; i<numRenderTargets; i++)
		m_pRenderTargetTextureArray.push_back(pTextureArray[i]); 
}

void D3D11RenderCommand_MRT::execute( RenderDispatcher* pDispatcher )
{
	if( m_pRenderTargetTextureArray.size() )
	{
		pDispatcher->setMultipleRenderTargets( m_numRenderTargets, (Texture2D**)&m_pRenderTargetTextureArray[0] );
		
		/*
		// Setting default viewports already done by RenderDispatcher::setRenderTarget()
		D3D11_VIEWPORT vp;
		vp.Width = (FLOAT)m_pRenderTargetTexture->getWidth();
		vp.Height = (FLOAT)m_pRenderTargetTexture->getHeight();
		vp.MinDepth = 0.0f;
		vp.MaxDepth = 1.0f;
		vp.TopLeftX = 0;
		vp.TopLeftY = 0;
		((DX11RenderDispatcher*)pDispatcher)->getImmediateContext()->RSSetViewports( 1, &vp );*/
	}
	else
		pDispatcher->setBackbufferAsRenderTarget();	// Default
}

void D3D11RenderCommand_ResolveMSAA::execute( RenderDispatcher* pDispatcher )
{
	pDispatcher->resolveMSAA( m_pDestination, m_pSource );
}

void D3D11RenderCommand_Draw::clone( RenderCommand* ptr )
{
	new (ptr) D3D11RenderCommand_Draw(*this);
}

void D3D11RenderCommand_BeginFrame::clone( RenderCommand* ptr )
{
	new (ptr) D3D11RenderCommand_BeginFrame(*this);
}

void D3D11RenderCommand_EndFrame::clone( RenderCommand* ptr )
{
	new (ptr) D3D11RenderCommand_EndFrame(*this);
}

void D3D11RenderCommand_ClearTexture::clone( RenderCommand* ptr )
{
	new (ptr) D3D11RenderCommand_ClearTexture(*this);
}

void D3D11RenderCommand_RenderTarget::clone( RenderCommand* ptr )
{
	new (ptr) D3D11RenderCommand_RenderTarget(*this);
}

void D3D11RenderCommand_MRT::clone( RenderCommand* ptr )
{
	new (ptr) D3D11RenderCommand_MRT(*this);
}

void D3D11RenderCommand_ResolveMSAA::clone( RenderCommand* ptr )
{
	new (ptr) D3D11RenderCommand_ResolveMSAA(*this);
}

