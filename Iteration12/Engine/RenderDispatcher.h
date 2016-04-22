#pragma once

#include "Prerequisites.h"

#include <d3d11.h>
#include <d3dx11.h>
#include <d3dcompiler.h>
#include <xnamath.h>
#include <map>
#include <unordered_map>
#include <vector>
#include <queue>
#include <string>

#include "Debug.h"
#include "xnaCollision.h"
#include "Math.h"
#include "Timer.h"
#include "Image.h"
#include "Shaderset.h"

enum SHADERMODEL;
enum TEXTURE_FORMAT;

enum GraphicsAPI
{
	GRAPHICS_API_DX10,
	GRAPHICS_API_DX10_1,
	GRAPHICS_API_DX11,
	GRAPHICS_API_OPENGL,
	GRAPHICS_API_OPENGL_ES,
	GRAPHICS_API_MANTLE,
	GRAPHICS_API_VULKAN,	// Probably wont need to implement this many :S
};

enum Multisampling
{
	MSAA_NONE,
	MSAA_2X,
	MSAA_4X,
	MSAA_8X,
	MSAA_16X,
	CSAA_8X,
	CSAA_8XQ,
	CSAA_16X,
	CSAA_16XQ,
};

struct RenderDispatcherConfig
{
	GraphicsAPI	api;
	UINT Width;
	UINT Height;
	UINT RefreshRate;
	Multisampling BackbufferMultisampling;
	UINT MaxAnisotropy;
	HWND OutputWindow;
	bool Windowed;
	bool Vsync;
	bool DebugDevice;
};


/*
	The subclasses of RenderDispatcher are the ones responsible for dispatching 
	graphics API calls, and (will soon be) the only classes that can directly access API's.
	Not thread-safe except resouce creation functions.
*/
class RenderDispatcher
{
public:
	RenderDispatcher() : frameFinished(false) {};

	static RenderDispatcher* create( RenderDispatcherConfig creationInfo );

	virtual HRESULT initialize( RenderDispatcherConfig creationInfo )=0;

	// Consume a render queue.
	void ConsumeRenderQueue( std::queue<RenderCommand> *pRenderQueue );
	// Executes a RenderCommand. Called by RenderSystem.
	//void ExecuteRenderCommand( RenderCommand *pRenderCommand );

	//virtual void addRenderAgent( IRenderAgent* pRenderAgent );

	// Bind shader parameters for drawing via the old ShaderParamBlock.
	virtual void BindShaderParams( ShaderParamBlock *pParamBlock )=0;
	// Bind shader parameters for drawing via the new ShaderParams class. Faster and more memory-efficient.
	virtual void BindShaderParams( ShaderParams *pParams )=0;
	// Bind constant buffers to the Vertex Shader.
	virtual void BindConstantBufferVS( UINT i, ConstantBufferData* pData )=0;
	// Bind constant buffers to the Vertex Shader.
	virtual void BindConstantBufferPS( UINT i, ConstantBufferData* pData )=0;
	// Set output HWND. This will reinitialize parts of the renderer.
	virtual void SetOutputWindow( HWND hWnd, UINT width, UINT height )=0;
	// Resize swapchain target
	virtual void ResizeTarget( UINT height, UINT width )=0;
	// Resize backbuffer
	virtual void ResizeBackbuffer( UINT height, UINT width )=0;
	// Set a single render target.
	virtual void SetRenderTarget( Texture2D* pRenderTarget )=0;
	// Set multiple render targets (MRTs).
	virtual void SetMultipleRenderTargets( UINT numRenderTargets, Texture2D** pRenderTargets )=0;
	// Set render target to default (directly to the back buffer).
	virtual void SetBackbufferAsRenderTarget()=0;
	// Resolve multisampled texture into a non-MSAA one.
	virtual void ResolveMSAA( Texture2D* pDestination, Texture2D* pSource )=0;
	// Present back buffer. SyncInterval not yet in use.
	virtual void Present( UINT SyncInterval )=0;

	inline void	SetBackbufferTexture( Texture2D* pTexture )		{ m_pBackbuffer = pTexture; };
	inline Texture2D*	GetBackbuffer()							{ return m_pBackbuffer; };

	// Backbuffer clears.
	virtual void ClearBackbuffer( float* clearColorRGBA )=0;
	virtual void ClearDepth( float depth )=0;
	virtual void ClearStencil( UINT8 stencil )=0;
	virtual void ClearDepthStencil( float depth, UINT8 stencil )=0;

	// Resource creation
	// All resourse creation calls are thread-safe
	virtual GeometryChunk*	CreateGeometryChunk( float* vertices, UINT stride, UINT byteWidth, BufferLayout layout, UINT *indices, UINT numIndices, bool dynamic = false, D3D_PRIMITIVE_TOPOLOGY topology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST ) = 0;
	virtual Shaderset*	CreateShadersetFromFile( std::wstring filename, std::string vertexShader, std::string pixelShader, SHADERMODEL sm, std::vector<ShaderMacro> macros, bool debug ) = 0;
	virtual Texture2D*	CreateTextureFromFile( std::wstring filename ) = 0;
	virtual Texture2D*	CreateTexture( UINT height, UINT width, TEXTURE_FORMAT format, const void* const data = NULL, size_t pitch = 0, size_t dataSize = 0) = 0; 
	//virtual Texture2D*	CreateTexture( UINT height, UINT width, TEXTURE_FORMAT format, const void* const data = NULL, size_t pitch = 0, size_t dataSize = 0) = 0;
	//virtual Texture3D*	CreateTexture3D( UINT height, UINT width, UINT depth, DXGI_FORMAT format ) = 0;
	virtual Texture3D*	CreateTexture3D( UINT width, UINT height, UINT depth, DXGI_FORMAT format, const void* const data, size_t pitch, size_t slicePitch, size_t dataSize ) = 0;
	// Create cubemap from 6 images in the order: x+,x-,y+,y-,z+,z-.
	// All images must have the same dimensions. Only R8G8B8A8_UNORM supported for now.
	virtual TextureCube* CreateCubemap( Image* const faces[6] ) = 0;

	// Flags and timing data.
	// TODO : a lot
	volatile bool				frameFinished;
	Timer						renderTimer, timer2;
	double						t_renderthread;
	double						t_renderloop;
	double						t_BeginFrame;
	double						t_EndFrame;
	double						t_work;
	double						t_starve;
	double						t_bindparams;
	double						t_1, t_2;
	double						t_bindtextures;
	double						t_drawcalls;
	double						numVerts;
	int							drawcalls;

	// The owning RenderSystem
	RenderSystem*				m_pRenderSystem;	

protected:
	HWND                        m_hWnd;
	Texture2D*					m_pBackbuffer;
};

// Implements the RenderDispatcher in DirectX 11
class DX11RenderDispatcher : public RenderDispatcher
{
public:
	DX11RenderDispatcher();

	HRESULT initialize( RenderDispatcherConfig creationConfig );

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
	void Present( UINT SyncInterval )									{ m_pSwapChain->Present( SyncInterval, 0 ); };

	void SetDevice( ID3D11Device* pDevice )								{ m_pDevice = pDevice; };
	void SetImmediateContext( ID3D11DeviceContext* pContext )			{ m_pImmediateContext = pContext; };
	void SetActiveShaderset( D3D11Shaderset* pShaderset )				{ m_pActiveShaderset = pShaderset; };

	inline ID3D11Device* GetDevice()									{ return m_pDevice; };
	inline ID3D11DeviceContext* GetImmediateContext()					{ return m_pImmediateContext; };
	inline ID3D11RenderTargetView*	GetBackbufferRenderTargetView()		{ return m_pRenderTargetView; };
	inline ID3D11DepthStencilView* GetBackbufferDepthStencilView()		{ return m_pDepthStencilView; };
	inline IDXGISwapChain*	GetSwapChain()								{ return m_pSwapChain; };
	inline D3D11Shaderset* GetActiveShaderset()							{ return m_pActiveShaderset; };
	
	//inline Texture2D* GetBackbuffer()											{ return (Texture2D*)m_pBackbufferTexture; };

	void ClearBackbuffer( float* clearColorRGBA );
	void ClearDepth( float depth );
	void ClearStencil( UINT8 stencil );
	void ClearDepthStencil( float depth, UINT8 stencil );

	// Resource creation
	GeometryChunk*	CreateGeometryChunk( float* vertices, UINT stride, UINT byteWidth, BufferLayout layout, UINT *indices, UINT numIndices, bool dynamic = false, D3D_PRIMITIVE_TOPOLOGY topology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
	Shaderset*		CreateShadersetFromFile( std::wstring filename, std::string vertexShader, std::string pixelShader, SHADERMODEL sm, std::vector<ShaderMacro> macros, bool debug );
	Texture2D*		CreateTextureFromFile( std::wstring filename );
	Texture2D*		CreateTexture( UINT height, UINT width, TEXTURE_FORMAT format, const void* const data, size_t pitch, size_t dataSize );
	TextureCube*	CreateCubemap( Image* const faces[6] );
	Texture3D*		CreateTexture3D( UINT width, UINT height, UINT depth, DXGI_FORMAT format, const void* const data, size_t pitch, size_t slicePitch, size_t dataSize );

	D3D11VertexBuffer* createVertexBuffer( const void* pData, int dataSize, BufferLayout layout, int stride, bool dynamic = false );
	D3D11IndexBuffer* createIndexBuffer( UINT* pData, int numOfIndices );

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

	D3D11Shaderset*				m_pActiveShaderset;

	// Default sampler states
	ID3D11SamplerState*			m_pPointSampler, 
								*m_pLinearSampler, 
								*m_pComparisonSampler, 
								*m_pLinearClampSampler;

	// Temporary buffers holding Constant Buffer data
	PBYTE						m_pVSCBDataBuffer[32];
	PBYTE						m_pPSCBDataBuffer[32];
	UINT						m_pVSCBDataBufferSize[32];
	UINT						m_pPSCBDataBufferSize[32];

	// Constant buffer for internal use
	struct InternalCB
	{
		ID3D11Buffer*	pBuffer;
		UINT	ByteSize;
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
};









// Helpers
DXGI_FORMAT toDXGIFormat( TEXTURE_FORMAT format );
TEXTURE_FORMAT toNGTextureFormat( DXGI_FORMAT format );

HRESULT CompileShaderFromFile( WCHAR* szFileName, DWORD flags, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut );

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

