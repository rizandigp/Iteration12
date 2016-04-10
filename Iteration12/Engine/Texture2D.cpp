#pragma once

#include "Texture2D.h"



void DX11Texture2D::bind( std::string const &name, RenderDispatcher* pDispatcher )
{
	DX11RenderDispatcher* pd3d11Dispatcher = dynamic_cast<DX11RenderDispatcher*>(pDispatcher);
	int index = pd3d11Dispatcher->getActiveShaderset()->getVSShaderResourceIndex( name );
	if (index!=-1) 
		pd3d11Dispatcher->getImmediateContext()->VSSetShaderResources( index, 1, &m_pSRV );

	index = pd3d11Dispatcher->getActiveShaderset()->getPSShaderResourceIndex( name );
	if (index!=-1) 
		pd3d11Dispatcher->getImmediateContext()->PSSetShaderResources( index, 1, &m_pSRV );
}

void DX11Texture2D::unbind( std::string const &name, RenderDispatcher* pDispatcher )
{
	DX11RenderDispatcher* pd3d11Dispatcher = dynamic_cast<DX11RenderDispatcher*>(pDispatcher);
	ID3D11ShaderResourceView* nullSRV[1];
	nullSRV[0]=NULL;

	INT index = pd3d11Dispatcher->getActiveShaderset()->getVSShaderResourceIndex( name );
	if (index!=-1) 
		pd3d11Dispatcher->getImmediateContext()->VSSetShaderResources( index, 1, nullSRV );

	index = pd3d11Dispatcher->getActiveShaderset()->getPSShaderResourceIndex( name );
	if (index!=-1) 
		pd3d11Dispatcher->getImmediateContext()->PSSetShaderResources( index, 1, nullSRV );
}

void DX11Texture2D::bindRenderTarget( RenderDispatcher* pDispatcher )
{
	DX11RenderDispatcher* pd3d11Dispatcher = dynamic_cast<DX11RenderDispatcher*>(pDispatcher);
	ID3D11RenderTargetView *pRTV = getRenderTargetView();
	ID3D11DepthStencilView *pDSV = getDepthStencilView();
	pd3d11Dispatcher->getImmediateContext()->OMSetRenderTargets( 1, &pRTV , pDSV );
}

void DX11Texture2D::unbindRenderTarget( RenderDispatcher* pDispatcher )
{
}

void DX11Texture2D::clearBuffer( float* clearColorRGBA, float depth, UINT8 stencil )
{
	m_pRenderSystem->clearTexture(this, clearColorRGBA);
}

void DX11Texture2D::releaseResources()
{
	m_pResource->Release();
	m_pSRV->Release();
	m_pRTV->Release();
	m_pDSV->Release();
}


void DX10Texture2D::bind( RenderDispatcher* pDispatcher, std::string name )
{
}

void DX10Texture2D::unbind( RenderDispatcher* pDispatcher, std::string name )
{
}

void DX10Texture2D::bindRenderTarget( RenderDispatcher* pDispatcher )
{
}

void DX10Texture2D::unbindRenderTarget( RenderDispatcher* pDispatcher )
{
}