#pragma once

#include "Texture2D.h"



void DX11Texture2D::Bind( std::string const &name, RenderDispatcher* pDispatcher )
{
	DX11RenderDispatcher* pd3d11Dispatcher = dynamic_cast<DX11RenderDispatcher*>(pDispatcher);
	int index = pd3d11Dispatcher->GetActiveShaderset()->GetVSShaderResourceIndex( name );
	if (index!=-1) 
		pd3d11Dispatcher->GetImmediateContext()->VSSetShaderResources( index, 1, &m_pSRV );

	index = pd3d11Dispatcher->GetActiveShaderset()->GetPSShaderResourceIndex( name );
	if (index!=-1) 
		pd3d11Dispatcher->GetImmediateContext()->PSSetShaderResources( index, 1, &m_pSRV );
}

void DX11Texture2D::Unbind( std::string const &name, RenderDispatcher* pDispatcher )
{
	DX11RenderDispatcher* pd3d11Dispatcher = dynamic_cast<DX11RenderDispatcher*>(pDispatcher);
	ID3D11ShaderResourceView* nullSRV[1];
	nullSRV[0]=NULL;

	INT index = pd3d11Dispatcher->GetActiveShaderset()->GetVSShaderResourceIndex( name );
	if (index!=-1) 
		pd3d11Dispatcher->GetImmediateContext()->VSSetShaderResources( index, 1, nullSRV );

	index = pd3d11Dispatcher->GetActiveShaderset()->GetPSShaderResourceIndex( name );
	if (index!=-1) 
		pd3d11Dispatcher->GetImmediateContext()->PSSetShaderResources( index, 1, nullSRV );
}

void DX11Texture2D::BindRenderTarget( RenderDispatcher* pDispatcher )
{
	DX11RenderDispatcher* pd3d11Dispatcher = dynamic_cast<DX11RenderDispatcher*>(pDispatcher);
	ID3D11RenderTargetView *pRTV = GetRenderTargetView();
	ID3D11DepthStencilView *pDSV = GetDepthStencilView();
	pd3d11Dispatcher->GetImmediateContext()->OMSetRenderTargets( 1, &pRTV , pDSV );
}

void DX11Texture2D::UnBindRenderTarget( RenderDispatcher* pDispatcher )
{
}

void DX11Texture2D::ClearBuffer( float* clearColorRGBA, float depth, UINT8 stencil )
{
	m_pRenderSystem->ClearTexture(this, clearColorRGBA);
}

void DX11Texture2D::ReleaseResources()
{
	m_pResource->Release();
	m_pSRV->Release();
	m_pRTV->Release();
	m_pDSV->Release();
}


void DX10Texture2D::Bind( RenderDispatcher* pDispatcher, std::string name )
{
}

void DX10Texture2D::Unbind( RenderDispatcher* pDispatcher, std::string name )
{
}

void DX10Texture2D::BindRenderTarget( RenderDispatcher* pDispatcher )
{
}

void DX10Texture2D::UnBindRenderTarget( RenderDispatcher* pDispatcher )
{
}