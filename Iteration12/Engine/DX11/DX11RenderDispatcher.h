#pragma once

#include "..\Prerequisites.h"
#include "..\RenderDispatcher.h"
#include <d3d11.h>
#include <d3dx11.h>
#include <d3dcompiler.h>
#include <xnamath.h>

// Implements the RenderDispatcher in DirectX 11
class DX11RenderDispatcher : public RenderDispatcher
{
public:
	DX11RenderDispatcher();

	HRESULT Initialize( RenderDispatcherConfig creationConfig );

	Renderer* CreateRenderer();
	void BindShaderParams( ShaderParamBlock *pParamBlock );
	void BindShaderParams( ShaderParams *pParams );
	void BindConstantBufferVS( UINT i, ConstantBufferData* pData );
	void BindConstantBufferPS( UINT i, ConstantBufferData* pData );
	void SetOutputWindow( HWND hWnd, UINT width, UINT height );
	void ResizeTarget( UINT height, UINT width );
	void ResizeBackbuffer( UINT height, UINT width );
	void SetRenderTarget( Texture2D* pRenderTarget );
	void SetMultipleRenderTargets( UINT numRenderTargets, Texture2D** pRenderTargetArray );
	void SetBackbufferAsRenderTarget();
	void ResolveMSAA( Texture2D* pDestination, Texture2D* pSource );
	void Present( UINT SyncInterval );
	void SetRenderState( const RenderState& renderState );
	void SetBlendState( const BlendState& blendState );

	void SetDevice( ID3D11Device* pDevice )								{ m_pDevice = pDevice; };
	void SetImmediateContext( ID3D11DeviceContext* pContext )			{ m_pImmediateContext = pContext; };
	void SetActiveShaderset( DX11Shaderset* pShaderset )				{ m_pActiveShaderset = pShaderset; };

	inline ID3D11Device* GetDevice()									{ return m_pDevice; };
	inline ID3D11DeviceContext* GetImmediateContext()					{ return m_pImmediateContext; };
	inline ID3D11RenderTargetView*	GetBackbufferRenderTargetView()		{ return m_pRenderTargetView; };
	inline ID3D11DepthStencilView* GetBackbufferDepthStencilView()		{ return m_pDepthStencilView; };
	inline IDXGISwapChain*	GetSwapChain()								{ return m_pSwapChain; };
	inline DX11Shaderset* GetActiveShaderset()							{ return m_pActiveShaderset; };
	
	//inline Texture2D* GetBackbuffer()											{ return (Texture2D*)m_pBackbufferTexture; };

	void ClearBackbuffer( float* clearColorRGBA );
	void ClearDepth( float depth );
	void ClearStencil( UINT8 stencil );
	void ClearDepthStencil( float depth, UINT8 stencil );

	// Resource creation
	GeometryChunk*	CreateGeometryChunk( float* vertices, UINT stride, UINT byteWidth, BufferLayout layout, UINT *indices, UINT numIndices, bool dynamic = false, PRIMITIVE_TOPOLOGY topology = PRIMITIVE_TOPOLOGY_TRIANGLELIST );
	Shaderset*		CreateShadersetFromFile( std::wstring filename, std::string vertexShader, std::string pixelShader, SHADERMODEL sm, std::vector<ShaderMacro> macros, bool debug );
	TextureCube*	CreateTextureCubeFromFile( std::wstring filename );
	Texture2D*		CreateTexture2DFromFile( std::wstring filename );
	Texture2D*		CreateTexture2D( UINT height, UINT width, TEXTURE_FORMAT format, const void* data, size_t pitch, size_t dataSize );
	TextureCube*	CreateCubemap( const Image* faces[6] );
	Texture3D*		CreateTexture3D( UINT width, UINT height, UINT depth, TEXTURE_FORMAT format, const void* data, size_t pitch, size_t slicePitch, size_t dataSize );

	DX11VertexBuffer*	CreateVertexBuffer( const void* pData, int dataSize, BufferLayout layout, int stride, bool dynamic = false );
	DX11IndexBuffer*	CreateIndexBuffer( UINT* pData, int numOfIndices );

protected:
	RenderDispatcherConfig		m_Config;
	ID3D11Device*				m_pDevice;
	ID3D11DeviceContext*		m_pImmediateContext;
	D3D_DRIVER_TYPE				m_driverType;
	D3D_FEATURE_LEVEL			m_featureLevel;
	IDXGISwapChain*             m_pSwapChain;
	ID3D11Texture2D*			m_pd3dBackbuffer;
	ID3D11Texture2D*			m_pDepthStencil;
	ID3D11DepthStencilView*		m_pDepthStencilView;
	ID3D11RenderTargetView*     m_pRenderTargetView;
	UINT						m_Width, m_Height;

	DX11Shaderset*				m_pActiveShaderset;

	RenderState					m_CurrentRenderState;
	BlendState					m_CurrentBlendState;
	std::unordered_map<BlendState, ID3D11BlendState*, BlendStateHasher> m_BlendStates;
	std::unordered_map<RenderState, std::pair<ID3D11RasterizerState*, ID3D11DepthStencilState*>, RenderStateHasher> m_RenderStates;

	// Default sampler states
	ID3D11SamplerState*			m_pPointSampler;
	ID3D11SamplerState*			m_pLinearSampler;
	ID3D11SamplerState*			m_pComparisonSampler;
	ID3D11SamplerState*			m_pLinearClampSampler;

	// Temporary buffers holding Constant Buffer data
	PBYTE						m_pVSCBDataBuffer[32];
	PBYTE						m_pPSCBDataBuffer[32];
	UINT						m_pVSCBDataBufferSize[32];
	UINT						m_pPSCBDataBufferSize[32];

	// Constant buffer for internal use
	struct InternalCB
	{
		// The D3D11 constant buffer
		ID3D11Buffer*	pBuffer;
		// Size in bytes
		UINT	ByteSize;
		// Indicates that it's currently unavailable (already bound to the pipeline)
		bool	InUse;
	};

	// Constant buffers
	// map[size in bytes, vector of CB's with that size]
	std::map<UINT, std::vector<InternalCB*>>	m_pConstantBuffers;

	/*
		Returns a constant buffer with the requested size in bytes to bind to the pipeline.
		Creates a new one if not found. May return NULL if it somehow fails to create one.
	*/
	InternalCB* GetAvailableConstantBuffer( UINT ByteSize );

private:
	// Doesn't check if a matching RenderState has already been created
	std::pair<ID3D11RasterizerState*, ID3D11DepthStencilState*> CreateRenderState( const RenderState& renderState );
	// Doesn't check if a matching BlendState has already been created
	// TODO : MRT blend states
	ID3D11BlendState* CreateBlendState( const BlendState& blendState );

	// Helpers
	DXGI_FORMAT ToDXGIFormat( TEXTURE_FORMAT format );
	TEXTURE_FORMAT ToNGTextureFormat( DXGI_FORMAT format );
	HRESULT CompileShaderFromFile( WCHAR* szFileName, DWORD flags, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut );
};

D3D_PRIMITIVE_TOPOLOGY ToD3DPrimitiveTopology( PRIMITIVE_TOPOLOGY topology );
PRIMITIVE_TOPOLOGY ToNGPrimitiveTopology( D3D_PRIMITIVE_TOPOLOGY topology );

//--------------------------------------------------------------------------------------
// Use this until D3DX11 comes online and we get some compilation helpers
//--------------------------------------------------------------------------------------
static const unsigned int MAX_INCLUDES = 9;
struct sInclude
{
	HANDLE         hFile;
	HANDLE         hFileMap;
	LARGE_INTEGER  FileSize;
	void           *pMapData;
};

class CIncludeHandler : public ID3DInclude
{
	private:
		struct sInclude   m_includeFiles[MAX_INCLUDES];
		unsigned int      m_nIncludes;

	public:
	CIncludeHandler()
	{
		// array initialization
		for ( unsigned int i=0; i<MAX_INCLUDES; i++)
		{
			m_includeFiles[i].hFile = INVALID_HANDLE_VALUE;
			m_includeFiles[i].hFileMap = INVALID_HANDLE_VALUE;
			m_includeFiles[i].pMapData = NULL;
		}
		m_nIncludes = 0;
	}
	~CIncludeHandler()
	{
		for (unsigned int i=0; i<m_nIncludes; i++)
		{
			UnmapViewOfFile( m_includeFiles[i].pMapData );

			if ( m_includeFiles[i].hFileMap != INVALID_HANDLE_VALUE)
			CloseHandle( m_includeFiles[i].hFileMap );

			if ( m_includeFiles[i].hFile != INVALID_HANDLE_VALUE)
			CloseHandle( m_includeFiles[i].hFile );
		}

		m_nIncludes = 0;
	}

	STDMETHOD(Open(
		D3D_INCLUDE_TYPE IncludeType,
		LPCSTR pFileName,
		LPCVOID pParentData,
		LPCVOID *ppData,
		UINT *pBytes
	))
	{
		unsigned int   incIndex = m_nIncludes+1;

		// Make sure we have enough room for this include file
		if ( incIndex >= MAX_INCLUDES )
		return E_FAIL;

		// try to open the file
		m_includeFiles[incIndex].hFile  = CreateFileA( pFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
												FILE_FLAG_SEQUENTIAL_SCAN, NULL );
		if( INVALID_HANDLE_VALUE == m_includeFiles[incIndex].hFile )
		{
		return E_FAIL;
		}

		// Get the file size
		GetFileSizeEx( m_includeFiles[incIndex].hFile, &m_includeFiles[incIndex].FileSize );

		// Use Memory Mapped File I/O for the header data
		m_includeFiles[incIndex].hFileMap = CreateFileMappingA( m_includeFiles[incIndex].hFile, NULL, PAGE_READONLY, m_includeFiles[incIndex].FileSize.HighPart, m_includeFiles[incIndex].FileSize.LowPart, pFileName);
		if( m_includeFiles[incIndex].hFileMap == NULL  )
		{
		if (m_includeFiles[incIndex].hFile != INVALID_HANDLE_VALUE)
			CloseHandle( m_includeFiles[incIndex].hFile );
		return E_FAIL;
		}

		// Create Map view
		*ppData = MapViewOfFile( m_includeFiles[incIndex].hFileMap, FILE_MAP_READ, 0, 0, 0 );
		*pBytes = m_includeFiles[incIndex].FileSize.LowPart;

		// Success - Increment the include file count
		m_nIncludes= incIndex;

		return S_OK;
	}

	STDMETHOD(Close( LPCVOID pData ))
	{
		// Defer Closure until the container destructor 
		return S_OK;
	}
};