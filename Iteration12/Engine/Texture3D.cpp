#pragma once

#include "Texture3D.h"

void DX11Texture3D::Bind( std::string name, RenderDispatcher* pDispatcher )
{
	DX11RenderDispatcher* pd3d11Dispatcher = static_cast<DX11RenderDispatcher*>(pDispatcher);
	INT index = pd3d11Dispatcher->GetActiveShaderset()->GetVSShaderResourceIndex( name );
	if (index!=-1) 
		pd3d11Dispatcher->GetImmediateContext()->VSSetShaderResources( index, 1, &m_pSRV );

	index = pd3d11Dispatcher->GetActiveShaderset()->GetPSShaderResourceIndex( name );
	if (index!=-1) 
		pd3d11Dispatcher->GetImmediateContext()->PSSetShaderResources( index, 1, &m_pSRV );
}

void DX11Texture3D::Unbind( std::string name, RenderDispatcher* pDispatcher )
{
	DX11RenderDispatcher* pd3d11Dispatcher = static_cast<DX11RenderDispatcher*>(pDispatcher);
	ID3D11ShaderResourceView* nullSRV[1] = {NULL};

	INT index = pd3d11Dispatcher->GetActiveShaderset()->GetVSShaderResourceIndex( name );
	if (index!=-1) 
		pd3d11Dispatcher->GetImmediateContext()->VSSetShaderResources( index, 1, nullSRV );

	index = pd3d11Dispatcher->GetActiveShaderset()->GetPSShaderResourceIndex( name );
	if (index!=-1) 
		pd3d11Dispatcher->GetImmediateContext()->PSSetShaderResources( index, 1, nullSRV );
}

// TODO : implement
void DX11Texture3D::UpdateData(const void* data)
{
}

/*
void D3D11Texture3D::BindRenderTarget( RenderDispatcher* pDispatcher )
{
	D3D11RenderDispatcher* pd3d11Dispatcher = static_cast<D3D11RenderDispatcher*>(pDispatcher);
	ID3D11RenderTargetView *pRTV = GetRenderTargetView();
	ID3D11DepthStencilView *pDSV = getDepthStencilView();
	pd3d11Dispatcher->GetImmediateContext()->OMSetRenderTargets( 1, &pRTV , pDSV );
}

void D3D11Texture3D::UnBindRenderTarget( RenderDispatcher* pDispatcher )
{
}
*/
/*
void D3D11Texture3D::ClearBuffer( float* clearColorRGBA, float depth, UINT8 stencil )
{
	m_pRenderSystem->ClearTexture(this, clearColorRGBA);
}*/