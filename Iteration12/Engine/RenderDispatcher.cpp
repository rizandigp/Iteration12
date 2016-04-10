#pragma once

#include "RenderDispatcher.h"
#include "GeometryChunk.h"
#include "Shaderset.h"
#include "ShaderParamBlock.h"
#include "ConstantBufferData.h"
#include "Texture2D.h"
#include "Buffer.h"
#include "ShaderParams.h"
#include "RenderCommand.h"
#include "TextureCube.h"
#include "Texture3D.h"
#include "Image.h"
#include <math.h>


DX11RenderDispatcher::DX11RenderDispatcher() : RenderDispatcher()
{
	// Null out constant buffers
	for(int i=0; i<32; i++)
	{
		m_pVSCBDataBuffer[i] = NULL;
		m_pPSCBDataBuffer[i] = NULL;
		m_pVSCBDataBufferSize[i] = 0;
		m_pPSCBDataBufferSize[i] = 0;
	}
}

HRESULT DX11RenderDispatcher::initialize( RenderDispatcherConfig creationConfig )
{
	m_Config = creationConfig;

	HRESULT hr = NULL;

	// Determine antialiasing sample count & sample quality
	UINT AACount, AAQuality;
	switch( creationConfig.BackbufferMultisampling )
	{
	case MSAA_NONE :
		AACount = 1;
		AAQuality = 0; break;
	case MSAA_2X :
		AACount = 2;
		AAQuality = 0; break;
	case MSAA_4X :
		AACount = 4;
		AAQuality = 0; break;
	case MSAA_8X :
		AACount = 8;
		AAQuality = 0; break;
	case MSAA_16X :
		AACount = 16;
		AAQuality = 0; break;
	case CSAA_8X :
		AACount = 8;
		AAQuality = 4; break;
	case CSAA_8XQ :
		AACount = 8;
		AAQuality = 8; break;
	case CSAA_16X :
		AACount = 16;
		AAQuality = 4; break;
	case CSAA_16XQ :
		AACount = 16;
		AAQuality = 8; break;
	default :
		AACount = 1;
		AAQuality = 0;
	}
	
	
    D3D_DRIVER_TYPE driverTypes[] =
    {
        D3D_DRIVER_TYPE_HARDWARE,
        D3D_DRIVER_TYPE_WARP,
        D3D_DRIVER_TYPE_REFERENCE,
    };
    UINT numDriverTypes = ARRAYSIZE( driverTypes );

    D3D_FEATURE_LEVEL featureLevels[] =
    {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
    };
    UINT numFeatureLevels = ARRAYSIZE( featureLevels );

	// Describe swap chain
    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory( &sd, sizeof( sd ) );
    sd.BufferCount = 1;
    sd.BufferDesc.Width = creationConfig.Width;
    sd.BufferDesc.Height = creationConfig.Height;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = creationConfig.RefreshRate;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = creationConfig.OutputWindow;
    sd.SampleDesc.Count = AACount;
    sd.SampleDesc.Quality = AAQuality;
    sd.Windowed = true;

	UINT createDeviceFlags = 0;
	if ( creationConfig.DebugDevice )
		createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;

	// Create DX10 device
    for( UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++ )
    {
        m_driverType = driverTypes[driverTypeIndex];
        hr = D3D11CreateDeviceAndSwapChain( NULL, m_driverType, NULL, createDeviceFlags, featureLevels, numFeatureLevels,
                                            D3D11_SDK_VERSION, &sd, &m_pSwapChain, &m_pDevice, &m_featureLevel, &m_pImmediateContext );
        if( SUCCEEDED( hr ) )
            break;
    }
 

	if( FAILED( hr ) )
	{
		DEBUG_OUTPUT( __FUNCTION__ );
		DEBUG_OUTPUT( " : Device creation failed\n" );
		return hr;
	}
	
	
	// Get backbuffer and create a render target view of it for rendering
    ID3D11Texture2D* pBackBuffer = NULL;
    hr = m_pSwapChain->GetBuffer( 0, __uuidof( ID3D11Texture2D ), ( LPVOID* )&pBackBuffer );
    if( FAILED( hr ) )
	{
		NGERROR( "Error getting backbuffer. Renderer initialization failed." )
        return hr;
	}

    hr = m_pDevice->CreateRenderTargetView( pBackBuffer, NULL, &m_pRenderTargetView );
    pBackBuffer->Release();
    if( FAILED( hr ) )
	{
		NGERROR( "Error creating RenderTargetView for the backbuffer. Renderer initialization failed." )
        return hr;
	}

    // Create depth stencil texture for the back buffer
    D3D11_TEXTURE2D_DESC descDepth;
    ZeroMemory( &descDepth, sizeof(descDepth) );
    descDepth.Width = creationConfig.Width;
    descDepth.Height = creationConfig.Height;
    descDepth.MipLevels = 1;
    descDepth.ArraySize = 1;
    descDepth.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    descDepth.SampleDesc.Count = AACount;
    descDepth.SampleDesc.Quality = AAQuality;
    descDepth.Usage = D3D11_USAGE_DEFAULT;
    descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    descDepth.CPUAccessFlags = 0;
    descDepth.MiscFlags = 0;
    hr = m_pDevice->CreateTexture2D( &descDepth, NULL, &m_pDepthStencil );
    if( FAILED( hr ) )
        return hr;

    // Create the depth stencil view
    D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
    ZeroMemory( &descDSV, sizeof(descDSV) );
    descDSV.Format = descDepth.Format;
    descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	if( creationConfig.BackbufferMultisampling!= MSAA_NONE )
		descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;
    descDSV.Texture2D.MipSlice = 0;
    hr = m_pDevice->CreateDepthStencilView( m_pDepthStencil, &descDSV, &m_pDepthStencilView );
    if( FAILED( hr ) )
        return hr;

    m_pImmediateContext->OMSetRenderTargets( 1, &m_pRenderTargetView, m_pDepthStencilView );

    // Setup the main viewport
	m_Width = creationConfig.Width;
	m_Height = creationConfig.Height;
    D3D11_VIEWPORT vp;
    vp.Width = (FLOAT)creationConfig.Width;
    vp.Height = (FLOAT)creationConfig.Height;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    m_pImmediateContext->RSSetViewports( 1, &vp );

	DX11Texture2D *pBackbufferTexture = new DX11Texture2D();
	pBackbufferTexture->setDimensions( m_Config.Height, m_Config.Width );
	pBackbufferTexture->setFormat( R8G8B8A8_UNORM );
	pBackbufferTexture->setMipLevels( 1 );
	pBackbufferTexture->setRenderTargetView( m_pRenderTargetView );
	pBackbufferTexture->setDepthStencilView( m_pDepthStencilView );
	setBackbufferTexture( pBackbufferTexture );

	// Create default sampler states
	// Point sampler
    D3D11_SAMPLER_DESC sampDesc;
    ZeroMemory( &sampDesc, sizeof(sampDesc) );
    sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    sampDesc.MinLOD = 0;
    sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
    hr = getDevice()->CreateSamplerState( &sampDesc, &m_pPointSampler );
	if( FAILED( hr ) )
        return hr;

	// Linear sampler
    ZeroMemory( &sampDesc, sizeof(sampDesc) );
    sampDesc.Filter = D3D11_FILTER_ANISOTROPIC; //D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    sampDesc.MinLOD = 0;
    sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	sampDesc.MaxAnisotropy = creationConfig.MaxAnisotropy;
    hr = getDevice()->CreateSamplerState( &sampDesc, &m_pLinearSampler );
	if( FAILED( hr ) )
        return hr;

	// Comparison sampler
	ZeroMemory( &sampDesc, sizeof(sampDesc) );
    sampDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampDesc.ComparisonFunc = D3D11_COMPARISON_LESS;
    sampDesc.MinLOD = 0;
    sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
    hr = getDevice()->CreateSamplerState( &sampDesc, &m_pComparisonSampler );
	if( FAILED( hr ) )
        return hr;

	// Linear clamp sampler
	ZeroMemory( &sampDesc, sizeof(sampDesc) );
    sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    sampDesc.MinLOD = 0;
    sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
    hr = getDevice()->CreateSamplerState( &sampDesc, &m_pLinearClampSampler );
	if( FAILED( hr ) )
        return hr;

	// Set the samplers
	getImmediateContext()->VSSetSamplers( 0, 1, &m_pPointSampler );
	getImmediateContext()->GSSetSamplers( 0, 1, &m_pPointSampler );
	getImmediateContext()->PSSetSamplers( 0, 1, &m_pPointSampler );

	getImmediateContext()->VSSetSamplers( 1, 1, &m_pLinearSampler );
	getImmediateContext()->GSSetSamplers( 1, 1, &m_pLinearSampler );
	getImmediateContext()->PSSetSamplers( 1, 1, &m_pLinearSampler );

	getImmediateContext()->VSSetSamplers( 2, 1, &m_pComparisonSampler );
	getImmediateContext()->GSSetSamplers( 2, 1, &m_pComparisonSampler );
	getImmediateContext()->PSSetSamplers( 2, 1, &m_pComparisonSampler );

	getImmediateContext()->VSSetSamplers( 3, 1, &m_pLinearClampSampler );
	getImmediateContext()->GSSetSamplers( 3, 1, &m_pLinearClampSampler );
	getImmediateContext()->PSSetSamplers( 3, 1, &m_pLinearClampSampler );

	// Default render state
	getImmediateContext()->RSSetState( NULL );

	// All good
	return S_OK;
}

void DX11RenderDispatcher::setOutputWindow( HWND hWnd, UINT width, UINT height )
{
	HRESULT hr = NULL;

	// Release the current swap chain
	m_pSwapChain->Release();

	m_Config.Width = width;
	m_Config.Height = height;
	m_Config.OutputWindow = hWnd;

	// Determine antialiasing sample count & sample quality
	UINT AACount, AAQuality;
	switch( m_Config.BackbufferMultisampling )
	{
	case MSAA_NONE :
		AACount = 1;
		AAQuality = 0; break;
	case MSAA_2X :
		AACount = 2;
		AAQuality = 0; break;
	case MSAA_4X :
		AACount = 4;
		AAQuality = 0; break;
	case MSAA_8X :
		AACount = 8;
		AAQuality = 0; break;
	case MSAA_16X :
		AACount = 16;
		AAQuality = 0; break;
	case CSAA_8X :
		AACount = 8;
		AAQuality = 4; break;
	case CSAA_8XQ :
		AACount = 8;
		AAQuality = 8; break;
	case CSAA_16X :
		AACount = 16;
		AAQuality = 4; break;
	case CSAA_16XQ :
		AACount = 16;
		AAQuality = 8; break;
	default :
		AACount = 1;
		AAQuality = 0;
	}
	
	// Describe new swap chain
    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory( &sd, sizeof( sd ) );
    sd.BufferCount = 1;
    sd.BufferDesc.Width = width;
    sd.BufferDesc.Height = height;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = m_Config.RefreshRate;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = hWnd;
    sd.SampleDesc.Count = AACount;
    sd.SampleDesc.Quality = AAQuality;
    sd.Windowed = true;

	// Create the new swap chain
	IDXGIFactory* factory;
	CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&factory);
	factory->CreateSwapChain( m_pDevice, &sd, &m_pSwapChain );
	factory->Release();

	// Get backbuffer and create a render target view of it for rendering
    ID3D11Texture2D* pBackBuffer = NULL;
    hr = m_pSwapChain->GetBuffer( 0, __uuidof( ID3D11Texture2D ), ( LPVOID* )&pBackBuffer );
    if( FAILED( hr ) )
	{
		NGERROR( "Error getting backbuffer. Renderer initialization failed." )
        return;
	}

    hr = m_pDevice->CreateRenderTargetView( pBackBuffer, NULL, &m_pRenderTargetView );
    pBackBuffer->Release();
    if( FAILED( hr ) )
	{
		NGERROR( "Error creating RenderTargetView for the backbuffer. Renderer initialization failed." )
        return;
	}

    // Create depth stencil texture for the back buffer
    D3D11_TEXTURE2D_DESC descDepth;
    ZeroMemory( &descDepth, sizeof(descDepth) );
    descDepth.Width = m_Config.Width;
    descDepth.Height = m_Config.Height;
    descDepth.MipLevels = 1;
    descDepth.ArraySize = 1;
    descDepth.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    descDepth.SampleDesc.Count = AACount;
    descDepth.SampleDesc.Quality = AAQuality;
    descDepth.Usage = D3D11_USAGE_DEFAULT;
    descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    descDepth.CPUAccessFlags = 0;
    descDepth.MiscFlags = 0;
    hr = m_pDevice->CreateTexture2D( &descDepth, NULL, &m_pDepthStencil );
    if( FAILED( hr ) )
	{
		NGERROR( "Error creating Depth Stencil for the backbuffer. Renderer initialization failed." )
        return;
	}

    // Create the depth stencil view
    D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
    ZeroMemory( &descDSV, sizeof(descDSV) );
    descDSV.Format = descDepth.Format;
    descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	if( m_Config.BackbufferMultisampling!= MSAA_NONE )
		descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;
    descDSV.Texture2D.MipSlice = 0;
    hr = m_pDevice->CreateDepthStencilView( m_pDepthStencil, &descDSV, &m_pDepthStencilView );
    if( FAILED( hr ) )
    {
		NGERROR( "Error creating Depth Stencil View for the backbuffer. Renderer initialization failed." )
        return;
	}

	// This doesn't actually work yet lol
	//m_pBackbuffer->release();
	m_pBackbuffer->setDimensions( height, width );
	m_pBackbuffer->setFormat( R8G8B8A8_UNORM );
}

void DX11RenderDispatcher::bindShaderParams( ShaderParamBlock *pParamBlock )
{
		Timer timer;
		// Get matrix, vector, and scalar type parameters
		std::map< std::pair< std::string, UINT > , XMFLOAT4X4, std::less<std::pair< std::string, UINT >> , tbb::scalable_allocator<std::pair<std::pair< std::string, UINT >,XMFLOAT4X4>> > *pMatrixParams = pParamBlock->getMatrixParams();
		std::map< std::pair< std::string, UINT > , XMFLOAT4 , std::less<std::pair< std::string, UINT >>, tbb::scalable_allocator<std::pair<std::pair< std::string, UINT >,XMFLOAT4>> > *pVectorParams = pParamBlock->getVectorParams();
		std::map< std::pair< std::string, UINT > , float , std::less<std::pair< std::string, UINT >>, tbb::scalable_allocator<std::pair<std::pair< std::string, UINT >,float>> > *pScalarParams = pParamBlock->getScalarParams();

		int i;

		// Data will be mapped to temporary buffers before mapped to gpu.
		// First we make sure enough space is allocated for our temporary buffers.
		UINT size;
		for (i=0; i<getActiveShaderset()->numVSConstantBuffers;i++)
		{
			size = getActiveShaderset()->getVSConstantBufferSize(i);
			if (!m_pVSCBDataBuffer[i])
			{
				m_pVSCBDataBuffer[i] = new BYTE[size];
				m_pVSCBDataBufferSize[i] = size;
			}
			else if (m_pVSCBDataBufferSize[i]<size)
			{
				delete[] m_pVSCBDataBuffer[i];
				m_pVSCBDataBuffer[i] = new BYTE[size];
				m_pVSCBDataBufferSize[i] = size;
			}

			ZeroMemory(m_pVSCBDataBuffer[i], sizeof( BYTE )*size);
		}

		for (i=0; i<getActiveShaderset()->numPSConstantBuffers;i++)
		{
			size = getActiveShaderset()->getPSConstantBufferSize(i);
			if (!m_pPSCBDataBuffer[i])
			{
				m_pPSCBDataBuffer[i] = new BYTE[size];
				m_pPSCBDataBufferSize[i] = size;
			}
			else if (m_pPSCBDataBufferSize[i]<size)
			{
				delete[] m_pPSCBDataBuffer[i];
				m_pPSCBDataBuffer[i] = new BYTE[size];
				m_pPSCBDataBufferSize[i] = size;
			}

			ZeroMemory(m_pPSCBDataBuffer[i], sizeof( BYTE )*size);
		}
		
		
		const ShaderVariable* pvar = NULL;

		// Set Matrix type parameters
		//	TODO :	-different types
		if ( !pMatrixParams->empty() )
		{
			for( std::map< std::pair< std::string, UINT > , XMFLOAT4X4, std::less<std::pair< std::string, UINT >> , 
				tbb::scalable_allocator<std::pair<std::pair< std::string, UINT >,XMFLOAT4X4>> >
				::const_iterator it=pMatrixParams->begin(); it!=pMatrixParams->end(); ++it )
			{
				pvar = getActiveShaderset()->getVSVariable( it->first.first );
				if(pvar)
				{
					if ( it->first.second < pvar->Elements )
						memcpy( (m_pVSCBDataBuffer[pvar->ConstantBuffer])+pvar->Offset+(it->first.second*sizeof(XMFLOAT4X4)), 
						&it->second, 
						min( pvar->Size - (it->first.second*sizeof(XMFLOAT4X4)), sizeof(XMFLOAT4X4)) );
					else
					{
						DEBUG_OUTPUT( __FUNCTION__ );
						DEBUG_OUTPUT( " : '" );
						DEBUG_OUTPUT( it->first.first.c_str() );
						DEBUG_OUTPUT( "' element '" );
						char str[10];
						_itoa_s(it->first.second, str, 10);
						DEBUG_OUTPUT( str );
						DEBUG_OUTPUT( "' : array out of bounds.\n" );
					}
				}

				pvar = getActiveShaderset()->getPSVariable( it->first.first );
				if(pvar)
				{
					if ( it->first.second < pvar->Elements )
						memcpy( (m_pPSCBDataBuffer[pvar->ConstantBuffer])+pvar->Offset+(it->first.second*sizeof(XMFLOAT4X4)), 
						&it->second, 
						min( pvar->Size - (it->first.second*sizeof(XMFLOAT4X4)), sizeof(XMFLOAT4X4)) );
					else
					{
						DEBUG_OUTPUT( __FUNCTION__ );
						DEBUG_OUTPUT( " : '" );
						DEBUG_OUTPUT( it->first.first.c_str() );
						DEBUG_OUTPUT( "' element '" );
						char str[10];
						_itoa_s(it->first.second, str, 10);
						DEBUG_OUTPUT( str );
						DEBUG_OUTPUT( "' : array out of bounds.\n" );
					}
				}
			}
		}
		
		// Set Vector type parameters
		//	TODO :	-different types
		if ( !pVectorParams->empty() )
		{
			for( std::map< std::pair< std::string, UINT > , XMFLOAT4 , std::less<std::pair< std::string, UINT >>, 
				tbb::scalable_allocator<std::pair<std::pair< std::string, UINT >,XMFLOAT4>> >
				::const_iterator it=pVectorParams->begin(); it!=pVectorParams->end(); ++it )
			{
				pvar = getActiveShaderset()->getVSVariable( it->first.first );
				if(pvar)
				{
					if ( it->first.second < pvar->Elements )
						memcpy( (void*)(m_pVSCBDataBuffer[pvar->ConstantBuffer]+pvar->Offset+(it->first.second*sizeof(XMFLOAT4))), 
						&it->second, 
						min( pvar->Size - (it->first.second*sizeof(XMFLOAT4)), sizeof(XMFLOAT4)) );
					else
					{
						DEBUG_OUTPUT( __FUNCTION__ );
						DEBUG_OUTPUT( " : '" );
						DEBUG_OUTPUT( it->first.first.c_str() );
						DEBUG_OUTPUT( "' element '" );
						char str[10];
						_itoa_s(it->first.second, str, 10);
						DEBUG_OUTPUT( str );
						DEBUG_OUTPUT( "' : array out of bounds.\n" );
					}
				}

				pvar = getActiveShaderset()->getPSVariable( it->first.first );
				if(pvar)
				{
					if ( it->first.second < pvar->Elements )
						memcpy( (void*)(m_pPSCBDataBuffer[pvar->ConstantBuffer]+pvar->Offset+(it->first.second*sizeof(XMFLOAT4))), 
						&it->second, 
						min( pvar->Size - (it->first.second*sizeof(XMFLOAT4)), sizeof(XMFLOAT4)) );
					else
					{
						DEBUG_OUTPUT( __FUNCTION__ );
						DEBUG_OUTPUT( " : '" );
						DEBUG_OUTPUT( it->first.first.c_str() );
						DEBUG_OUTPUT( "' element '" );
						char str[10];
						_itoa_s(it->first.second, str, 10);
						DEBUG_OUTPUT( str );
						DEBUG_OUTPUT( "' : array out of bounds.\n" );
					}
				}
			}
		}
		
		// Set Scalar type parameters
		//	TODO :	different types
		//
		if ( !pScalarParams->empty() )
		{
			for( std::map< std::pair< std::string, UINT > , float , std::less<std::pair< std::string, UINT >>, 
				tbb::scalable_allocator<std::pair<std::pair< std::string, UINT >,float>> >
				::const_iterator it=pScalarParams->begin(); it!=pScalarParams->end(); ++it )
			{
				pvar = getActiveShaderset()->getVSVariable( it->first.first );
				if(pvar)
				{
					if ( it->first.second < pvar->Elements )
						memcpy( (void*)(m_pVSCBDataBuffer[pvar->ConstantBuffer]+pvar->Offset+(it->first.second*sizeof(XMFLOAT4))), 
						&it->second, 
						min( pvar->Size - (it->first.second*sizeof(XMFLOAT4)), sizeof(XMFLOAT4)) );
					else
					{
						DEBUG_OUTPUT( __FUNCTION__ );
						DEBUG_OUTPUT( " : '" );
						DEBUG_OUTPUT( it->first.first.c_str() );
						DEBUG_OUTPUT( "' element '" );
						char str[10];
						_itoa_s(it->first.second, str, 10);
						DEBUG_OUTPUT( str );
						DEBUG_OUTPUT( "' : array out of bounds.\n" );
					}
				}

				pvar = getActiveShaderset()->getPSVariable( it->first.first );
				if(pvar)
				{
					if ( it->first.second < pvar->Elements )
						memcpy( (void*)(m_pPSCBDataBuffer[pvar->ConstantBuffer]+pvar->Offset+(it->first.second*sizeof(XMFLOAT4))), 
						&it->second, 
						min( pvar->Size - (it->first.second*sizeof(XMFLOAT4)), sizeof(XMFLOAT4)) );
					else
					{
						DEBUG_OUTPUT( __FUNCTION__ );
						DEBUG_OUTPUT( " : '" );
						DEBUG_OUTPUT( it->first.first.c_str() );
						DEBUG_OUTPUT( "' element '" );
						char str[10];
						_itoa_s(it->first.second, str, 10);
						DEBUG_OUTPUT( str );
						DEBUG_OUTPUT( "' : array out of bounds.\n" );
					}
				}
			}
		}

		/*
		float *temp = reinterpret_cast<float*>(m_pVSCBDataBuffer[1]);
		float test[100];
		ZeroMemory( test, sizeof(float)*1000 );
		for (i=0; i<m_pVSCBDataBufferSize[1]/sizeof(float); i++)
		{
			temp = reinterpret_cast<float*>(m_pVSCBDataBuffer[1]+i*sizeof(float));
			test[i] = *temp;
			temp++;
		}
		*/

		// Mark all constant buffers as unused 
		for (std::map<UINT, std::vector<InternalCB*>>::iterator it = m_pConstantBuffers.begin(); it!=m_pConstantBuffers.end(); ++it)
		{
			std::vector<InternalCB*>* pCBs = &it->second;
			for (std::vector<InternalCB*>::iterator it = pCBs->begin(); it != pCBs->end(); ++it)
			{
				(*it)->InUse = false;
			}
		}

		t_1 += timer.getMiliseconds();
		timer.start();

		// Send to GPU
		// TODO : -only map updated buffers
		//        -batch updates (reduce api calls)
		D3D11_MAPPED_SUBRESOURCE subresource;

		// Vertex shader
		for (i=0; i<getActiveShaderset()->numVSConstantBuffers;i++)
		{
			// Get constant buffer of the appropriate size
			InternalCB* pVSCB = getAvailableConstantBuffer( m_pVSCBDataBufferSize[i] );
			if (pVSCB)
			{
				// Map data to gpu and bind to pipeline
				pVSCB->InUse = true;
				getImmediateContext()->Map( pVSCB->pBuffer, 0, D3D11_MAP_WRITE_DISCARD, NULL, &subresource );
				PBYTE cbdata = (PBYTE)subresource.pData;
				memcpy( cbdata, m_pVSCBDataBuffer[i], pVSCB->ByteSize );
				getImmediateContext()->Unmap( pVSCB->pBuffer, 0 );

				getImmediateContext()->VSSetConstantBuffers( i, 1, &pVSCB->pBuffer );
			}
			else
			{
				char str[10];
				_itoa_s((UINT)ceil(float(m_pVSCBDataBufferSize[i]/16.0f))*16,str, 10);
				DEBUG_OUTPUT( __FUNCTION__ );
				DEBUG_OUTPUT( " : Internal engine error. (Vertex Shader) Could not get Internal Constant Buffer of size " );
				DEBUG_OUTPUT( str );
				DEBUG_OUTPUT( " bytes (slot #" );
				_itoa_s(i,str, 10);
				DEBUG_OUTPUT( str );
				DEBUG_OUTPUT( ") (invalid or NULL), data could not be updated.\n" );
			}
		}

		// Pixel shader
		for (i=0; i<getActiveShaderset()->numPSConstantBuffers;i++)
		{
			// Get constant buffer of the appropriate size
			InternalCB* pPSCB = getAvailableConstantBuffer( m_pPSCBDataBufferSize[i] );
			if (pPSCB)
			{
				// Map data to gpu and bind to pipeline
				pPSCB->InUse = true;
				getImmediateContext()->Map( pPSCB->pBuffer, 0, D3D11_MAP_WRITE_DISCARD, NULL, &subresource );
				PBYTE cbdata = (PBYTE)subresource.pData;
				memcpy( cbdata, m_pPSCBDataBuffer[i], pPSCB->ByteSize );
				getImmediateContext()->Unmap( pPSCB->pBuffer, 0 );

				getImmediateContext()->PSSetConstantBuffers( i, 1, &pPSCB->pBuffer );
			}
			else
			{
				char str[10];
				_itoa_s((UINT)ceil(float(m_pPSCBDataBufferSize[i]/16.0f))*16,str, 10);
				DEBUG_OUTPUT( __FUNCTION__ );
				DEBUG_OUTPUT( " : Internal engine error. (Pixel Shader) Could not get Internal Constant Buffer of size " );
				DEBUG_OUTPUT( str );
				DEBUG_OUTPUT( " bytes (slot #" );
				_itoa_s(i,str, 10);
				DEBUG_OUTPUT( str );
				DEBUG_OUTPUT( ") (invalid or NULL), data could not be updated.\n" );
			}
		}
		
		t_2 += timer.getMiliseconds();
}

DX11RenderDispatcher::InternalCB* DX11RenderDispatcher::getAvailableConstantBuffer( UINT ByteSize )
{
	// Constant buffers are always muliples of 16 bytes
	UINT CBSize = (UINT)ceil(float(ByteSize/16.0f))*16;
	
	// Find the constant buffer with the requested size
	std::map<UINT, std::vector<InternalCB*>>::iterator cbvector = m_pConstantBuffers.find( CBSize );
	if (cbvector!=m_pConstantBuffers.end())
	{
		std::vector<InternalCB*>* pCBs = &cbvector->second;
		for (std::vector<InternalCB*>::iterator it = pCBs->begin(); it != pCBs->end(); ++it)
		{
			if (!(*it)->InUse)
				return *it;
		}
	}

	// If not found, create new one
	InternalCB* cb = new InternalCB();
	cb->ByteSize = CBSize;
	cb->InUse = false;

	D3D11_BUFFER_DESC bd;
	ZeroMemory( &bd, sizeof(bd) );
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = CBSize;
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	if(FAILED(getDevice()->CreateBuffer( &bd, NULL, &cb->pBuffer )))
	{
		delete cb;
		return NULL;
	}

	m_pConstantBuffers[ByteSize].push_back( cb );

	return cb;
}

void DX11RenderDispatcher::bindConstantBufferVS( UINT i, ConstantBufferData* pData )
{
	D3D11_MAPPED_SUBRESOURCE subresource;

	// Get constant buffer of the appropriate size
	InternalCB* pVSCB = getAvailableConstantBuffer( pData->getByteSize() );
	if (pVSCB)
	{
		// Map data to gpu and bind to pipeline
		pVSCB->InUse = true;
		getImmediateContext()->Map( pVSCB->pBuffer, 0, D3D11_MAP_WRITE_DISCARD, NULL, &subresource );
		PBYTE cbdata = (PBYTE)subresource.pData;
		memcpy( cbdata, pData->getData(), pData->getByteSize() );
		getImmediateContext()->Unmap( pVSCB->pBuffer, 0 );

		getImmediateContext()->VSSetConstantBuffers( i, 1, &pVSCB->pBuffer );
	}
	else
	{
		char str[10];
		_itoa_s((UINT)ceil(float(pData->getByteSize()/16.0f))*16,str, 10);
		DEBUG_OUTPUT( __FUNCTION__ );
		DEBUG_OUTPUT( " : Internal engine error. (Vertex Shader) Could not get Internal Constant Buffer of size " );
		DEBUG_OUTPUT( str );
		DEBUG_OUTPUT( " bytes (slot #" );
		_itoa_s(i,str, 10);
		DEBUG_OUTPUT( str );
		DEBUG_OUTPUT( ") (invalid or NULL), constant buffer data could not be updated.\n" );
	}
}

void DX11RenderDispatcher::bindConstantBufferPS( UINT i, ConstantBufferData* pData )
{
	D3D11_MAPPED_SUBRESOURCE subresource;

	// Get constant buffer of the appropriate size
	InternalCB* pPSCB = getAvailableConstantBuffer( pData->getByteSize() );
	if (pPSCB)
	{
		// Map data to gpu and bind to pipeline
		pPSCB->InUse = true;
		getImmediateContext()->Map( pPSCB->pBuffer, 0, D3D11_MAP_WRITE_DISCARD, NULL, &subresource );
		PBYTE cbdata = (PBYTE)subresource.pData;
		memcpy( cbdata, pData->getData(), pData->getByteSize() );
		getImmediateContext()->Unmap( pPSCB->pBuffer, 0 );

		getImmediateContext()->PSSetConstantBuffers( i, 1, &pPSCB->pBuffer );
	}
	else
	{
		char str[10];
		_itoa_s((UINT)ceil(float(pData->getByteSize()/16.0f))*16,str, 10);
		DEBUG_OUTPUT( __FUNCTION__ );
		DEBUG_OUTPUT( " : Internal engine error. (Pixel Shader) Could not get Internal Constant Buffer of size " );
		DEBUG_OUTPUT( str );
		DEBUG_OUTPUT( " bytes (slot #" );
		_itoa_s(i,str, 10);
		DEBUG_OUTPUT( str );
		DEBUG_OUTPUT( ") (invalid or NULL), constant buffer data could not be updated.\n" );
	}
}

void DX11RenderDispatcher::bindShaderParams( ShaderParams *pParams )
{
	if (pParams==NULL)
	{
		DEBUG_OUTPUT( __FUNCTION__ );
		DEBUG_OUTPUT( " : ShaderParam is NULL.\n" );
		return;
	}

	// Mark all constant buffers as unused
	for (std::map<UINT, std::vector<InternalCB*>>::iterator it = m_pConstantBuffers.begin(); it!=m_pConstantBuffers.end(); ++it)
	{
		std::vector<InternalCB*>* pCBs = &it->second;
		for (std::vector<InternalCB*>::iterator it = pCBs->begin(); it != pCBs->end(); ++it)
		{
			(*it)->InUse = false;
		}
	}

	// Bind
	for (int i=0; i<pParams->getNumConstantBuffersVS(); i++)
	{
		bindConstantBufferVS(i, pParams->getVSConstantBuffer(i));
	}
	for (int i=0; i<pParams->getNumConstantBuffersPS(); i++)
	{
		bindConstantBufferPS(i, pParams->getPSConstantBuffer(i));
	}
}

void DX11RenderDispatcher::setBackbufferAsRenderTarget()
{
	m_pImmediateContext->OMSetRenderTargets( 1, &m_pRenderTargetView, m_pDepthStencilView );
   
	D3D11_VIEWPORT vp;
    vp.Width = (FLOAT)m_Width;
    vp.Height = (FLOAT)m_Height;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    m_pImmediateContext->RSSetViewports( 1, &vp );
}


////////////////////////////
// Resource creation
////////////////////////////
GeometryChunk*	DX11RenderDispatcher::createGeometryChunk( float* vertices, UINT stride, UINT byteWidth, BufferLayout layout, UINT* indices, UINT numIndices, bool dynamic, D3D_PRIMITIVE_TOPOLOGY topology)
{
	// Create DX11 buffers
	D3D11VertexBuffer*	pVertexBuffer = createVertexBuffer( vertices, byteWidth, layout, stride, dynamic );
	D3D11IndexBuffer*	pIndexBuffer = createIndexBuffer( indices, numIndices );
	
	if (!pVertexBuffer)
	{
		DEBUG_OUTPUT( __FUNCTION__ );
		DEBUG_OUTPUT( " : Failed creating vertex buffer. Returning NULL.\n" );
		return NULL;
	}
	if (!pIndexBuffer)
	{
		DEBUG_OUTPUT( __FUNCTION__ );
		DEBUG_OUTPUT( " : Failed creating index buffer. Returning NULL.\n" );
		return NULL;
	}
	
	// Compute bounding box
	XNA::AxisAlignedBox AABB;
	XNA::ComputeBoundingAxisAlignedBoxFromPoints( &AABB, (byteWidth/stride), (XMFLOAT3*)vertices, stride ); 
	
	D3D11GeometryChunk*	pChunk = new D3D11GeometryChunk();
	pChunk->addVertexBuffer( pVertexBuffer );
	pChunk->setIndexBuffer( pIndexBuffer );
	pChunk->setPrimitiveTopology( topology );
	pChunk->setAABB( &AABB );
	pChunk->setDynamic( dynamic );

	return pChunk;
}

D3D11VertexBuffer* DX11RenderDispatcher::createVertexBuffer( const void* pData, int dataSize, BufferLayout layout, int stride, bool dynamic )
{
	ID3D11Device* pDevice = getDevice();

	// TODO : store this
	ID3D11Buffer*	pd3d11Buffer = NULL;

	// Create d3d10 vertex buffer
	D3D11_BUFFER_DESC bd;
    bd.Usage = dynamic ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_IMMUTABLE;
    bd.ByteWidth = dataSize;
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.CPUAccessFlags = dynamic ? D3D11_CPU_ACCESS_WRITE : 0;
    bd.MiscFlags = 0;
    D3D11_SUBRESOURCE_DATA InitData;
    InitData.pSysMem = pData;
	if	(pDevice)
    if (FAILED( pDevice->CreateBuffer( &bd, &InitData, &pd3d11Buffer ) ))
	{
		DEBUG_OUTPUT( __FUNCTION__ );
		DEBUG_OUTPUT( " : Failed creating D3D11 buffer. Returning NULL.\n" );
		return NULL;
	}

	D3D11VertexBuffer*	pBuffer = new D3D11VertexBuffer( pd3d11Buffer, layout );
	pBuffer->setStride( stride );

	return pBuffer;
}

D3D11IndexBuffer* DX11RenderDispatcher::createIndexBuffer( UINT* pData, int numOfIndices )
{
	ID3D11Device* pDevice = getDevice();

	// TODO : store this
	ID3D11Buffer*	pd3d11Buffer = NULL;

	// Create d3d11 index buffer
	D3D11_BUFFER_DESC bd;
    bd.Usage = D3D11_USAGE_IMMUTABLE;
    bd.ByteWidth = sizeof( UINT ) * numOfIndices;
    bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    bd.CPUAccessFlags = 0;
    bd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA InitData;
    InitData.pSysMem = pData;
	if	(pDevice)
    if (FAILED( pDevice->CreateBuffer( &bd, &InitData, &pd3d11Buffer ) ))
	{
		DEBUG_OUTPUT( __FUNCTION__ );
		DEBUG_OUTPUT( " : Failed creating D3D11 buffer. Returning NULL.\n" );
		return NULL;
	}

	D3D11IndexBuffer*	pBuffer = new D3D11IndexBuffer( pd3d11Buffer );
	pBuffer->setNumberOfIndices(numOfIndices);
	return pBuffer;
}

Shaderset*	DX11RenderDispatcher::createShadersetFromFile( std::wstring filename, std::string vertexShader, std::string pixelShader, SHADERMODEL sm, std::vector<ShaderMacro> macros, bool debug )
{
	HRESULT hr = S_OK;
	D3D11Shaderset* pShaderset = new D3D11Shaderset();
	ID3D11VertexShader* pVertexShader = NULL;
	ID3D11PixelShader*  pPixelShader = NULL;

    DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
	if (debug)
		dwShaderFlags |= D3DCOMPILE_DEBUG;

	ID3DBlob* pVSBlob, *pPSBlob, *pErrorBlob;
	std::string shaderProfile;

	if (!getDevice())
	{
		DEBUG_OUTPUT( __FUNCTION__ );
		DEBUG_OUTPUT( " : D3D11 device invalid or uninitialized. Returning NULL.\n" );
		return NULL;
	}

	// Shader macros
	D3D10_SHADER_MACRO *macroDefines;
	std::vector<D3D10_SHADER_MACRO> macrosVector;
	if (!macros.empty())
	{
		for( std::vector<ShaderMacro>::size_type i = 0; i < macros.size(); ++i )
		{
			D3D10_SHADER_MACRO macro;
			macro.Name = macros[i].Name.c_str();
			macro.Definition = macros[i].Definition.c_str();
			macrosVector.push_back(macro);
		}
		// Null-terminated array
		D3D10_SHADER_MACRO macro;
		macro.Name = NULL;
		macro.Definition = NULL;
		macrosVector.push_back( macro );

		macroDefines = &macrosVector[0];
	}
	else
		macroDefines = NULL;

	/////////////////////////////////////////////////////////
	// Vertex Shader
	/////////////////////////////////////////////////////////

	if (sm==SM_4_0) shaderProfile = "vs_4_0";
	else if (sm==SM_5_0) shaderProfile = "vs_5_0";
	else if (sm==SM_AUTO) shaderProfile = "vs_5_0";

	// Compile the vertex shader
    hr = D3DX11CompileFromFile( filename.c_str(), macroDefines, NULL, vertexShader.c_str(), shaderProfile.c_str(), 
        dwShaderFlags, 0, NULL, &pVSBlob, &pErrorBlob, NULL );

    if( FAILED(hr) )
    {
		DEBUG_OUTPUT( __FUNCTION__ );
		DEBUG_OUTPUT( " : " );
		DEBUG_OUTPUTW( filename.c_str() );
		DEBUG_OUTPUT( " : failed compiling vertex shader. Returning NULL.\nError blob :\n" );
        if( pErrorBlob != NULL )
		{
			// Print out the error
            DEBUG_OUTPUT( (char*)pErrorBlob->GetBufferPointer() );
			pErrorBlob->Release();
		}
        return NULL;
    }

    if( pErrorBlob ) pErrorBlob->Release();
    
    // Create the vertex shader object
    hr = getDevice()->CreateVertexShader( pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), NULL, &pVertexShader );
    if( FAILED( hr ) )
    {    
        pVSBlob->Release();
		DEBUG_OUTPUT( __FUNCTION__ );
		DEBUG_OUTPUT( " : " );
		DEBUG_OUTPUTW( filename.c_str() );
		DEBUG_OUTPUT( " : failed creating vertex shader object from compiled shader. Returning NULL.\n" );
        return NULL;
    }

	pShaderset->setVertexShader( pVertexShader );

	// Create Vertex shader reflection
	ID3D11ShaderReflection* pShaderReflection = NULL;
	hr = D3DReflect( pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), IID_ID3D11ShaderReflection, (void**) &pShaderReflection );
	//pVSBlob->Release();
	pShaderset->setInputSignature( pVSBlob );

	if( FAILED( hr ) )
        return NULL;
	
	D3D11_SHADER_DESC desc1;
	D3D11_SHADER_INPUT_BIND_DESC desc2;
	D3D11_SHADER_BUFFER_DESC desc3;
	D3D11_SHADER_VARIABLE_DESC desc4;
	D3D11_SHADER_TYPE_DESC desc5;
	ZeroMemory( &desc1, sizeof( D3D11_SHADER_DESC ) );
	ZeroMemory( &desc2, sizeof( D3D11_SHADER_INPUT_BIND_DESC ) );
	ZeroMemory( &desc3, sizeof( D3D11_SHADER_BUFFER_DESC ) );
	ZeroMemory( &desc4, sizeof( D3D11_SHADER_VARIABLE_DESC ) );
	ZeroMemory( &desc5, sizeof( D3D11_SHADER_TYPE_DESC ) );
	pShaderReflection->GetDesc( &desc1 );

	// Get indices of texture bindings
	int i=0, i1=0, i2=0;
	for (i=0; i<desc1.BoundResources; i++)
	{
		pShaderReflection->GetResourceBindingDesc( i, &desc2 );
		// Map resource name to its index
		if (desc2.Type==D3D_SIT_TEXTURE)	pShaderset->setVSShaderResourceIndex( desc2.Name, desc2.BindPoint );
	}

	int ii=0;
	ID3D11ShaderReflectionConstantBuffer* pShaderReflectionCB = NULL;
	ID3D11ShaderReflectionVariable* pShaderReflectionVariable = NULL;
	ID3D11ShaderReflectionType*	pShaderReflectionType = NULL;

	/*	Get all shader variables in the shader and set a ShaderVariable struct for each
		of them, which contains the name of the variable (in the shader), index of the 
		constant buffer it belongs to, size, type, and the number of array elements it has
		(1 if it's not an array). This information is used by the ShaderParam class to map 
		data to the appropriate constant buffer & location within that constant buffer,
		for the RenderDispatcher to then send to GPU for rendering.
	*/
	pShaderset->numVSConstantBuffers = desc1.ConstantBuffers;
	for (i=0; i<desc1.ConstantBuffers; i++)
	{
		pShaderReflectionCB = pShaderReflection->GetConstantBufferByIndex( i );
		pShaderReflectionCB->GetDesc( &desc3 );
		pShaderset->setVSConstantBufferSize( i, desc3.Size );
		
		for (ii=0; ii<desc3.Variables; ii++)
		{
			pShaderReflectionVariable = pShaderReflectionCB->GetVariableByIndex( ii );
			pShaderReflectionVariable->GetDesc( &desc4 );
			pShaderReflectionVariable->GetType()->GetDesc( &desc5 );

			// TODO :	variable type
			ShaderVariable var;
			var.ConstantBuffer = i;
			var.Name = desc4.Name;
			var.Size = desc4.Size;
			var.Offset = desc4.StartOffset;
			var.Elements = ((desc5.Elements>0) ? desc5.Elements : 1);

			pShaderset->setVSVariable( desc4.Name, var );
		}
	}
	
	pShaderReflection->Release();

	///////////////////////////////////////////////////////////
	// Pixel Shader
	///////////////////////////////////////////////////////////

	if (sm==SM_4_0) shaderProfile = "ps_4_0";
	else if (sm==SM_5_0) shaderProfile = "ps_5_0";
	else if (sm==SM_AUTO) shaderProfile = "ps_5_0";

	// Compile the pixel shader
    hr = D3DX11CompileFromFile( filename.c_str(), macroDefines, NULL, pixelShader.c_str(), shaderProfile.c_str(), 
        dwShaderFlags, 0, NULL, &pPSBlob, &pErrorBlob, NULL );
    if( FAILED(hr) )
    {
		DEBUG_OUTPUT( __FUNCTION__ );
		DEBUG_OUTPUT( " : " );
		DEBUG_OUTPUTW( filename.c_str() );
		DEBUG_OUTPUT( " : failed compiling pixel shader. Returning NULL.\n" );
        if( pErrorBlob != NULL )
		{
            DEBUG_OUTPUT( (char*)pErrorBlob->GetBufferPointer() );
			pErrorBlob->Release();
		}
        return NULL;
    }
    if( pErrorBlob ) pErrorBlob->Release();

    // Create the pixel shader object
    hr = getDevice()->CreatePixelShader( pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &pPixelShader );
    if( FAILED( hr ) )
	{
		pPixelShader->Release();
		DEBUG_OUTPUT( __FUNCTION__ );
		DEBUG_OUTPUT( " : " );
		DEBUG_OUTPUTW( filename.c_str() );
		DEBUG_OUTPUT( " : failed creating pixel shader object from compiled shader. Returning NULL.\n" );
        return NULL;
	}

	pShaderset->setPixelShader( pPixelShader );

	// Create pixel shader reflection
	pShaderReflection = NULL;
	hr = D3DReflect( pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), IID_ID3D11ShaderReflection, (void**) &pShaderReflection );
	pPSBlob->Release();	
	
	if( FAILED( hr ) )
        return NULL;

	ZeroMemory( &desc1, sizeof( D3D11_SHADER_DESC ) );
	ZeroMemory( &desc2, sizeof( D3D11_SHADER_INPUT_BIND_DESC ) );
	ZeroMemory( &desc3, sizeof( D3D11_SHADER_BUFFER_DESC ) );
	ZeroMemory( &desc4, sizeof( D3D11_SHADER_VARIABLE_DESC ) );
	ZeroMemory( &desc5, sizeof( D3D11_SHADER_TYPE_DESC ) );
	pShaderReflection->GetDesc( &desc1 );

	// Get indices of texture bindings
	i=0; i1=0; i2=0;
	for (i=0; i<desc1.BoundResources; i++)
	{
		pShaderReflection->GetResourceBindingDesc( i, &desc2 );
		// Map resource name to its index
		if (desc2.Type==D3D_SIT_TEXTURE)	pShaderset->setPSShaderResourceIndex( desc2.Name, desc2.BindPoint );
	}

	ii=0;
	pShaderReflectionCB = NULL;
	pShaderReflectionVariable = NULL;

	/*	Get all shader variables in the shader and set a ShaderVariable struct for each
		of them, which contains the name of the variable (in the shader), index of the 
		constant buffer it belongs to, size, type, and the number of array elements it has
		(1 if it's not an array). This information is used by the ShaderParam class to map 
		data to the appropriate constant buffer & location within that constant buffer,
		for the RenderDispatcher to then send to GPU for rendering.
	*/
	pShaderset->numPSConstantBuffers = desc1.ConstantBuffers;
	for (i=0; i<desc1.ConstantBuffers; i++)
	{
		pShaderReflectionCB = pShaderReflection->GetConstantBufferByIndex( i );
		pShaderReflectionCB->GetDesc( &desc3 );
		pShaderset->setPSConstantBufferSize( i, desc3.Size );
		
		for (ii=0; ii<desc3.Variables; ii++)
		{
			ZeroMemory( &desc5, sizeof( D3D11_SHADER_TYPE_DESC ) );
			pShaderReflectionVariable = pShaderReflectionCB->GetVariableByIndex( ii );
			pShaderReflectionVariable->GetDesc( &desc4 );
			pShaderReflectionVariable->GetType()->GetDesc( &desc5 );

			// TODO :	variable type
			ShaderVariable var;
			var.ConstantBuffer = i;
			var.Name = desc4.Name;
			var.Size = desc4.Size;
			var.Offset = desc4.StartOffset;
			var.Elements = ((desc5.Elements>0) ? desc5.Elements : 1);
			//var.IsArray = (desc5.Elements>0);

			pShaderset->setPSVariable( desc4.Name, var );
		}
	}

	pShaderReflection->Release();


	return pShaderset;
}

Texture2D*	DX11RenderDispatcher::createTextureFromFile( std::wstring filename )
{
	ID3D11Device *pDevice = getDevice();
	ID3D11ShaderResourceView *pSRV = NULL;
	// Load the Texture
	HRESULT hr = D3DX11CreateShaderResourceViewFromFile( pDevice, filename.c_str(), NULL, NULL, &pSRV, NULL );
    if( FAILED( hr ) )
	{
		DEBUG_OUTPUT( __FUNCTION__ );
		DEBUG_OUTPUT( " : failed loading file : " );
		DEBUG_OUTPUTW( filename.c_str() );
		DEBUG_OUTPUT( "\n" );
        return NULL;
	}

	// Get description
	ID3D11Resource *pResource = NULL;
	pSRV->GetResource(&pResource);
	ID3D11Texture2D *pTextureInterface = 0;
	pResource->QueryInterface<ID3D11Texture2D>(&pTextureInterface);
	D3D11_TEXTURE2D_DESC desc;
	pTextureInterface->GetDesc(&desc);

	// Create the texture
	DX11Texture2D* ptex = new DX11Texture2D();
	ptex->setShaderResourceView( pSRV );
	ptex->setDimensions( desc.Height, desc.Width );
	return ptex;
}

void DX11RenderDispatcher::clearBackbuffer( float* clearColorRGBA )
{
	//
    // Clear the back buffer
    //
    getImmediateContext()->ClearRenderTargetView( m_pRenderTargetView, clearColorRGBA );
}

void DX11RenderDispatcher::clearDepth( float depth )
{
	//
    // Clear the depth buffer
    //
    getImmediateContext()->ClearDepthStencilView( m_pDepthStencilView, D3D11_CLEAR_DEPTH, depth, 0 );
}

void DX11RenderDispatcher::clearStencil( UINT8 stencil )
{
	//
    // Clear the stencil buffer
    //
    getImmediateContext()->ClearDepthStencilView( m_pDepthStencilView, D3D11_CLEAR_STENCIL, 0.0f, stencil );
}

void DX11RenderDispatcher::clearDepthStencil( float depth, UINT8 stencil )
{
	//
    // Clear the depth stencil buffer
    //
	getImmediateContext()->ClearDepthStencilView( m_pDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, depth, stencil );
}

void DX11RenderDispatcher::setRenderTarget( Texture2D* pRenderTarget )
{
	ID3D11RenderTargetView *pRTV = ((DX11Texture2D*)pRenderTarget)->getRenderTargetView();
	ID3D11DepthStencilView *pDSV = ((DX11Texture2D*)pRenderTarget)->getDepthStencilView();
	getImmediateContext()->OMSetRenderTargets( 1, &pRTV , pDSV );

	D3D11_VIEWPORT vp;
	vp.Width = (FLOAT)pRenderTarget->getWidth();
	vp.Height = (FLOAT)pRenderTarget->getHeight();
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	getImmediateContext()->RSSetViewports( 1, &vp );
}

void DX11RenderDispatcher::setMultipleRenderTargets( UINT numRenderTargets, Texture2D** pRenderTargetArray )
{
	// Get Render Target View pointers into an array
	ID3D11RenderTargetView *RTVs[8];
	for (int i=0; i<numRenderTargets; i++ )
	{
		Texture2D* tex = pRenderTargetArray[i];
		RTVs[i] = ((DX11Texture2D*)tex)->getRenderTargetView();
	}

	// The Depth Stencil View of the first texture is used
	ID3D11DepthStencilView *pDSV = ((DX11Texture2D*)pRenderTargetArray[0])->getDepthStencilView();

	getImmediateContext()->OMSetRenderTargets( numRenderTargets, RTVs , pDSV );

	// Set default viewport (the whole texture)
	D3D11_VIEWPORT vp;
	vp.Width = (FLOAT)pRenderTargetArray[0]->getWidth();
	vp.Height = (FLOAT)pRenderTargetArray[0]->getHeight();
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	getImmediateContext()->RSSetViewports( 1, &vp );
}

void DX11RenderDispatcher::resolveMSAA( Texture2D* pDestination, Texture2D* pSource )
{
	m_pImmediateContext->ResolveSubresource( ((DX11Texture2D*)pDestination)->getResource(), D3D11CalcSubresource(0,0,1), ((DX11Texture2D*)pDestination)->getResource(), 0, (DXGI_FORMAT)((DX11Texture2D*)pDestination)->getFormat() );
}

Texture2D* DX11RenderDispatcher::createTexture( UINT height, UINT width, TEXTURE_FORMAT format, const void* const data, size_t pitch, size_t dataSize )
{
	// D3D resource pointers
	ID3D11Texture2D* pMap, *pDepthStencil;
	ID3D11DepthStencilView* pDSV;
	ID3D11RenderTargetView*	pRTV;
	ID3D11ShaderResourceView* pSRV;
	HRESULT hr;

	// Texture description
	D3D11_TEXTURE2D_DESC texDesc;
	texDesc.ArraySize = 1;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	texDesc.CPUAccessFlags = NULL;
	texDesc.Format = (DXGI_FORMAT)format;
	texDesc.Height = height;
	texDesc.Width = width;
	texDesc.MipLevels = 1;
	texDesc.MiscFlags = NULL;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	
	if ( data != NULL )
	{
		// Initial data
		D3D11_SUBRESOURCE_DATA initData;
		initData.pSysMem = data;
		initData.SysMemPitch = pitch;
		initData.SysMemSlicePitch = dataSize;
		hr = this->getDevice()->CreateTexture2D( &texDesc , &initData , &pMap );
	}
	else
		hr = this->getDevice()->CreateTexture2D( &texDesc , NULL , &pMap );

	// Depth-stencil description
	texDesc.ArraySize = 1;
	texDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	texDesc.CPUAccessFlags = NULL;
	texDesc.Format = DXGI_FORMAT_D32_FLOAT;
	texDesc.Height = height;
	texDesc.Width = width;
	texDesc.MipLevels = 0;
	texDesc.MiscFlags = NULL;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	hr = this->getDevice()->CreateTexture2D( &texDesc , NULL , &pDepthStencil );

	// Create Depth Stencil View
	D3D11_DEPTH_STENCIL_VIEW_DESC DSVDesc;
	ZeroMemory( &DSVDesc, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC) );
	DSVDesc.Flags = 0;
	DSVDesc.Format = DXGI_FORMAT_D32_FLOAT;
	DSVDesc.Texture2D.MipSlice = 0;
	DSVDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	hr = this->getDevice()->CreateDepthStencilView( pDepthStencil , &DSVDesc , &pDSV );
	
	// Create Render Target View
	D3D11_RENDER_TARGET_VIEW_DESC RTVDesc;
	ZeroMemory( &RTVDesc, sizeof(D3D11_RENDER_TARGET_VIEW_DESC) );
	RTVDesc.Format = (DXGI_FORMAT)format;
	RTVDesc.Texture2D.MipSlice = 0;
	RTVDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	hr = this->getDevice()->CreateRenderTargetView( pMap , &RTVDesc , &pRTV );
	
	// Create Shader Resource View
	D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc;
	ZeroMemory( &SRVDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC) );
	SRVDesc.Format = (DXGI_FORMAT)format;
	SRVDesc.Texture2D.MipLevels = 1;
	SRVDesc.Texture2D.MostDetailedMip = 0;
	SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	hr = this->getDevice()->CreateShaderResourceView( pMap , &SRVDesc , &pSRV );

	// There are log2(N)+1 mips of an image with N as largest dimension.
	int numMips = log( (double)max(width,height) )/log((double)2.0)+1;

	// Create the object
	DX11Texture2D* pTex = new DX11Texture2D();
	pTex->setFormat( format );
	pTex->setResource( pMap );
	pTex->setShaderResourceView( pSRV );
	pTex->setRenderTargetView( pRTV );
	pTex->setDepthStencilView( pDSV );
	pTex->setDimensions( height, width );
	pTex->setMipLevels( numMips );

	return pTex;
}

Texture3D*	DX11RenderDispatcher::createTexture3D( UINT width, UINT height, UINT depth, DXGI_FORMAT format, const void* const data, size_t pitch, size_t slicePitch, size_t dataSize )
{
	ID3D11Texture3D* pMap;
	ID3D11ShaderResourceView* pSRV;
	HRESULT hr;

	D3D11_TEXTURE3D_DESC texDesc;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	texDesc.CPUAccessFlags = NULL;
	texDesc.Format = format;
	texDesc.Height = height;
	texDesc.Width = width;
	texDesc.Depth = depth;
	texDesc.MipLevels = 1;
	texDesc.MiscFlags = NULL;
	texDesc.Usage = D3D11_USAGE_DEFAULT;

	//float test[16*16*16];
	//memcpy(test,&data(0,0,0),sizeof(float)*data.height()*data.width()*data.depth());

	D3D11_SUBRESOURCE_DATA initialData;
	ZeroMemory( &initialData, sizeof(D3D11_SUBRESOURCE_DATA) );
	initialData.pSysMem = (void*)data;
	initialData.SysMemPitch = pitch;
	initialData.SysMemSlicePitch = slicePitch;

	hr = this->getDevice()->CreateTexture3D( &texDesc , &initialData , &pMap );

	D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc;
	ZeroMemory( &SRVDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC) );
	SRVDesc.Format = format;
	SRVDesc.Texture2D.MipLevels = 1;
	SRVDesc.Texture2D.MostDetailedMip = 0;
	SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE3D;
	hr = this->getDevice()->CreateShaderResourceView( pMap , &SRVDesc , &pSRV );

	DX11Texture3D* tex = new DX11Texture3D();
	tex->setResource( pMap );
	tex->setShaderResourceView( pSRV );
	tex->setDimensions( height, width, depth );

	return tex;
}

TextureCube* DX11RenderDispatcher::createCubemap( Image* const faces[6] )
{
	// Using DXGI_FORMAT_R8G8B8A8_UNORM (28) as an example.
	DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM;

	// D3D Objects to create
	ID3D11Texture2D* cubeTexture = NULL;
	ID3D11ShaderResourceView* shaderResourceView = NULL;

	// There are log2(N)+1 mips of an image with max dimension N.
	int numMips = log( (double)max(faces[0]->width(),faces[0]->height()) )/log((double)2.0)+1;

	// Cube texture description
	D3D11_TEXTURE2D_DESC texDesc;
	texDesc.Width = faces[0]->width();
	texDesc.Height = faces[0]->height();
	texDesc.MipLevels = numMips;
	texDesc.ArraySize = 6;			// 6 Cubemap faces
	texDesc.Format = format;
	texDesc.CPUAccessFlags = 0;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE | D3D11_RESOURCE_MISC_GENERATE_MIPS;

	// The Shader Resource view description
	D3D11_SHADER_RESOURCE_VIEW_DESC SMViewDesc;
	SMViewDesc.Format = texDesc.Format;
	SMViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
	SMViewDesc.TextureCube.MipLevels = texDesc.MipLevels;
	SMViewDesc.TextureCube.MostDetailedMip = 0;
	 
	// Generate all mip levels
	Image **mips = new Image*[numMips*6];
	for (int face=0; face<6; face++)
	{
		mips[face*numMips] = faces[face];
		for(int miplevel=1; miplevel<numMips; miplevel++)
		{
			// Create mip images for all but the bottommost mip
			mips[face*numMips+miplevel] = new Image( faces[face]->width() >> miplevel, faces[face]->height() >> miplevel );

			// Iterate every pixel
			for(int i=0; i<mips[face*numMips+miplevel]->width(); i++)
			{
				for(int j=0; j<mips[face*numMips+miplevel]->height(); j++)
				{
					// Average 4 pixels of the lower mip
					// Convert from gamma to linear space for correct blending
					Color average;
					average = mips[face*numMips+miplevel-1]->getPixel(i*2,j*2).toLinear()/4;
					average += mips[face*numMips+miplevel-1]->getPixel(i*2+1,j*2).toLinear()/4;
					average += mips[face*numMips+miplevel-1]->getPixel(i*2,j*2+1).toLinear()/4;
					average += mips[face*numMips+miplevel-1]->getPixel(i*2+1,j*2+1).toLinear()/4;

					// then back to gamma as this pixel's value
					mips[face*numMips+miplevel]->operator()(i,j) = average.toGamma();
				}
			}
		}
	}
	 
	// Fill in the cubemap faces with the data
	std::vector<D3D11_SUBRESOURCE_DATA> pData(numMips*6);
	for (int i = 0; i < numMips*6; i++)
	{
		// Pointer to the pixel data
		pData[i].pSysMem = mips[i]->data(); 
		// Line width in bytes
		pData[i].SysMemPitch = mips[i]->stride(); 
		// This is only used for 3d textures.
		pData[i].SysMemSlicePitch = 0;
	}

	// Create the Texture Resource
	HRESULT hr = getDevice()->CreateTexture2D(&texDesc, &pData[0], &cubeTexture);
	if (hr != S_OK)
	{
		DEBUG_OUTPUT( __FUNCTION__ );
		DEBUG_OUTPUT( " : ID3D11Device::CreateTexture2D() failed. Returning NULL./n" );
		return NULL;
	}

	// Create the Shader Resource View
	hr = getDevice()->CreateShaderResourceView(cubeTexture, &SMViewDesc, &shaderResourceView);
	if (hr != S_OK)
	{
		DEBUG_OUTPUT( __FUNCTION__ );
		DEBUG_OUTPUT( " : ID3D11Device::CreateShaderResourceView() failed. Returning NULL./n" );
		return NULL;
	}

	// Free memory
	for (int face=0; face<6; face++)
	{
		for(int miplevel=1; miplevel<numMips; miplevel++)
		{
			delete mips[face*numMips+miplevel-1];
		}
	}
	delete[] mips;

	// Create the object
	TextureCube* tex = new TextureCube();
	tex->setFormat( R8G8B8A8_UNORM );
	tex->setResource( cubeTexture );
	tex->setShaderResourceView( shaderResourceView );
	tex->setRenderTargetView( NULL );
	tex->setDepthStencilView( NULL );
	tex->setDimensions( faces[0]->height(), faces[0]->width() );

	return tex;
}


/*
//
// Helper functions
//
HRESULT CompileShaderFromFile( WCHAR* szFileName, DWORD flags, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut )
{
    HRESULT hr = S_OK;

    // find the file
    WCHAR str[MAX_PATH];
    WCHAR workingPath[MAX_PATH], filePath[MAX_PATH];
    WCHAR *strLastSlash = NULL;
    bool  resetCurrentDir = false;

    // Get the current working directory so we can restore it later
    UINT nBytes = GetCurrentDirectory( MAX_PATH, workingPath );
    if (nBytes==MAX_PATH)
    {
      return E_FAIL;
    }

    // Check we can find the file first
    DXUTFindDXSDKMediaFileCch( str, MAX_PATH, szFileName );

    // Check if the file is in the current working directory
    wcscpy_s( filePath, MAX_PATH, str );

    strLastSlash = wcsrchr( filePath, TEXT( '\\' ) );
    if( strLastSlash )
    {
        // Chop the exe name from the exe path
        *strLastSlash = 0;

        SetCurrentDirectory( filePath );
        resetCurrentDir  = true;
    }

    // open the file
    HANDLE hFile = CreateFile( str, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
                               FILE_FLAG_SEQUENTIAL_SCAN, NULL );
    if( INVALID_HANDLE_VALUE == hFile )
        return E_FAIL;

    // Get the file size
    LARGE_INTEGER FileSize;
    GetFileSizeEx( hFile, &FileSize );

    // create enough space for the file data
    BYTE* pFileData = new BYTE[ FileSize.LowPart ];
    if( !pFileData )
        return E_OUTOFMEMORY;

    // read the data in
    DWORD BytesRead;
    if( !ReadFile( hFile, pFileData, FileSize.LowPart, &BytesRead, NULL ) )
        return E_FAIL; 

    CloseHandle( hFile );

    // Create an Include handler instance
    CIncludeHandler* pIncludeHandler = new CIncludeHandler;

    // Compile the shader using optional defines and an include handler for header processing
    ID3DBlob* pErrorBlob;
    hr = D3DCompile( pFileData, FileSize.LowPart, "none", NULL,     static_cast< ID3DInclude* > (pIncludeHandler), 
                     szEntryPoint, szShaderModel, flags, D3DCOMPILE_EFFECT_ALLOW_SLOW_OPS, ppBlobOut, &pErrorBlob );

	//DEBUG_OUTPUT( (LPCSTR)pErrorBlob->GetBufferPointer() );

    delete pIncludeHandler;
    delete []pFileData;

    // Restore the current working directory if we need to 
    if ( resetCurrentDir )
    {
        SetCurrentDirectory( workingPath );
    }


    if( FAILED(hr) )
    {
        //OutputDebugStringA( (char*)pErrorBlob->GetBufferPointer() );
		DEBUG_OUTPUT( __FUNCTION__ );
		DEBUG_OUTPUT( " : shader compilation failed\n" );
		DEBUG_OUTPUT( (char*)pErrorBlob->GetBufferPointer() );
        SAFE_RELEASE( pErrorBlob );
        return hr;
    }
    SAFE_RELEASE( pErrorBlob );

    return hr;
}
*/