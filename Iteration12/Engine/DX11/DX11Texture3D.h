#pragma once

#include "..\Texture3D.h"
#include <d3d11.h>


// The DX11 implementation of the Texture3D class
class DX11Texture3D : public Texture3D
{
public:
	DX11Texture3D();

	void Bind( const std::string& name, RenderDispatcher* pDispatcher );
	void Unbind( const std::string& name, RenderDispatcher* pDispatcher );

	void UpdateData(const void* data);
	//void ClearBuffer( float* clearColorRGBA, float depth, UINT8 stencil );

	inline void SetResource( ID3D11Texture3D* ptr )						{ m_pResource = ptr; };
	inline void SetShaderResourceView( ID3D11ShaderResourceView* pSRV )	{ m_pSRV = pSRV; };
	
	inline ID3D11Texture3D* GetResource()							{ return m_pResource; };
	inline ID3D11ShaderResourceView* GetShaderResourceView()		{ return m_pSRV; };

private:
	ID3D11Texture3D*			m_pResource;
	ID3D11ShaderResourceView*	m_pSRV;
	ID3D11RenderTargetView*		m_pRTV;
	ID3D11DepthStencilView*		m_pDSV;
};