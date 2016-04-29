#pragma once

#include "..\Texture2D.h"

//
// I don't think DX10 will ever be implemented
//
class DX10Texture2D : public Texture2D
{
public:
	DX10Texture2D();

	void Bind( const std::string& name, RenderDispatcher* pDispatcher );
	void Unbind( const std::string& name, RenderDispatcher* pDispatcher );
	void BindRenderTarget( RenderDispatcher* pDispatcher );
	void UnbindRenderTarget( RenderDispatcher* pDispatcher );

	inline void SetShaderResourceView( ID3D10ShaderResourceView* pSRV )	{ m_pSRV = pSRV; };
	inline void SetRenderTargetView( ID3D10RenderTargetView* pRTV )		{ m_pRTV = pRTV; };
	inline void SetDepthStencilView( ID3D10DepthStencilView* pDSV )		{ m_pDSV = pDSV; };
	
	inline ID3D10ShaderResourceView* GetShaderResourceView()		{ return m_pSRV; };
	inline ID3D10RenderTargetView* GetRenderTargetView()			{ return m_pRTV; };
	inline ID3D10DepthStencilView* GetDepthStencilView()			{ return m_pDSV; };

private:
	ID3D10ShaderResourceView*	m_pSRV;
	ID3D10RenderTargetView*		m_pRTV;
	ID3D10DepthStencilView*		m_pDSV;
};