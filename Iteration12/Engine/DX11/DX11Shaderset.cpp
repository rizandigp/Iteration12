#pragma once

#include "DX11Shaderset.h"
#include "DX11RenderDispatcher.h"


DX11Shaderset::DX11Shaderset()
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

DX11Shaderset::~DX11Shaderset()
{
	SAFE_RELEASE( m_pVertexShader );
	SAFE_RELEASE( m_pGeometryShader );
	SAFE_RELEASE( m_pPixelShader );
	SAFE_RELEASE( m_pHullShader );
	SAFE_RELEASE( m_pDomainShader );
	SAFE_RELEASE( m_pComputeShader );
	SAFE_RELEASE( m_pInputSignature );
}

void DX11Shaderset::Bind( RenderDispatcher* pDispatcher )
{
	DX11RenderDispatcher* pd3d11Dispatcher = dynamic_cast<DX11RenderDispatcher*>(pDispatcher);
	pd3d11Dispatcher->SetActiveShaderset( this );
	pd3d11Dispatcher->GetImmediateContext()->VSSetShader( m_pVertexShader, NULL, 0 );
	pd3d11Dispatcher->GetImmediateContext()->PSSetShader( m_pPixelShader, NULL, 0 );
}