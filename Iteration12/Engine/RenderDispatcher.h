#pragma once

#include "Prerequisites.h"

#include "Debug.h"
#include "xnaCollision.h"
#include "Math.h"
#include "Timer.h"
#include "Image.h"
#include "Shaderset.h"
#include "GeometryChunk.h"
#include "RenderState.h"
#include "BlendState.h"


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
	graphics API calls.
	Not thread-safe except resouce creation functions.
*/
class RenderDispatcher
{
public:
	RenderDispatcher();

	static RenderDispatcher* Create( RenderDispatcherConfig creationInfo );

	virtual HRESULT Initialize( RenderDispatcherConfig creationInfo )=0;

	// Consume a render queue.
	void ConsumeRenderQueue( std::queue<RenderCommand> *pRenderQueue );
	// Executes a RenderCommand. Called by RenderSystem.
	//void ExecuteRenderCommand( RenderCommand *pRenderCommand );

	// Creates an object renderer
	virtual Renderer* CreateRenderer()=0;
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
	//
	virtual void SetRenderState( const RenderState& renderState, UINT StencilRef = 0 )=0;
	//
	virtual void SetBlendState( const BlendState& blendState )=0;

	inline void	SetBackbufferTexture( Texture2D* pTexture )		{ m_pBackbuffer = pTexture; };
	inline Texture2D*	GetBackbuffer()							{ return m_pBackbuffer; };

	// Backbuffer clears.
	virtual void ClearBackbuffer( float* clearColorRGBA )=0;
	virtual void ClearDepth( float depth )=0;
	virtual void ClearStencil( UINT8 stencil )=0;
	virtual void ClearDepthStencil( float depth, UINT8 stencil )=0;

	// Resource creation
	// All resourse creation calls are thread-safe
	virtual GeometryChunk*	CreateGeometryChunk( float* vertices, UINT stride, UINT byteWidth, BufferLayout layout, UINT *indices, UINT numIndices, bool dynamic = false, PRIMITIVE_TOPOLOGY topology = PRIMITIVE_TOPOLOGY_TRIANGLELIST ) = 0;
	virtual Shaderset*	CreateShadersetFromFile( std::wstring filename, std::string vertexShader, std::string pixelShader, SHADERMODEL sm, std::vector<ShaderMacro> macros, bool debug ) = 0;
	virtual TextureCube* CreateTextureCubeFromFile( std::wstring filename ) = 0;
	virtual Texture2D*	CreateTexture2DFromFile( std::wstring filename ) = 0;
	virtual Texture2D*	CreateTexture2D( UINT height, UINT width, TEXTURE_FORMAT format, const void* data = NULL, size_t pitch = 0, size_t dataSize = 0) = 0; 
	//virtual Texture2D*	CreateTexture2D( UINT height, UINT width, TEXTURE_FORMAT format, const void* const data = NULL, size_t pitch = 0, size_t dataSize = 0) = 0;
	//virtual Texture3D*	CreateTexture2D3D( UINT height, UINT width, UINT depth, DXGI_FORMAT format ) = 0;
	virtual Texture3D*	CreateTexture3D( UINT width, UINT height, UINT depth, TEXTURE_FORMAT format, const void* data, size_t pitch, size_t slicePitch, size_t dataSize ) = 0;
	// Create cubemap from 6 images in the order: x+,x-,y+,y-,z+,z-.
	// All images must have the same dimensions. Only R8G8B8A8_UNORM supported for now.
	virtual TextureCube* CreateCubemap( const Image* faces[6] ) = 0;

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