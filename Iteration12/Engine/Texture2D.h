#pragma once

#include "Prerequisites.h"

#include <d3d11.h>
#include "RenderSystem.h"
#include "GPUResource.h"

enum TEXTURE_FORMAT
{
	TEXTURE_FORMAT_UNKNOWN		= 0,
	R32G32B32A32_TYPELESS       = 1,
    R32G32B32A32_FLOAT          = 2,
    R32G32B32A32_UINT           = 3,
    R32G32B32A32_SINT           = 4,
    R32G32B32_TYPELESS          = 5,
    R32G32B32_FLOAT             = 6,
    R32G32B32_UINT              = 7,
    R32G32B32_SINT              = 8,
    R16G16B16A16_TYPELESS       = 9,
    R16G16B16A16_FLOAT          = 10,
    R16G16B16A16_UNORM          = 11,
    R16G16B16A16_UINT           = 12,
    R16G16B16A16_SNORM          = 13,
    R16G16B16A16_SINT           = 14,
    R32G32_TYPELESS             = 15,
    R32G32_FLOAT                = 16,
    R32G32_UINT                 = 17,
    R32G32_SINT                 = 18,
    R32G8X24_TYPELESS           = 19,
    D32_FLOAT_S8X24_UINT        = 20,
    R32_FLOAT_X8X24_TYPELESS    = 21,
    X32_TYPELESS_G8X24_UINT     = 22,
    R10G10B10A2_TYPELESS        = 23,
    R10G10B10A2_UNORM           = 24,
    R10G10B10A2_UINT            = 25,
    R11G11B10_FLOAT             = 26,
    R8G8B8A8_TYPELESS           = 27,
    R8G8B8A8_UNORM              = 28,
    R8G8B8A8_UNORM_SRGB         = 29,
    R8G8B8A8_UINT               = 30,
    R8G8B8A8_SNORM              = 31,
    R8G8B8A8_SINT               = 32,
    R16G16_TYPELESS             = 33,
    R16G16_FLOAT                = 34,
    R16G16_UNORM                = 35,
    R16G16_UINT                 = 36,
    R16G16_SNORM                = 37,
    R16G16_SINT                 = 38,
    R32_TYPELESS                = 39,
    D32_FLOAT                   = 40,
    R32_FLOAT                   = 41,
    R32_UINT                    = 42,
    R32_SINT                    = 43,
    R24G8_TYPELESS              = 44,
    D24_UNORM_S8_UINT           = 45,
    R24_UNORM_X8_TYPELESS       = 46,
    X24_TYPELESS_G8_UINT        = 47,
    R8G8_TYPELESS               = 48,
    R8G8_UNORM                  = 49,
    R8G8_UINT                   = 50,
    R8G8_SNORM                  = 51,
    R8G8_SINT                   = 52,
    R16_TYPELESS                = 53,
    R16_FLOAT                   = 54,
    D16_UNORM                   = 55,
    R16_UNORM                   = 56,
    R16_UINT                    = 57,
    R16_SNORM                   = 58,
    R16_SINT                    = 59,
    R8_TYPELESS                 = 60,
    R8_UNORM                    = 61,
    R8_UINT                     = 62,
    R8_SNORM                    = 63,
    R8_SINT                     = 64,
    A8_UNORM                    = 65,
    R1_UNORM                    = 66,
    R9G9B9E5_SHAREDEXP          = 67,
    R8G8_B8G8_UNORM             = 68,
    G8R8_G8B8_UNORM             = 69,
    BC1_TYPELESS                = 70,
    BC1_UNORM                   = 71,
    BC1_UNORM_SRGB              = 72,
    BC2_TYPELESS                = 73,
    BC2_UNORM                   = 74,
    BC2_UNORM_SRGB              = 75,
    BC3_TYPELESS                = 76,
    BC3_UNORM                   = 77,
    BC3_UNORM_SRGB              = 78,
    BC4_TYPELESS                = 79,
    BC4_UNORM                   = 80,
    BC4_SNORM                   = 81,
    BC5_TYPELESS                = 82,
    BC5_UNORM                   = 83,
    BC5_SNORM                   = 84,
    B5G6R5_UNORM                = 85,
    B5G5R5A1_UNORM              = 86,
    B8G8R8A8_UNORM              = 87,
    B8G8R8X8_UNORM              = 88,
    R10G10B10_XR_BIAS_A2_UNORM  = 89,
    B8G8R8A8_TYPELESS           = 90,
    B8G8R8A8_UNORM_SRGB         = 91,
    B8G8R8X8_TYPELESS           = 92,
    B8G8R8X8_UNORM_SRGB         = 93,
    BC6H_TYPELESS               = 94,
    BC6H_UF16                   = 95,
    BC6H_SF16                   = 96,
    BC7_TYPELESS                = 97,
    BC7_UNORM                   = 98,
    BC7_UNORM_SRGB              = 99,
};

class Texture2D : public GPUResource
{
public:
	Texture2D() : m_Height(0), m_Width(0), m_pRenderSystem(NULL), m_Format(TEXTURE_FORMAT_UNKNOWN)	{};
	virtual ~Texture2D() {};

	inline void setRenderSystem( RenderSystem* ptr )					{ m_pRenderSystem = ptr; };
	inline void setDimensions( UINT height, UINT width )				{ m_Height = height; m_Width = width; };
	inline void setFormat( TEXTURE_FORMAT format )						{ m_Format = format; };
	inline void setMipLevels( UINT levels )								{ m_MipLevels = levels; };

	inline RenderSystem* getRenderSystem()							{ return m_pRenderSystem; };
	inline UINT getHeight()											{ return m_Height; };
	inline UINT getWidth()											{ return m_Width; };
	inline Vector2 getDimensions()									{ return Vector2( m_Width, m_Height ); };
	inline TEXTURE_FORMAT getFormat()								{ return m_Format; };
	inline UINT getMipLevels()										{ return m_MipLevels; };

	
	// Bind as shader input to the pipeline
	virtual void bind( std::string const &name, RenderDispatcher* pDispatcher )=0;
	// Unbind from pipeline
	virtual void unbind( std::string const &name, RenderDispatcher* pDispatcher )=0;
	// Bind to the pipeline as render target
	virtual void bindRenderTarget( RenderDispatcher* pDispatcher)=0;
	// Unbind render target from pipeline
	virtual void unbindRenderTarget( RenderDispatcher* pDispatcher)=0;

	virtual void clearBuffer( float* clearColorRGBA, float depth, UINT8 stencil )=0;
	virtual void releaseResources()=0;

protected:
	UINT						m_Height, m_Width, m_MipLevels;
	RenderSystem*				m_pRenderSystem;		// Owner
	TEXTURE_FORMAT				m_Format;
};

class DX11Texture2D : public Texture2D
{
public:
	DX11Texture2D() : m_pSRV(NULL), m_pRTV(NULL), m_pDSV(NULL)	{};

	void bind(  std::string const &name, RenderDispatcher* pDispatcher );
	void unbind(  std::string const &name, RenderDispatcher* pDispatcher );
	void bindRenderTarget( RenderDispatcher* pDispatcher );
	void unbindRenderTarget( RenderDispatcher* pDispatcher );
	void releaseResources();

	void clearBuffer( float* clearColorRGBA, float depth, UINT8 stencil );

	// Set DX11 objects
	inline void setResource( ID3D11Texture2D* pResource )				{ m_pResource = pResource; };
	inline void setShaderResourceView( ID3D11ShaderResourceView* pSRV )	{ m_pSRV = pSRV; };
	inline void setRenderTargetView( ID3D11RenderTargetView* pRTV )		{ m_pRTV = pRTV; };
	inline void setDepthStencilView( ID3D11DepthStencilView* pDSV )		{ m_pDSV = pDSV; };
	
	// Get DX11 objects
	inline ID3D11Texture2D* getResource()							{ return m_pResource; };
	inline ID3D11ShaderResourceView* getShaderResourceView()		{ return m_pSRV; };
	inline ID3D11RenderTargetView* getRenderTargetView()			{ return m_pRTV; };
	inline ID3D11DepthStencilView* getDepthStencilView()			{ return m_pDSV; };

private:
	ID3D11Texture2D*			m_pResource;
	ID3D11ShaderResourceView*	m_pSRV;
	ID3D11RenderTargetView*		m_pRTV;
	ID3D11DepthStencilView*		m_pDSV;
};

class DX10Texture2D : public Texture2D
{
public:
	DX10Texture2D() : m_pSRV(NULL), m_pRTV(NULL), m_pDSV(NULL)	{ Texture2D; };

	void bind( RenderDispatcher* pDispatcher, std::string name );
	void unbind( RenderDispatcher* pDispatcher, std::string name );
	void bindRenderTarget( RenderDispatcher* pDispatcher );
	void unbindRenderTarget( RenderDispatcher* pDispatcher );

	inline void setShaderResourceView( ID3D10ShaderResourceView* pSRV )	{ m_pSRV = pSRV; };
	inline void setRenderTargetView( ID3D10RenderTargetView* pRTV )		{ m_pRTV = pRTV; };
	inline void setDepthStencilView( ID3D10DepthStencilView* pDSV )		{ m_pDSV = pDSV; };
	
	inline ID3D10ShaderResourceView* getShaderResourceView()		{ return m_pSRV; };
	inline ID3D10RenderTargetView* getRenderTargetView()			{ return m_pRTV; };
	inline ID3D10DepthStencilView* getDepthStencilView()			{ return m_pDSV; };

private:
	ID3D10ShaderResourceView*	m_pSRV;
	ID3D10RenderTargetView*		m_pRTV;
	ID3D10DepthStencilView*		m_pDSV;
};