#pragma once

#include "..\TextureCube.h"
#include <d3d11.h>


// The DX11 implementation of the TextureCube class
class DX11TextureCube : public TextureCube
{
public:
	DX11TextureCube();

	void Bind( const std::string& name, RenderDispatcher* pDispatcher );
	void Unbind( const std::string& name, RenderDispatcher* pDispatcher );

	void ClearBuffer( float* clearColorRGBA, float depth, UINT8 stencil );

	void ReleaseResources();
	// TODO : Other BRDFs
	void Prefilter();

	// Set DX11 objects
	inline void SetResource( ID3D11Texture2D* pResource )				{ m_pResource = pResource; };
	inline void SetShaderResourceView( ID3D11ShaderResourceView* pSRV )	{ m_pSRV = pSRV; };
	inline void SetRenderTargetView( ID3D11RenderTargetView* pRTV )		{ m_pRTV = pRTV; };
	inline void SetDepthStencilView( ID3D11DepthStencilView* pDSV )		{ m_pDSV = pDSV; };
	
	// Get DX11 objects
	inline ID3D11Texture2D* GetResource()							{ return m_pResource; };
	inline ID3D11ShaderResourceView* GetShaderResourceView()		{ return m_pSRV; };
	inline ID3D11RenderTargetView* GetRenderTargetView()			{ return m_pRTV; };
	inline ID3D11DepthStencilView* GetDepthStencilView()			{ return m_pDSV; };

private:
	ID3D11Texture2D*			m_pResource;
	ID3D11ShaderResourceView*	m_pSRV;
	ID3D11RenderTargetView*		m_pRTV;
	ID3D11DepthStencilView*		m_pDSV;
};