#pragma once

#include "DX11Texture3D.h"
#include "DX11RenderDispatcher.h"
#include "DX11Shaderset.h"

DX11Texture3D::DX11Texture3D() :	m_pResource(NULL),
									m_pSRV(NULL),
									m_pRTV(NULL),
									m_pDSV(NULL)
{
}

void DX11Texture3D::Bind( const std::string& name, RenderDispatcher* pDispatcher )
{
	DX11RenderDispatcher* pd3d11Dispatcher = static_cast<DX11RenderDispatcher*>(pDispatcher);
	INT index = pd3d11Dispatcher->GetActiveShaderset()->GetVSShaderResourceIndex( name );
	if (index!=-1) 
		pd3d11Dispatcher->GetImmediateContext()->VSSetShaderResources( index, 1, &m_pSRV );

	index = pd3d11Dispatcher->GetActiveShaderset()->GetPSShaderResourceIndex( name );
	if (index!=-1) 
		pd3d11Dispatcher->GetImmediateContext()->PSSetShaderResources( index, 1, &m_pSRV );
}

void DX11Texture3D::Unbind( const std::string& name, RenderDispatcher* pDispatcher )
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

/*void DX11Texture3D::ClearBuffer( float* clearColorRGBA, float depth, UINT8 stencil )
{
	m_pRenderSystem->ClearTexture(this, clearColorRGBA);
}*/
