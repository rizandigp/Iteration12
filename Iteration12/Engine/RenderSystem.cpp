#pragma once

#include "RenderSystem.h"
#include "TextureCube.h"
#include "Array3D.h"
#include "FullscreenQuad.h"
#include "DX11\DX11RenderDispatcher.h"


RenderSystem::RenderSystem() :	m_pBoundingBoxWireframeMesh(NULL), 
								frameFinishedSemaphore(0), 
								m_Multithreaded(false)
{
}

// TODO : Don't use HRESULT
HRESULT RenderSystem::Initialize( RenderSystemConfig creationConfig )
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

	if (creationConfig.api = GRAPHICS_API_DX11 )
		m_pDispatcher = new DX11RenderDispatcher();
	else
	{
		NGERROR( "GL/MANTLE/VULKAN not implemented yet. Only DX11 implemented for now. Render System initialization failed." );
		return S_FALSE;
	}

	m_pDispatcher->Initialize( rdConfig );
	m_pDispatcher->m_pRenderSystem = this;

	m_RenderCommandAllocator.Initialize( 2048 );

	m_pRenderTarget = NULL;

	return S_OK;
};

void RenderSystem::SetOutputWindow( HWND hWnd, UINT width, UINT height )
{
	m_pDispatcher->SetOutputWindow( hWnd, width, height );
}

GeometryChunk*	 RenderSystem::CreateGeometryChunk( float* vertices, UINT stride, UINT byteWidth, BufferLayout layout, UINT *indices, UINT numIndices, bool dynamic, PRIMITIVE_TOPOLOGY topology )
{
	GeometryChunk* pGeom = m_pDispatcher->CreateGeometryChunk( vertices, stride, byteWidth, layout, indices, numIndices, dynamic, topology );
	if (pGeom)
		pGeom->SetRenderSystem( this );

	return pGeom;
}

Shaderset*	 RenderSystem::CreateShadersetFromFile( std::wstring filename, std::string vertexShader, std::string pixelShader, SHADERMODEL sm, std::vector<ShaderMacro>* macros, bool debug )
{
	// Empty vector
	std::vector<ShaderMacro> shaderMacros;
	if (macros!=NULL)
		shaderMacros = *macros;

	return m_pDispatcher->CreateShadersetFromFile( filename, vertexShader, pixelShader, sm, shaderMacros, debug );
}

Texture2D*	 RenderSystem::CreateTexture2DFromFile( std::wstring filename )
{
	return m_pDispatcher->CreateTexture2DFromFile( filename );
}

TextureCube* RenderSystem::CreateTextureCubeFromFile( std::wstring filename )
{
	return m_pDispatcher->CreateTextureCubeFromFile( filename );
}

Texture2D*	 RenderSystem::CreateTexture2D( UINT height, UINT width, TEXTURE_FORMAT format )
{
	Texture2D* tex = m_pDispatcher->CreateTexture2D( height, width, format );

	if (tex)
		tex->SetRenderSystem(this);
	
	return tex;
}

Texture2D*	 RenderSystem::CreateTexture2D( const Image* initialData )
{
	Texture2D* tex;
	if ( initialData != NULL)
		tex = m_pDispatcher->CreateTexture2D( initialData->Height(), 
											initialData->Width(), 
											R8G8B8A8_UNORM, 
											initialData->Data(), 
											initialData->Pitch(), 
											initialData->DataSize() );

	if (tex)
		tex->SetRenderSystem(this);
	
	return tex;
}

TextureCube* RenderSystem::CreateCubemap( const Image* faces[6] )
{
	// Check that all faces are of equal dimensions
	UINT width = faces[0]->Width();
	UINT height = faces[0]->Height();
	for (int i=1; i<6; i++)
	{
		if ((faces[i]->Width() != width)||(faces[i]->Height() != height))
		{
			NGWARNING( "Texture creation failed, all faces of a cubemap must be of equal dimensions. Returning NULL." );
			return NULL;
		}
	}

	TextureCube* tex = m_pDispatcher->CreateCubemap( faces );
	if (tex)
		tex->SetRenderSystem(this);
	
	return tex;
}

void RenderSystem::SetCamera( Camera3D* pCamera )
{
	m_pCamera = pCamera;
};

Camera3D* RenderSystem::GetCamera()
{
	return m_pCamera;
}

RenderDispatcher* RenderSystem::GetRenderDispatcher()
{
	return m_pDispatcher;
}

void RenderSystem::ConsumeRenderQueue( bool returnAfterFrameFinished )
{
	RenderCommand* rc;
	bool notEmpty = true;
	this->GetRenderDispatcher()->frameFinished = false;
	if (returnAfterFrameFinished)
	{
		Timer looptimer;
		Timer rcTimer;
		Timer deallocTimer;
		Timer popTimer;
		do
		{
			//looptimer.start();
			popTimer.Start();
			notEmpty = m_RenderQueue.pop( rc );
			if(notEmpty)
			{
				t_queuepop += popTimer.GetMicroseconds();
				rcTimer.Start();
				rc->Execute( this->m_pDispatcher );
				t_commands += rcTimer.GetMiliseconds();

				deallocTimer.Start();
				m_RenderCommandAllocator.Deallocate(rc);
				//delete rc;
				t_dealloc += deallocTimer.GetMiliseconds();
			}
			else
				t_renderstarve += popTimer.GetMiliseconds();
			//GetRenderDispatcher()->t_renderloop += looptimer.GetMiliseconds();
		}
		while (!this->GetRenderDispatcher()->frameFinished);
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
			popTimer.Start();
			notEmpty = m_RenderQueue.pop( rc );
			if(notEmpty)
			{
				t_queuepop += popTimer.GetMicroseconds();
				rcTimer.Start();
				rc->Execute( this->m_pDispatcher );
				t_commands += rcTimer.GetMiliseconds();

				deallocTimer.Start();
				m_RenderCommandAllocator.Deallocate(rc);
				//delete rc;
				t_dealloc += deallocTimer.GetMiliseconds();
			}
			else
				t_renderstarve += popTimer.GetMiliseconds();
			//GetRenderDispatcher()->t_renderloop += looptimer.GetMiliseconds();
		}
	}
	
	return;
}

void RenderSystem::SignalFrameFinished()
{
	frameFinishedSemaphore.post();
}

void RenderSystem::WaitForFrameToFinish()
{
	frameFinishedSemaphore.wait();
}

void RenderSystem::Submit( RenderCommand* pRenderCommand )
{
	if (m_Multithreaded)
	{
		// If multithreading is enabled, push this render command to the render queue 
		// to be executed on a dedicated render thread
		SubmitThreaded( pRenderCommand );
	}
	else
	{
		// We're running single-threaded, render directly
		Timer timer;
		pRenderCommand->Execute( m_pDispatcher );
		t_commands += timer.GetMiliseconds();
	}
}

void RenderSystem::SubmitThreaded( RenderCommand* pRenderCommand )
{
	//Submit( pRenderCommand );
	
	// Allocate
	Timer allocTimer;
	RenderCommand* ptr;
	do 
	{
		ptr = m_RenderCommandAllocator.Allocate();// (RenderCommand*) new RenderCommandBlock();//
	} 
	while (ptr==NULL);

	//memcpy(ptr, pRenderCommand, sizeof(RenderCommandBlock));
	pRenderCommand->Clone( ptr );
	t_alloc += allocTimer.GetMiliseconds();
	//ptr->m_InUse = true;

	// Push into render queue
	Timer pushTimer;
	bool pushed;
	do
	{
		pushed = m_RenderQueue.push( ptr );
	} 
	while (!pushed);
	
	//m_RenderQueue.push( ptr );
	t_queuepush += pushTimer.GetMicroseconds();
}

void RenderSystem::SetRenderTarget( Texture2D* pRenderTarget )
{
	m_pRenderTarget = pRenderTarget;
	D3D11RenderCommand_RenderTarget rc;
	rc.SetRenderTargetTexture( (DX11Texture2D*)pRenderTarget );

	Submit( &rc );
}

void RenderSystem::SetMultipleRenderTargets( UINT numRenderTargets, Texture2D** pRenderTargetArray )
{
	// Check that all render targets have the same dimensions
	UINT width = pRenderTargetArray[0]->GetWidth();
	UINT height = pRenderTargetArray[0]->GetHeight();
	for (int i=1; i<numRenderTargets; i++)
	{
		if((pRenderTargetArray[i]->GetWidth() != width)||(pRenderTargetArray[0]->GetHeight() != height))
		{
			NGWARNING( "Failed : Multiple Render Targets (MRTs) must have the same dimensions. Render target unchanged." );
			return;
		}
	}

	// Create command and submit
	D3D11RenderCommand_MRT rc;
	rc.SetRenderTargetTextures( numRenderTargets, (DX11Texture2D**)pRenderTargetArray );

	Submit( &rc );
}

void RenderSystem::SetBackbufferAsRenderTarget()
{
	// A NULL render target automatically sets it to the backbuffer
	D3D11RenderCommand_RenderTarget rc;
	rc.SetRenderTargetTexture( NULL );
	
	Submit( &rc );
}

void RenderSystem::ResolveMSAA( Texture2D* pDestination, Texture2D* pSource )
{
	D3D11RenderCommand_ResolveMSAA rc;
	rc.SetDestination( pDestination );
	rc.SetSource( pSource );

	Submit( &rc );
}

void RenderSystem::ClearTexture( Texture* texture, float* clearColorRGBA )
{
	D3D11RenderCommand_ClearTexture rc;
	rc.SetClearColor( clearColorRGBA );
	rc.SetTexture( texture );
	
	Submit( &rc );
}

void RenderSystem::ClearTexture( Texture* texture, Vector4 clearColorRGBA )
{
	float color[4] = { clearColorRGBA.x, clearColorRGBA.y, clearColorRGBA.z, clearColorRGBA.w };
	ClearTexture( texture, color );
}

void RenderSystem::DownsampleTexture( Texture2D* target, Texture2D* source )
{
	// TODO : put up a warning if the target resolution isn't right

	// Get our current bound render target so we can revert back to it later
	Texture2D* renderTarget = this->GetRenderTarget();

	// Set target texture as render target
	this->SetRenderTarget( target );
	float clearColor[4] = { 0.5f, 0.125f, 0.3f, 1.0f };
	this->ClearTexture( target, clearColor );

	// Fullscreen pass
	FullscreenQuad quad( this );
	//quad.SetShaderset( this->LoadShaderset( L"Shaders/Downsample.hlsl", "VS", "PS", SM_AUTO ) );
	//quad.SetTexture( "texSource", source );
	//ShaderParamBlock params;
	//params.assign( "TargetDimensions", 0,  &Vector4( target->GetWidth(), target->GetHeight(), 0.0f, 0.0f ) );
	//quad.SetShaderParams( params );
	//quad.Render(false);

	// Revert back to original render target
	this->SetRenderTarget( renderTarget );
}

void RenderSystem::BeginFrame( float* clearColorRGBA, float depthClear, UINT8 stencilClear )
{
	drawcalls = 0;
	t_commands = 0.0f;
	t_alloc = 0.0f;
	t_dealloc = 0.0f;
	t_material = 0.0f;
	t_shadowmapbinding = 0.0f;
	t_queuepush = 0.0f;
	t_queuepop = 0.0f;
	t_renderstarve = 0.0f;
	t_shaderparams = 0.0f;
	m_pDispatcher->frameFinished = false;
	GetRenderDispatcher()->timer2.Start();
	D3D11RenderCommand_BeginFrame rc;

	rc.SetClearColor( clearColorRGBA );
	rc.SetDepthStencilClear( depthClear, stencilClear );

	Submit( &rc );
}

void RenderSystem::BeginFrame()
{
	float ClearColor[4] = { 0.0f, 0.125f, 0.3f, 1.0f }; //red,green,blue,alpha
	BeginFrame( ClearColor, 1.0f, 0 );
}

void RenderSystem::EndFrame()
{
	D3D11RenderCommand_EndFrame  rc;
	
	Submit( &rc );
}


void RenderSystem::Present( UINT SyncInterval )
{
	m_pDispatcher->Present( SyncInterval );
}

void present( RenderOutput* pOutput, UINT SyncInterval )
{

}

void RenderSystem::ClearBackbuffer( float* clearColorRGBA )
{
	m_pDispatcher->ClearBackbuffer( clearColorRGBA );
}

void RenderSystem::ClearDepthStencil( float depth, UINT8 stencil )
{
	m_pDispatcher->ClearDepthStencil( depth, stencil );
}

Mesh* RenderSystem::LoadMesh( std::string filename, bool cache )
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
			Mesh* mesh = MeshLoader::FromFile( (DX11RenderDispatcher*)m_pDispatcher, filename.c_str() );
			m_pMeshes[filename] = mesh;
			return mesh;
		}
	}
	else
	{
		DEBUG_OUTPUT( "Loading " );
		DEBUG_OUTPUT( filename.c_str() );
		DEBUG_OUTPUT( "\n" );
		return MeshLoader::FromFile( (DX11RenderDispatcher*)m_pDispatcher, filename.c_str() );
	}
}

Texture2D*	RenderSystem::LoadTexture2D( std::wstring filename, bool cache )
{
	if (cache)
	{
		// Find texture in the cache
		std::map< std::wstring, Texture* >::iterator it = m_pTextures.find(filename);
		if (it != m_pTextures.end())
			return (Texture2D*)(*it).second;
		else
		{
			DEBUG_OUTPUT( "Loading " );
			DEBUG_OUTPUT( wstring_to_string(filename).c_str() );
			DEBUG_OUTPUT( "\n" );
			// If not found, create and cache it
			Texture2D* tex = CreateTexture2DFromFile( filename );
			if (tex)
			{
				tex->SetRenderSystem(this);
				m_pTextures[filename] = tex;
			}
			return tex;
		}
	}
	else
	{
		DEBUG_OUTPUT( "Loading " );
		DEBUG_OUTPUT( wstring_to_string(filename).c_str() );
		DEBUG_OUTPUT( "\n" );
		Texture2D* tex = CreateTexture2DFromFile( filename );
		if (tex)
			tex->SetRenderSystem(this);
		return tex;
	}
}

TextureCube* RenderSystem::LoadTextureCube( std::wstring filename, bool cache )
{
	if (cache)
	{
		// Find texture in the cache
		std::map< std::wstring, Texture* >::iterator it = m_pTextures.find(filename);
		if (it != m_pTextures.end())
			return (TextureCube*)(*it).second;
		else
		{
			DEBUG_OUTPUT( "Loading " );
			DEBUG_OUTPUT( wstring_to_string(filename).c_str() );
			DEBUG_OUTPUT( "\n" );
			// If not found, create and cache it
			TextureCube* tex = CreateTextureCubeFromFile( filename );
			if (tex)
			{
				tex->SetRenderSystem(this);
				m_pTextures[filename] = tex;
			}
			return tex;
		}
	}
	else
	{
		DEBUG_OUTPUT( "Loading " );
		DEBUG_OUTPUT( wstring_to_string(filename).c_str() );
		DEBUG_OUTPUT( "\n" );
		TextureCube* tex = CreateTextureCubeFromFile( filename );
		if (tex)
			tex->SetRenderSystem(this);
		return tex;
	}
}

Shaderset* RenderSystem::LoadShaderset( std::wstring filename, std::string vertexShader, std::string pixelShader, SHADERMODEL sm,  std::vector<ShaderMacro>* macros, bool debug, bool cache )
{
	if (cache)
	{
		ShadersetDescription desc;
		desc.filename = filename;
		desc.vertexShader = vertexShader;
		desc.pixelShader = pixelShader;
		desc.sm = sm;
		desc.debug = debug;
		if (macros!=NULL) desc.macros = *macros;

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
			Shaderset* ret = CreateShadersetFromFile( filename, vertexShader, pixelShader, sm, macros, debug );
			m_pShadersets.push_back( std::make_pair( desc, ret ) );
			return ret;
		}
	}
	else
	{
		DEBUG_OUTPUT( "Loading " );
		DEBUG_OUTPUT( wstring_to_string(filename).c_str() );
		DEBUG_OUTPUT( "\n" );
		return CreateShadersetFromFile( filename, vertexShader, pixelShader, sm, macros, debug );
	}
}


Mesh* RenderSystem::CreateBoxWireframeMesh(Vector3 dimensions)
{
	// Find box mesh with matching dimensions
	std::vector< std::pair<Vector3, Mesh*> >::iterator it = std::find_if( m_pBoxWireframeMeshes.begin(),  m_pBoxWireframeMeshes.end(), FindFirst<Vector3, Mesh*>( dimensions ) );
	
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
		bufflayout.AddElement( "POSITION", 0, R32G32B32_FLOAT );
		bufflayout.AddElement( "TEXCOORD", 0, R32G32_FLOAT );

		// Create the object
		Submesh* submesh = new Submesh();
		submesh->SetGeometryChunk( this->CreateGeometryChunk( (float*)&verts, 
			bufflayout.GetByteSize(), 
			bufflayout.GetByteSize() * 8, 
			bufflayout, (UINT*)&indices, 
			24, 
			false, 
			PRIMITIVE_TOPOLOGY_LINELIST ) );
		Mesh* mesh = new Mesh();
		mesh->SetName("Box Wireframe Mesh");
		mesh->AddSubmesh( submesh );

		m_pBoxWireframeMeshes.push_back( std::make_pair(dimensions,mesh) );
		return mesh;
	}
}

Mesh* RenderSystem::CreatePlaneMesh(Vector2 dimensions, Vector2 uvscale)
{
	// Find box mesh with matching dimensions
	std::vector< std::pair< std::pair<Vector2,Vector2> , Mesh* > >::iterator it = std::find_if(  m_pPlaneMeshes.begin(),  m_pPlaneMeshes.end(), FindFirst<std::pair<Vector2,Vector2>, Mesh*>( std::make_pair(dimensions,uvscale) ) );
	
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
		bufflayout.AddElement( "POSITION", 0, R32G32B32_FLOAT );
		bufflayout.AddElement( "TEXCOORD", 0, R32G32_FLOAT );
		bufflayout.AddElement( "NORMAL", 0, R32G32B32_FLOAT );
		bufflayout.AddElement( "TANGENT", 0, R32G32B32_FLOAT );
		bufflayout.AddElement( "BITANGENT", 0, R32G32B32_FLOAT );

		Submesh* submesh = new Submesh();
		submesh->SetGeometryChunk( this->CreateGeometryChunk( (float*)&verts, 
									bufflayout.GetByteSize(), 
									bufflayout.GetByteSize() * 4, 
									bufflayout, 
									(UINT*)&indices, 
									6) );
		Mesh* mesh = new Mesh();
		mesh->SetName("Plane Mesh");
		mesh->AddSubmesh( submesh );

		m_pPlaneMeshes.push_back( std::make_pair(std::make_pair(dimensions,uvscale),mesh) );
		return mesh;
	}
}

Mesh* RenderSystem::CreateMesh( float* vertices, UINT numVertices, UINT* indices, UINT numIndices, BufferLayout vertexLayout, bool dynamic, PRIMITIVE_TOPOLOGY topology )
{
	// TODO : store mesh

	Submesh* submesh = new Submesh();
	submesh->SetGeometryChunk( this->CreateGeometryChunk( vertices, 
								vertexLayout.GetByteSize(), 
								vertexLayout.GetByteSize() * numVertices, 
								vertexLayout, 
								indices, 
								numIndices, 
								dynamic,
								topology) );

	Mesh* mesh = new Mesh();
	if (dynamic)
		mesh->SetName("Dynamic mesh created with RenderSystem::CreateMesh()");
	else
		mesh->SetName("Static mesh created with RenderSystem::CreateMesh()");
	mesh->AddSubmesh( submesh );

	return mesh;
}

Texture2D* RenderSystem::CreateNoiseTexture( int Width, int Height )
{
	// Find texture with matching dimensions
	std::vector< std::pair< Vector2 , Texture2D* > >::iterator it = std::find_if(  m_NoiseTextures.begin(),  m_NoiseTextures.end(), FindFirst<Vector2, Texture2D*>( Vector2(Width, Height) ) );

	// If we have it already, return it
	if (it!= m_NoiseTextures.end())
		return it->second;

	else
	{
		Array2D<Vector2> noise( 2, 2, Vector2(1.0f, 0.0f));
		for (int i = 0; i < 2; ++i) 
		{
			for (int j = 0; j < 2; ++j) 
			{
			   Vector2 value = Vector2(
					random(0.0f, 1.0f),
					random(0.0f, 1.0f)
					);
				value.normalise();

				noise(i, j) = value;
			}
		}
		Texture2D* noiseTexture = this->CreateTexture2D( &noise, R32G32_FLOAT );
		m_NoiseTextures.push_back( std::make_pair( Vector2(Width, Height), noiseTexture ) );

		return noiseTexture;
	}
}

Renderer* RenderSystem::CreateRenderer()
{
	/*if (m_CreationInfo.api==GRAPHICS_API_DX11)
	{
		Renderer* ptr = new DX11Renderer();
		ptr->SetRenderSystem(this);
		return ptr;
	}
	else
	{*/
		Renderer* renderer = m_pDispatcher->CreateRenderer();
		renderer->SetRenderSystem(this);
		return renderer;
	//}
}