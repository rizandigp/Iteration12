#pragma once

#include "Texture3D.h"

void DX11Texture3D::bind( std::string name, RenderDispatcher* pDispatcher )
{
	DX11RenderDispatcher* pd3d11Dispatcher = static_cast<DX11RenderDispatcher*>(pDispatcher);
	INT index = pd3d11Dispatcher->getActiveShaderset()->getVSShaderResourceIndex( name );
	if (index!=-1) 
		pd3d11Dispatcher->getImmediateContext()->VSSetShaderResources( index, 1, &m_pSRV );

	index = pd3d11Dispatcher->getActiveShaderset()->getPSShaderResourceIndex( name );
	if (index!=-1) 
		pd3d11Dispatcher->getImmediateContext()->PSSetShaderResources( index, 1, &m_pSRV );
}

void DX11Texture3D::unbind( std::string name, RenderDispatcher* pDispatcher )
{
	DX11RenderDispatcher* pd3d11Dispatcher = static_cast<DX11RenderDispatcher*>(pDispatcher);
	ID3D11ShaderResourceView* nullSRV[1] = {NULL};

	INT index = pd3d11Dispatcher->getActiveShaderset()->getVSShaderResourceIndex( name );
	if (index!=-1) 
		pd3d11Dispatcher->getImmediateContext()->VSSetShaderResources( index, 1, nullSRV );

	index = pd3d11Dispatcher->getActiveShaderset()->getPSShaderResourceIndex( name );
	if (index!=-1) 
		pd3d11Dispatcher->getImmediateContext()->PSSetShaderResources( index, 1, nullSRV );
}

void DX11Texture3D::updateData(const void* data)
{
}

/*
void D3D11Texture3D::bindRenderTarget( RenderDispatcher* pDispatcher )
{
	D3D11RenderDispatcher* pd3d11Dispatcher = static_cast<D3D11RenderDispatcher*>(pDispatcher);
	ID3D11RenderTargetView *pRTV = getRenderTargetView();
	ID3D11DepthStencilView *pDSV = getDepthStencilView();
	pd3d11Dispatcher->getImmediateContext()->OMSetRenderTargets( 1, &pRTV , pDSV );
}

void D3D11Texture3D::unbindRenderTarget( RenderDispatcher* pDispatcher )
{
}
*/
/*
void D3D11Texture3D::clearBuffer( float* clearColorRGBA, float depth, UINT8 stencil )
{
	m_pRenderSystem->clearTexture(this, clearColorRGBA);
}*/