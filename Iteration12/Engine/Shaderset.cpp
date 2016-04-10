#pragma once

#include "Shaderset.h"
#include "RenderDispatcher.h"

D3D11Shaderset::D3D11Shaderset()
{
	m_pVertexShader = NULL;
	m_pGeometryShader = NULL;
	m_pPixelShader = NULL;
	m_pHullShader = NULL;
	m_pDomainShader = NULL;
	m_pComputeShader = NULL;
	m_pInputSignature = NULL;
	numVSConstantBuffers = 0;
	numGSConstantBuffers = 0;
	numPSConstantBuffers = 0;
	numCSConstantBuffers = 0;
}

D3D11Shaderset::~D3D11Shaderset()
{
	SAFE_RELEASE( m_pVertexShader );
	SAFE_RELEASE( m_pGeometryShader );
	SAFE_RELEASE( m_pPixelShader );
	SAFE_RELEASE( m_pHullShader );
	SAFE_RELEASE( m_pDomainShader );
	SAFE_RELEASE( m_pComputeShader );
	SAFE_RELEASE( m_pInputSignature );
}

void D3D11Shaderset::bind( RenderDispatcher* pDispatcher )
{
	DX11RenderDispatcher* pd3d11Dispatcher = dynamic_cast<DX11RenderDispatcher*>(pDispatcher);
	pd3d11Dispatcher->setActiveShaderset( this );
	pd3d11Dispatcher->getImmediateContext()->VSSetShader( m_pVertexShader, NULL, 0 );
	pd3d11Dispatcher->getImmediateContext()->PSSetShader( m_pPixelShader, NULL, 0 );
}