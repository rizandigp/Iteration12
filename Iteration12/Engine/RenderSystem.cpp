#pragma once

#include "RenderSystem.h"
#include "TextureCube.h"
#include "Array3D.h"
#include "FullscreenQuad.h"

HRESULT RenderSystem::initialize( RenderSystemConfig creationConfig )
{
	m_Config = creationConfig;
	m_Multithreaded = creationConfig.Multithreaded;

	RenderDispatcherConfig rdConfig;
	rdConfig.api = creationConfig.api;
	rdConfig.BackbufferMultisampling = creationConfig.BackbufferMultisampling;
	rdConfig.MaxAnisotropy = creationConfig.MaxAnisotropy;
	rdConfig.Height = creationConfig.Height;
	rdConfig.Width = creationConfig.Width;
	rdConfig.RefreshRate = creationConfig.RefreshRate;
	rdConfig.Vsync = creationConfig.Vsync;
	rdConfig.Windowed = creationConfig.Windowed;
	rdConfig.OutputWindow = creationConfig.OutputWindow;
	rdConfig.DebugDevice = creationConfig.DebugDevice;

	m_pDispatcher = new DX11RenderDispatcher();
	m_pDispatcher->initialize( rdConfig );
	m_pDispatcher->m_pRenderSystem = this;

	m_RenderCommandAllocator.initialize( 2048 );

	return S_OK;
};

void RenderSystem::setOutputWindow( HWND hWnd, UINT width, UINT height )
{
	m_pDispatcher->setOutputWindow( hWnd, width, height );
}

GeometryChunk*	 RenderSystem::createGeometryChunk( float* vertices, UINT stride, UINT byteWidth, BufferLayout layout, UINT *indices, UINT numIndices, bool dynamic, D3D_PRIMITIVE_TOPOLOGY topology )
{
	GeometryChunk* pGeom = m_pDispatcher->createGeometryChunk( vertices, stride, byteWidth, layout, indices, numIndices, dynamic, topology );
	if (pGeom)
		pGeom->setRenderSystem( this );

	return pGeom;
}

Shaderset*	 RenderSystem::createShadersetFromFile( std::wstring filename, std::string vertexShader, std::string pixelShader, SHADERMODEL sm, std::vector<ShaderMacro>* macros, bool debug )
{
	// Empty vector
	std::vector<ShaderMacro> shaderMacros;
	if (macros!=NULL)
		shaderMacros = *macros;

	return m_pDispatcher->createShadersetFromFile( filename, vertexShader, pixelShader, sm, shaderMacros, debug );
}

Texture2D*	 RenderSystem::createTextureFromFile( std::wstring filename )
{
	return m_pDispatcher->createTextureFromFile( filename );
}

Texture2D*	 RenderSystem::createTexture( UINT height, UINT width, TEXTURE_FORMAT format )
{
	Texture2D* tex = m_pDispatcher->createTexture( height, width, format );

	if (tex)
		tex->setRenderSystem(this);
	
	return tex;
}

Texture2D*	 RenderSystem::createTexture( const Image* const initialData )
{
	Texture2D* tex;
	if ( initialData != NULL)
		tex = m_pDispatcher->createTexture( initialData->height(), 
											initialData->width(), 
											R8G8B8A8_UNORM, 
											initialData->data(), 
											initialData->pitch(), 
											initialData->dataSize() );

	if (tex)
		tex->setRenderSystem(this);
	
	return tex;
}

TextureCube* RenderSystem::createCubemap( Image* faces[6] )
{
	// Check that all faces are of equal dimensions
	UINT width = faces[0]->width();
	UINT height = faces[0]->height();
	for (int i=1; i<6; i++)
	{
		if ((faces[i]->width() != width)||(faces[i]->height() != height))
		{
			NGWARNING( "Texture creation failed, all faces of a cubemap must be of equal dimensions. Returning NULL." );
			return NULL;
		}
	}

	TextureCube* tex = m_pDispatcher->createCubemap( faces );
	if (tex)
		tex->setRenderSystem(this);
	
	return tex;
}

void RenderSystem::setCamera( Camera3D* pCamera )
{
	m_pCamera = pCamera;
};

Camera3D* RenderSystem::getCamera()
{
	return m_pCamera;
}

RenderDispatcher* RenderSystem::getRenderDispatcher()
{
	return m_pDispatcher;
}

void RenderSystem::consumeRenderQueue( bool returnAfterFrameFinished )
{
	RenderCommand* rc;
	bool notEmpty = true;
	this->getRenderDispatcher()->frameFinished = false;
	if (returnAfterFrameFinished)
	{
		Timer looptimer;
		Timer rcTimer;
		Timer deallocTimer;
		Timer popTimer;
		do
		{
			//looptimer.start();
			popTimer.start();
			notEmpty = m_RenderQueue.pop( rc );
			if(notEmpty)
			{
				t_queuepop += popTimer.getMicroseconds();
				rcTimer.start();
				rc->execute( this->m_pDispatcher );
				t_commands += rcTimer.getMiliseconds();

				deallocTimer.start();
				m_RenderCommandAllocator.deallocate(rc);
				//delete rc;
				t_dealloc += deallocTimer.getMiliseconds();
			}
			else
				t_renderstarve += popTimer.getMiliseconds();
			//getRenderDispatcher()->t_renderloop += looptimer.getMiliseconds();
		}
		while (!this->getRenderDispatcher()->frameFinished);
		return;
	}
	else
	{
		Timer looptimer;
		Timer rcTimer;
		Timer deallocTimer;
		Timer popTimer;
		while (1)
		{
			//looptimer.start();
			popTimer.start();
			notEmpty = m_RenderQueue.pop( rc );
			if(notEmpty)
			{
				t_queuepop += popTimer.getMicroseconds();
				rcTimer.start();
				rc->execute( this->m_pDispatcher );
				t_commands += rcTimer.getMiliseconds();

				deallocTimer.start();
				m_RenderCommandAllocator.deallocate(rc);
				//delete rc;
				t_dealloc += deallocTimer.getMiliseconds();
			}
			else
				t_renderstarve += popTimer.getMiliseconds();
			//getRenderDispatcher()->t_renderloop += looptimer.getMiliseconds();
		}
	}
	
	return;
}

void RenderSystem::signalFrameFinished()
{
	frameFinishedSemaphore.post();
}

void RenderSystem::waitForFrameToFinish()
{
	frameFinishedSemaphore.wait();
}

void RenderSystem::submit( RenderCommand* pRenderCommand )
{
	if (m_Multithreaded)
		// If multithreading is enabled, push this render command to the render queue 
		// to be rendered on a dedicated thread
		submitThreaded( pRenderCommand );

	else
	{
		// Single-threaded, render directly
		Timer timer;
		pRenderCommand->execute( m_pDispatcher );
		t_commands += timer.getMiliseconds();
	}
}

void RenderSystem::submitThreaded( RenderCommand* pRenderCommand )
{
	//submit( pRenderCommand );
	
	// Allocate
	Timer allocTimer;
	RenderCommand* ptr;
	do {
		ptr = m_RenderCommandAllocator.allocate();// (RenderCommand*) new RenderCommandBlock();//
	} while (ptr==NULL);

	//memcpy(ptr, pRenderCommand, sizeof(RenderCommandBlock));
	pRenderCommand->clone( ptr );
	t_alloc += allocTimer.getMiliseconds();
	//ptr->m_InUse = true;

	// Push into render queue
	Timer pushTimer;
	bool pushed;
	do
	{
		pushed = m_RenderQueue.push( ptr );
	} while (!pushed);
	
	//m_RenderQueue.push( ptr );
	t_queuepush += pushTimer.getMicroseconds();
}

void RenderSystem::setRenderTarget( Texture2D* pRenderTarget )
{
	m_pRenderTarget = pRenderTarget;
	D3D11RenderCommand_RenderTarget rc;
	rc.setRenderTargetTexture( (DX11Texture2D*)pRenderTarget );

	submit( &rc );
}

void RenderSystem::setMultipleRenderTargets( UINT numRenderTargets, Texture2D** pRenderTargetArray )
{
	// Check that all render targets have the same dimensions
	UINT width = pRenderTargetArray[0]->getWidth();
	UINT height = pRenderTargetArray[0]->getHeight();
	for (int i=1; i<numRenderTargets; i++)
	{
		if((pRenderTargetArray[i]->getWidth() != width)||(pRenderTargetArray[0]->getHeight() != height))
		{
			NGWARNING( "Failed : Multiple Render Targets (MRTs) must have the same dimensions. Render target unchanged." );
			return;
		}
	}

	// Create command and submit
	D3D11RenderCommand_MRT rc;
	rc.setRenderTargetTextures( numRenderTargets, (DX11Texture2D**)pRenderTargetArray );

	submit( &rc );
}

void RenderSystem::setBackbufferAsRenderTarget()
{
	// A NULL render target automatically sets it to the backbuffer
	D3D11RenderCommand_RenderTarget rc;
	rc.setRenderTargetTexture( NULL );
	
	submit( &rc );
}

void RenderSystem::resolveMSAA( Texture2D* pDestination, Texture2D* pSource )
{
	D3D11RenderCommand_ResolveMSAA rc;
	rc.setDestination( pDestination );
	rc.setSource( pSource );

	submit( &rc );
}

void RenderSystem::clearTexture( Texture2D* pTexture, float* clearColorRGBA )
{
	D3D11RenderCommand_ClearTexture rc;
	rc.setClearColor( clearColorRGBA );
	rc.setTexture( (DX11Texture2D*)pTexture );
	
	submit( &rc );
}

void RenderSystem::downsampleTexture( Texture2D* target, Texture2D* source )
{
	// TODO : put up a warning if the target resolution isn't right

	// Get our current bound render target so we can revert back to it later
	Texture2D* renderTarget = this->getRenderTarget();

	// Set target texture as render target
	this->setRenderTarget( target );
	float clearColor[4] = { 0.5f, 0.125f, 0.3f, 1.0f };
	this->clearTexture( target, clearColor );

	// Fullscreen pass
	FullscreenQuad* quad = new FullscreenQuad( this );
	quad->setShaderset( this->loadShaderset( L"Shaders/Downsample.hlsl", "VS", "PS", SM_AUTO ) );
	quad->setTexture( "texSource", source );
	ShaderParamBlock params;
	params.assign( "TargetDimensions", 0,  &Vector4( target->getWidth(), target->getHeight(), 0.0f, 0.0f ) );
	quad->setShaderParams( params );
	quad->render(false);

	// Revert back to original render target
	this->setRenderTarget( renderTarget );
}

void RenderSystem::beginFrame( float* clearColorRGBA, float depthClear, UINT8 stencilClear )
{
	drawcalls = 0;
	t_commands = 0.0f;
	t_alloc = 0.0f;
	t_dealloc = 0.0f;
	t_material = 0.0f;
	t_queuepush = 0.0f;
	t_queuepop = 0.0f;
	t_renderstarve = 0.0f;
	t_shaderparams = 0.0f;
	m_pDispatcher->frameFinished = false;
	getRenderDispatcher()->timer2.start();
	D3D11RenderCommand_BeginFrame rc;

	rc.setClearColor( clearColorRGBA );
	rc.setDepthStencilClear( depthClear, stencilClear );

	submit( &rc );
}

void RenderSystem::beginFrame()
{
	float ClearColor[4] = { 0.0f, 0.125f, 0.3f, 1.0f }; //red,green,blue,alpha
	beginFrame( ClearColor, 1.0f, 0 );
}

void RenderSystem::endFrame()
{
	D3D11RenderCommand_EndFrame  rc;
	
	submit( &rc );
}


void RenderSystem::present( UINT SyncInterval )
{
	m_pDispatcher->present( SyncInterval );
}

void present( RenderOutput* pOutput, UINT SyncInterval )
{

}

void RenderSystem::clearBackbuffer( float* clearColorRGBA )
{
	m_pDispatcher->clearBackbuffer( clearColorRGBA );
}

void RenderSystem::clearDepthStencil( float depth, UINT8 stencil )
{
	m_pDispatcher->clearDepthStencil( depth, stencil );
}

Mesh* RenderSystem::loadMesh( std::string filename, bool cache )
{
	if (cache)
	{
		// Find mesh in the cache
		std::map< std::string, Mesh* >::iterator it = m_pMeshes.find(filename);
		if (it != m_pMeshes.end())
			return new Mesh(*(*it).second);
		else
		{
			DEBUG_OUTPUT( "Loading " );
			DEBUG_OUTPUT( filename.c_str() );
			DEBUG_OUTPUT( "\n" );
			// If not found, create and cache it
			Mesh* mesh = MeshLoader::fromFile( (DX11RenderDispatcher*)m_pDispatcher, filename.c_str() );
			m_pMeshes[filename] = mesh;
			return mesh;
		}
	}
	else
	{
		DEBUG_OUTPUT( "Loading " );
		DEBUG_OUTPUT( filename.c_str() );
		DEBUG_OUTPUT( "\n" );
		return MeshLoader::fromFile( (DX11RenderDispatcher*)m_pDispatcher, filename.c_str() );
	}
}

Texture2D*	RenderSystem::loadTexture( std::wstring filename, bool cache )
{
	if (cache)
	{
		// Find texture in the cache
		std::map< std::wstring, Texture2D* >::iterator it = m_pTextures.find(filename);
		if (it != m_pTextures.end())
			return (*it).second;
		else
		{
			DEBUG_OUTPUT( "Loading " );
			DEBUG_OUTPUT( wstring_to_string(filename).c_str() );
			DEBUG_OUTPUT( "\n" );
			// If not found, create and cache it
			Texture2D* tex = createTextureFromFile( filename );
			if (tex)
				tex->setRenderSystem(this);
			m_pTextures[filename] = tex;
			return tex;
		}
	}
	else
	{
		DEBUG_OUTPUT( "Loading " );
		DEBUG_OUTPUT( wstring_to_string(filename).c_str() );
		DEBUG_OUTPUT( "\n" );
		Texture2D* tex = createTextureFromFile( filename );
		if (tex)
			tex->setRenderSystem(this);
		return tex;
	}
}


Shaderset* RenderSystem::loadShaderset( std::wstring filename, std::string vertexShader, std::string pixelShader, SHADERMODEL sm,  std::vector<ShaderMacro>* macros, bool debug, bool cache )
{
	if (cache)
	{
		ShadersetDescription desc;
		desc.filename = filename;
		desc.vertexShader = vertexShader;
		desc.pixelShader = pixelShader;
		desc.sm = sm;
		desc.debug = debug;

		// Find matching shaderset in the cache
		std::vector< std::pair<ShadersetDescription, Shaderset*> >::iterator it = std::find_if( m_pShadersets.begin(), m_pShadersets.end(), FindFirst<ShadersetDescription, Shaderset*>(desc) );
		if (it!=m_pShadersets.end())
		{
			// If found, return it
			return it->second;
		}
		else
		{
			DEBUG_OUTPUT( "Loading " );
			DEBUG_OUTPUT( wstring_to_string(filename).c_str() );
			DEBUG_OUTPUT( "\n" );
			// If not found, create and cache it
			Shaderset* ret = createShadersetFromFile( filename, vertexShader, pixelShader, sm, macros, debug );
			m_pShadersets.push_back( std::make_pair( desc, ret ) );
			return ret;
		}
	}
	else
	{
		DEBUG_OUTPUT( "Loading " );
		DEBUG_OUTPUT( wstring_to_string(filename).c_str() );
		DEBUG_OUTPUT( "\n" );
		return createShadersetFromFile( filename, vertexShader, pixelShader, sm, macros, debug );
	}
}


Mesh* RenderSystem::createBoxWireframeMesh(XMFLOAT3 dimensions)
{
	// Find box mesh with matching dimensions
	std::vector< std::pair<XMFLOAT3, Mesh*> >::iterator it = std::find_if( m_pBoxWireframeMeshes.begin(),  m_pBoxWireframeMeshes.end(), FindFirst<XMFLOAT3, Mesh*>( dimensions ) );
	
	// If we have it already, return it
	if (it!= m_pBoxWireframeMeshes.end())
		return it->second;

	else
	{
		// Make the mesh
		XMFLOAT3 halfExtents;
		halfExtents.x = dimensions.x/2.0f;
		halfExtents.y = dimensions.y/2.0f;
		halfExtents.z = dimensions.z/2.0f;

		// Define vertex format
		struct SimpleVertex
		{
			Vector3 Position;
			Vector2 Tex;
		};

		// Define vertex buffer
		SimpleVertex verts[] =
		{
			{ Vector3( -halfExtents.x, halfExtents.y, -halfExtents.z ), Vector2( 0.0f, 0.0f ), },// +-Y
			{ Vector3( halfExtents.x, halfExtents.y, -halfExtents.z ), Vector2( 1.0f, 0.0f ), },
			{ Vector3( halfExtents.x, halfExtents.y, halfExtents.z ), Vector2( 1.0f, 1.0f ), },
			{ Vector3( -halfExtents.x, halfExtents.y, halfExtents.z ), Vector2( 0.0f, 1.0f ), },

			{ Vector3( -halfExtents.x, -halfExtents.y, -halfExtents.z ), Vector2( 0.0f, 0.0f ), },
			{ Vector3( halfExtents.x, -halfExtents.y, -halfExtents.z ), Vector2( 1.0f, 0.0f ), },
			{ Vector3( halfExtents.x, -halfExtents.y, halfExtents.z ), Vector2( 1.0f, 1.0f ), },
			{ Vector3( -halfExtents.x, -halfExtents.y, halfExtents.z ), Vector2( 0.0f, 1.0f ), },
		};

		// Define index buffer
		UINT indices[] =
		{
			0,1,
			2,1,
	
			0,3,
			2,3,
					
			4,5,	
			5,6,

			4,7,
			6,7,
	
			0,4,
			1,5,

			2,6,
			3,7,
		};

		// Define vertex layout of the Vertex Buffer
		BufferLayout bufflayout;
		bufflayout.addElement( "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT );
		bufflayout.addElement( "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT );

		// Create the object
		Submesh* submesh = new Submesh();
		submesh->setGeometryChunk( this->createGeometryChunk( (float*)&verts, 
			bufflayout.getByteSize(), 
			bufflayout.getByteSize() * 8, 
			bufflayout, (UINT*)&indices, 
			24, 
			false, 
			D3D_PRIMITIVE_TOPOLOGY_LINELIST ) );
		Mesh* mesh = new Mesh();
		mesh->setName("Box Wireframe Mesh");
		mesh->addSubmesh( submesh );

		m_pBoxWireframeMeshes.push_back( std::make_pair(dimensions,mesh) );
		return mesh;
	}
}

Mesh* RenderSystem::createPlaneMesh(XMFLOAT2 dimensions, XMFLOAT2 uvscale)
{
	// Find box mesh with matching dimensions
	std::vector< std::pair< std::pair<XMFLOAT2,XMFLOAT2> , Mesh* > >::iterator it = std::find_if(  m_pPlaneMeshes.begin(),  m_pPlaneMeshes.end(), FindFirst<std::pair<XMFLOAT2,XMFLOAT2>, Mesh*>( std::make_pair(dimensions,uvscale) ) );
	
	// If we have it already, return it
	if (it!= m_pPlaneMeshes.end())
		return it->second;

	else
	{
		// Make the mesh
		XMFLOAT3 halfExtents;
		halfExtents.x = dimensions.x/2.0f;
		halfExtents.y = dimensions.y/2.0f;

		// Define vertex buffer with tangent space data
		Vertex verts[] =
		{
			{ Vector3( -halfExtents.x, -halfExtents.y, 0.0f ), Vector2( 0.0f, 0.0f ), Vector3( 0.0f, 0.0f, 1.0f ), Vector3( uvscale.x/fabs(uvscale.x), 0.0f, 0.0f ), Vector3( 0.0f, uvscale.y/fabs(uvscale.y), 0.0f ),  },// -Z
			{ Vector3( halfExtents.x, -halfExtents.y, 0.0f ), Vector2( uvscale.x, 0.0f ), Vector3( 0.0f, 0.0f, 1.0f ), Vector3( uvscale.x/fabs(uvscale.x), 0.0f, 0.0f ), Vector3( 0.0f, uvscale.y/fabs(uvscale.y), 0.0f ),  },
			{ Vector3( halfExtents.x, halfExtents.y, 0.0f ), Vector2( uvscale.x, uvscale.y ), Vector3( 0.0f, 0.0f, 1.0f ), Vector3( uvscale.x/fabs(uvscale.x), 0.0f, 0.0f ), Vector3( 0.0f, uvscale.y/fabs(uvscale.y), 0.0f ),  },
			{ Vector3( -halfExtents.x, halfExtents.y, 0.0f ), Vector2( 0.0f, uvscale.y ), Vector3( 0.0f, 0.0f, 1.0f ), Vector3( uvscale.x/fabs(uvscale.x), 0.0f, 0.0f ), Vector3( 0.0f, uvscale.y/fabs(uvscale.y), 0.0f ),  },
		};

		// Define index buffer
		UINT indices[] =
		{
			0,1,3,
			3,1,2,
		};

		BufferLayout bufflayout;
		bufflayout.addElement( "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT );
		bufflayout.addElement( "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT );
		bufflayout.addElement( "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT );
		bufflayout.addElement( "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT );
		bufflayout.addElement( "BITANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT );

		Submesh* submesh = new Submesh();
		submesh->setGeometryChunk( this->createGeometryChunk( (float*)&verts, 
									bufflayout.getByteSize(), 
									bufflayout.getByteSize() * 4, 
									bufflayout, 
									(UINT*)&indices, 
									6) );
		Mesh* mesh = new Mesh();
		mesh->setName("Plane Mesh");
		mesh->addSubmesh( submesh );

		m_pPlaneMeshes.push_back( std::make_pair(std::make_pair(dimensions,uvscale),mesh) );
		return mesh;
	}
}

Mesh* RenderSystem::createMesh( float* vertices, UINT numVertices, UINT* indices, UINT numIndices, BufferLayout vertexLayout, bool dynamic, D3D_PRIMITIVE_TOPOLOGY topology )
{
	// TODO : store mesh

	Submesh* submesh = new Submesh();
	submesh->setGeometryChunk( this->createGeometryChunk( vertices, 
								vertexLayout.getByteSize(), 
								vertexLayout.getByteSize() * numVertices, 
								vertexLayout, 
								indices, 
								numIndices, 
								dynamic,
								topology) );

	Mesh* mesh = new Mesh();
	if (dynamic)
		mesh->setName("Dynamic mesh created with RenderSystem::createMesh()");
	else
		mesh->setName("Static mesh created with RenderSystem::createMesh()");
	mesh->addSubmesh( submesh );

	return mesh;
}

Renderer* RenderSystem::createRenderer()
{
	/*if (m_CreationInfo.api==GRAPHICS_API_DX11)
	{
		Renderer* ptr = new DX11Renderer();
		ptr->setRenderSystem(this);
		return ptr;
	}
	else
	{*/
		Renderer* ptr = new Renderer();
		ptr->setRenderSystem(this);
		return ptr;
	//}
}