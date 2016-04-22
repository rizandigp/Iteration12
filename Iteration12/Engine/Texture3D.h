#pragma once

#include "Prerequisites.h"

#include <d3d11.h>
#include "GPUResource.h"
#include "Texture2D.h"

class Texture3D : public GPUResource
{
public:
	Texture3D() : m_Height(0), m_Width(0), m_Depth(0), m_pRenderSystem(NULL)	{};

	inline void SetRenderSystem( RenderSystem* ptr )					{ m_pRenderSystem = ptr; };
	inline void SetDimensions( UINT height, UINT width, UINT depth )	{ m_Height = height; m_Width = width; m_Depth = depth; };

	inline RenderSystem* GetRenderSystem()							{ return m_pRenderSystem; };
	inline UINT GetHeight()											{ return m_Height; };
	inline UINT GetWidth()											{ return m_Width; };
	inline UINT GetDepth()											{ return m_Depth; };

	virtual void Bind( std::string name, RenderDispatcher* pDispatcher )=0;
	virtual void Unbind( std::string name, RenderDispatcher* pDispatcher )=0;
	//virtual void BindRenderTarget( RenderDispatcher* pDispatcher)=0;
	//virtual void UnBindRenderTarget( RenderDispatcher* pDispatcher)=0;

	virtual void UpdateData(const void* data)=0;
	virtual void ClearBuffer( float* clearColorRGBA, float depth, UINT8 stencil )=0;

protected:
	UINT						m_Height, m_Width, m_Depth, m_Miplevels;
	RenderSystem*				m_pRenderSystem;		// Owner
	TEXTURE_FORMAT				m_Format;
};

class DX11Texture3D : public Texture3D
{
public:
	DX11Texture3D() : m_pResource(NULL), m_pSRV(NULL), m_pRTV(NULL), m_pDSV(NULL)	{};

	void Bind(  std::string name, RenderDispatcher* pDispatcher );
	void Unbind(  std::string name, RenderDispatcher* pDispatcher );
	//void BindRenderTarget( RenderDispatcher* pDispatcher );
	//void UnBindRenderTarget( RenderDispatcher* pDispatcher );

	void UpdateData(const void* data);
	void ClearBuffer( float* clearColorRGBA, float depth, UINT8 stencil ) {};

	inline void SetResource( ID3D11Texture3D* ptr )						{ m_pResource = ptr; };
	inline void SetShaderResourceView( ID3D11ShaderResourceView* pSRV )	{ m_pSRV = pSRV; };
	//inline void SetRenderTargetView( ID3D11RenderTargetView* pRTV )		{ m_pRTV = pRTV; };
	//inline void setDepthStencilView( ID3D11DepthStencilView* pDSV )		{ m_pDSV = pDSV; };
	
	inline ID3D11Texture3D* GetResource()							{ return m_pResource; };
	inline ID3D11ShaderResourceView* GetShaderResourceView()		{ return m_pSRV; };
	//inline ID3D11RenderTargetView* GetRenderTargetView()			{ return m_pRTV; };
	//inline ID3D11DepthStencilView* getDepthStencilView()			{ return m_pDSV; };

private:
	ID3D11Texture3D*			m_pResource;
	ID3D11ShaderResourceView*	m_pSRV;
	ID3D11RenderTargetView*		m_pRTV;
	ID3D11DepthStencilView*		m_pDSV;
};