#pragma once

#include "Prerequisites.h"


#include "Camera3D.h"
#include "Math.h"
#include "Utilities.h"
#include "Timer.h"
#include "Material.h"
#include "Mesh.h"
#include "MeshLoader.h"
#include "RenderCommand.h"
//#include "DX11RenderCommand.h"
#include "RenderDispatcher.h"
#include "Array3D.h"
#include "Array2D.h"

#include <vector>
#include <queue>
#include <utility>

#include <boost/lockfree/queue.hpp>
#include <boost/lockfree/spsc_queue.hpp>
#include <boost/interprocess/sync/interprocess_semaphore.hpp>
#include <concurrent_queue.h>

class RenderCommandAllocator
{
public:
	void initialize(UINT numBlocks)	{	m_Blocks.resize( numBlocks );
										for(UINT i=0; i<numBlocks; i++)
										{
											RenderCommandBlock* ptr = &m_Blocks[i];
											ZeroMemory(ptr, sizeof(RenderCommandBlock));
											m_Free.push(ptr);
										}
									};	

	inline RenderCommand* allocate()	{	RenderCommandBlock* block; 
											if (m_Free.try_pop(block)) 
												return reinterpret_cast<RenderCommand*>(block); 
											else return NULL; };

	inline void deallocate(RenderCommand* ptr)	{	ptr->~RenderCommand(); 
													/*ZeroMemory(ptr, sizeof(RenderCommandBlock));*/ 
													m_Free.push(reinterpret_cast<RenderCommandBlock*>(ptr)); };

protected:
	//boost::lockfree::spsc_queue<RenderCommandBlock*, boost::lockfree::capacity<1024>> m_Free;
	//boost::lockfree::queue<RenderCommandBlock*, boost::lockfree::capacity<1024>> m_Free;
	Concurrency::concurrent_queue<RenderCommandBlock*,tbb::scalable_allocator<RenderCommandBlock*>> m_Free;
	std::vector<RenderCommandBlock> m_Blocks;
};

struct ShadersetDescription
{
	std::wstring filename;
	std::string vertexShader;
	std::string pixelShader;
	SHADERMODEL sm;
	bool debug;

	inline bool ShadersetDescription::operator== (const ShadersetDescription& other) const
	{
		if (filename==other.filename)
			if (vertexShader==other.vertexShader)
				if (pixelShader==other.pixelShader)
					if (sm==other.sm)
						if (debug==other.debug)
							return true;
		return false;
	}
};

struct RenderSystemConfig
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
	bool Multithreaded;
};

class RenderSystem
{
public:
	RenderSystem() :m_pBoundingBoxWireframeMesh(NULL), frameFinishedSemaphore(0), m_Multithreaded(false)	{};

	HRESULT initialize( RenderSystemConfig creationConfig );

	RenderDispatcher* getRenderDispatcher();

	// Set output HWND. This will reinitialize some parts of the renderer.
	// KNOWN BUG: This call will cause PIX to crash
	void setOutputWindow( HWND hWnd, UINT width, UINT height );
	// Set a single render target.
	void setRenderTarget( Texture2D* pRenderTarget );
	// Set multiple render targets (MRTs).
	void setMultipleRenderTargets( UINT numRenderTargets, Texture2D** pRenderTargetArray );
	// Set render target to default (directly to the back buffer).
	void setBackbufferAsRenderTarget();
	// Resolve multisampled texture into a non-MSAA one.
	void resolveMSAA( Texture2D* pDestination, Texture2D* pSource );
	// Set active camera for rendering.
	void setCamera( Camera3D* pCamera );
	// Submit render command to execute.
	virtual void submit( RenderCommand* pRenderCommand );
	/*	Submit render command to execute (multithreaded).
		Submitted render commands are pushed on the render queue to be 
		executed on a dedicated rendering thread by consumeRenderQueue().*/
	void submitThreaded( RenderCommand* pRenderCommand );
	// Present the main backbuffer. SyncInterval is not yet used.
	void present( UINT SyncInterval );
	// Present a render output. SyncInterval is not yet used.
	void present( RenderOutput* pOutput, UINT SyncInterval );
	// Render target clears.
	void clearBackbuffer( float* clearColorRGBA );
	void clearDepthStencil( float depth, UINT8 stencil );
	// Texture clear
	void clearTexture( Texture2D* pTexture, float* clearColorRGBA );
	void downsampleTexture( Texture2D* target, Texture2D* source );
	// Returns current bound render target
	Texture2D*	getRenderTarget()	{ return m_pRenderTarget; };
	// Get backbuffer
	Texture2D*	getBackbuffer();

	// Returns configuration of the render system.
	RenderSystemConfig	getConfig()		{ return m_Config; };
	// Returns active camera.
	Camera3D*	getCamera();

	// Begin and end called every frame.
	virtual void	beginFrame( float* clearColorRGBA, float depthClear, UINT8 stencilClear );
	virtual void	beginFrame();
	virtual void	endFrame();

	/*	Executes all render commands in the render queue.
		Called on the rendering thread only when multithreaded rendering is enabled,
		on single-threaded rendering render queues are not used. */
	virtual void	consumeRenderQueue( bool returnAfterFrameFinished );

	// Synchronization purposes.
	void signalFrameFinished();
	void waitForFrameToFinish();

	// Cached resource loading & creation.
	virtual Mesh*	loadMesh( std::string filename, bool cache = true );
	virtual Shaderset*	loadShaderset( std::wstring filename, std::string vertexShader, std::string pixelShader, SHADERMODEL sm, std::vector<ShaderMacro>* macros = NULL, bool debug = false, bool cache = true );
	virtual Texture2D*	loadTexture( std::wstring filename, bool cache = true );
	// Resource creation.
	virtual GeometryChunk*	createGeometryChunk( float* vertices, UINT stride, UINT byteWidth, BufferLayout layout, UINT *indices, UINT numIndices, bool dynamic = false, D3D_PRIMITIVE_TOPOLOGY topology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
	virtual Shaderset*	createShadersetFromFile( std::wstring filename, std::string vertexShader, std::string pixelShader, SHADERMODEL sm, std::vector<ShaderMacro>* macros, bool debug );
	virtual Texture2D*	createTextureFromFile( std::wstring filename );
	virtual Texture2D*	createTexture( UINT height, UINT width, TEXTURE_FORMAT format );
	virtual Texture2D*	createTexture( const Image* const initialData );
	template<typename T> Texture2D*	createTexture( const Array2D<T>* const initialData, TEXTURE_FORMAT format );
	//virtual	Texture3D*	createTexture3D( UINT height, UINT width, UINT depth, TEXTURE_FORMAT format );
	template<typename T> Texture3D* createTexture3D( Array3D<T>* data, TEXTURE_FORMAT format );

	// Create cubemap from 6 images in the order: x+,x-,y+,y-,z+,z-.
	// All images must have the same dimensions. Only TEXTURE_FORMAT_R8G8B8A8_UNORM supported for now.
	virtual TextureCube*	createCubemap( Image* faces[6] );

	// Simple meshes
	//virtual Mesh* createBoxMesh( XMFLOAT3 dimensions, XMFLOAT3 uvscale );
	virtual Mesh* createPlaneMesh( XMFLOAT2 dimensions, XMFLOAT2 uvscale );
	virtual Mesh* createBoxWireframeMesh(XMFLOAT3 dimensions);
	virtual Mesh* createMesh( float* vertices, UINT numVertices, UINT* indices, UINT numIndices, BufferLayout vertexLayout, bool dynamic = false, D3D_PRIMITIVE_TOPOLOGY topology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

	// Object renderer
	virtual Renderer*	createRenderer();

	// Timings
	//double t_work;
	//double t_renderloop;
	double	t_commands;
	double	t_alloc;
	double	t_dealloc;
	double	t_material;
	double	t_queuepush;
	double	t_queuepop;
	double	t_renderstarve;
	double	t_shaderparams;
	int		drawcalls;

protected:
	RenderSystemConfig	m_Config;
	RenderDispatcher*	m_pDispatcher;
	Texture2D*			m_pRenderTarget;
	Camera3D*	m_pCamera;
	bool m_Multithreaded;

	// This semaphore tells whether or not the current frame has finished rendering
	boost::interprocess::interprocess_semaphore frameFinishedSemaphore;

	// Resource caches
	std::vector< std::pair<ShadersetDescription, Shaderset*> >	m_pShadersets;
	std::map< std::wstring, Texture2D* > m_pTextures;
	std::map< std::string, Mesh* > m_pMeshes;

	// Simple meshes
	std::vector< std::pair< std::pair<XMFLOAT2,XMFLOAT2> , Mesh* > > m_pPlaneMeshes;
	std::vector< std::pair< XMFLOAT3, Mesh* > > m_pBoxWireframeMeshes;

	Mesh* m_pBoundingBoxWireframeMesh;

	//boost::lockfree::spsc_queue<RenderCommand*, boost::lockfree::capacity<2048> > m_RenderQueue;
	boost::lockfree::queue<RenderCommand*, boost::lockfree::capacity<2048>> m_RenderQueue;
	//Concurrency::concurrent_queue<RenderCommand*, tbb::scalable_allocator<RenderCommand*>> m_RenderQueue;
	//std::queue<RenderCommand*> m_RenderQueue;
	RenderCommandAllocator m_RenderCommandAllocator;

	Timer timer;
	Timer workTimer;
};

template<typename T> 
Texture2D*	RenderSystem::createTexture( const Array2D<T>* const initialData, TEXTURE_FORMAT format )
{
	Texture2D* tex;
	if ( initialData != NULL)
		tex = m_pDispatcher->createTexture( initialData->height(), 
											initialData->width(), 
											format, 
											initialData->data(), 
											initialData->pitch(), 
											initialData->size() );

	if (tex)
		tex->setRenderSystem(this);
	
	return tex;
}

template<typename T>
Texture3D* RenderSystem::createTexture3D( Array3D<T>* data, TEXTURE_FORMAT format )
{
	Texture3D* tex = m_pDispatcher->createTexture3D( data->width(), data->height(), data->depth(), format, data->data(), data->pitch(), data->slicePitch(), data->size() ); 
	if (tex)
		tex->setRenderSystem(this);
	
	return tex;
}