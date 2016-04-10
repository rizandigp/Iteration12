#pragma once

#include "Prerequisites.h"

#include <d3d11.h>
#include "GPUResource.h"
#include "Texture2D.h"

class Texture3D : public GPUResource
{
public:
	Texture3D() : m_Height(0), m_Width(0), m_Depth(0), m_pRenderSystem(NULL)	{};

	inline void setRenderSystem( RenderSystem* ptr )					{ m_pRenderSystem = ptr; };
	inline void setDimensions( UINT height, UINT width, UINT depth )	{ m_Height = height; m_Width = width; m_Depth = depth; };

	inline RenderSystem* getRenderSystem()							{ return m_pRenderSystem; };
	inline UINT getHeight()											{ return m_Height; };
	inline UINT getWidth()											{ return m_Width; };
	inline UINT getDepth()											{ return m_Depth; };

	virtual void bind( std::string name, RenderDispatcher* pDispatcher )=0;
	virtual void unbind( std::string name, RenderDispatcher* pDispatcher )=0;
	//virtual void bindRenderTarget( RenderDispatcher* pDispatcher)=0;
	//virtual void unbindRenderTarget( RenderDispatcher* pDispatcher)=0;

	virtual void updateData(const void* data)=0;
	virtual void clearBuffer( float* clearColorRGBA, float depth, UINT8 stencil )=0;

protected:
	UINT						m_Height, m_Width, m_Depth, m_Miplevels;
	RenderSystem*				m_pRenderSystem;		// Owner
	TEXTURE_FORMAT				m_Format;
};

class DX11Texture3D : public Texture3D
{
public:
	DX11Texture3D() : m_pResource(NULL), m_pSRV(NULL), m_pRTV(NULL), m_pDSV(NULL)	{};

	void bind(  std::string name, RenderDispatcher* pDispatcher );
	void unbind(  std::string name, RenderDispatcher* pDispatcher );
	//void bindRenderTarget( RenderDispatcher* pDispatcher );
	//void unbindRenderTarget( RenderDispatcher* pDispatcher );

	void updateData(const void* data);
	void clearBuffer( float* clearColorRGBA, float depth, UINT8 stencil ) {};

	inline void setResource( ID3D11Texture3D* ptr )						{ m_pResource = ptr; };
	inline void setShaderResourceView( ID3D11ShaderResourceView* pSRV )	{ m_pSRV = pSRV; };
	//inline void setRenderTargetView( ID3D11RenderTargetView* pRTV )		{ m_pRTV = pRTV; };
	//inline void setDepthStencilView( ID3D11DepthStencilView* pDSV )		{ m_pDSV = pDSV; };
	
	inline ID3D11Texture3D* getResource()							{ return m_pResource; };
	inline ID3D11ShaderResourceView* getShaderResourceView()		{ return m_pSRV; };
	//inline ID3D11RenderTargetView* getRenderTargetView()			{ return m_pRTV; };
	//inline ID3D11DepthStencilView* getDepthStencilView()			{ return m_pDSV; };

private:
	ID3D11Texture3D*			m_pResource;
	ID3D11ShaderResourceView*	m_pSRV;
	ID3D11RenderTargetView*		m_pRTV;
	ID3D11DepthStencilView*		m_pDSV;
};