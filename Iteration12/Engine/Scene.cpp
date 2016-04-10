#pragma once

#include "Scene.h"
#include "LightVolume.h"
#include "Entity_Water.h"
#include "Camera3D.h"

void Scene::init( RenderSystem* pRendering, PhysicsSystem* pPhysics )
{
	setRenderSystem( pRendering );
	setPhysicsSystem( pPhysics );

	// G-buffers
	RenderSystemConfig config = m_pRenderSystem->getConfig();
	m_pGBuffer[0] = m_pRenderSystem->createTexture( config.Height, config.Width, R8G8B8A8_UNORM );
	m_pGBuffer[1] = m_pRenderSystem->createTexture( config.Height, config.Width, R32G32B32A32_FLOAT );
	m_pGBuffer[2] = m_pRenderSystem->createTexture( config.Height, config.Width, R8G8B8A8_UNORM );

	// HDR render target
	m_pHDRRenderTarget = m_pRenderSystem->createTexture( config.Height, config.Width, R16G16B16A16_FLOAT );

	// Fullscreen passes
	m_pFullscreenQuad = new FullscreenQuad( pRendering );
	m_pFullscreenQuad->setShaderset( m_pRenderSystem->loadShaderset( L"Shaders/Deferred.hlsl", "VS", "PS", SM_5_0 ) );
	m_pFullscreenQuad->setTexture( "gbuffer0", m_pGBuffer[0] );
	m_pFullscreenQuad->setTexture( "gbuffer1", m_pGBuffer[1] );
	m_pFullscreenQuad->setTexture( "gbuffer2", m_pGBuffer[2] );

	// Create noise texture for the SSAO
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

			noise(i, j) = value;//Vector2(0.0f, 1.0f);
		}
	}
	Texture2D *noiseTexture = m_pRenderSystem->createTexture( &noise, R32G32_FLOAT );

	// Half-res gbuffer[1] (normals and depth)
	m_pHalfRes = m_pRenderSystem->createTexture( config.Height/2, config.Width/2, R32G32B32A32_FLOAT );

	// Half-res SSAO buffer
	m_pSSAOBuffer = m_pRenderSystem->createTexture( config.Height/2, config.Width/2, R16_UNORM );

	m_pSSAOPass = new FullscreenQuad( pRendering );
	m_pSSAOPass->setShaderset( m_pRenderSystem->loadShaderset( L"Shaders/SSAO.hlsl", "VS", "PS", SM_5_0 ) );
	m_pSSAOPass->setTexture( "gbuffer0", m_pGBuffer[0] );
	m_pSSAOPass->setTexture( "gbuffer1", m_pHalfRes );
	m_pSSAOPass->setTexture( "texNoise", noiseTexture );

	m_pSSAOCombinePass = new FullscreenQuad( pRendering );
	m_pSSAOCombinePass->setShaderset( m_pRenderSystem->loadShaderset( L"Shaders/SSAOCombine.hlsl", "VS", "PS", SM_5_0 ) );
	m_pSSAOCombinePass->setTexture( "gbuffer0", m_pGBuffer[0] );
	m_pSSAOCombinePass->setTexture( "gbuffer1", m_pGBuffer[1] );
	m_pSSAOCombinePass->setTexture( "texSSAO", m_pSSAOBuffer );

		// Compile tonemapping shader with diferrent settings
	m_pTonemappingShaderset[0] = m_pRenderSystem->loadShaderset( L"Shaders/Tonemapping.hlsl", "VS", "PS", SM_5_0  );

	std::vector<ShaderMacro> macros;
	macros.push_back( ShaderMacro("TONEMAPPING_OPERATOR", "1") );
	m_pTonemappingShaderset[1] = m_pRenderSystem->loadShaderset( L"Shaders/Tonemapping.hlsl", "VS", "PS", SM_5_0, &macros, false );

	macros[0].Name = "TONEMAPPING_OPERATOR";
	macros[0].Definition = "2";
	m_pTonemappingShaderset[2] = m_pRenderSystem->loadShaderset( L"Shaders/Tonemapping.hlsl", "VS", "PS", SM_5_0, &macros, false );

	macros[0].Name = "TONEMAPPING_OPERATOR";
	macros[0].Definition = "3";
	m_pTonemappingShaderset[3] = m_pRenderSystem->loadShaderset( L"Shaders/Tonemapping.hlsl", "VS", "PS", SM_5_0, &macros, false );

	m_pTonemappingPass = new FullscreenQuad( m_pRenderSystem );
	m_pTonemappingPass->setShaderset( m_pTonemappingShaderset[0] );
	m_pTonemappingPass->setTexture( "texSource", m_pHDRRenderTarget );
	ShaderParamBlock shaderParams;
	shaderParams.assign( "fExposure", 0, 1.0f );
	m_pTonemappingPass->setShaderParams(shaderParams);

	m_pSunlight = new FullscreenQuad( pRendering );
	m_pSunlight->setShaderset( m_pRenderSystem->loadShaderset( L"Shaders/DeferredSunlight.hlsl", "VS", "PS", SM_5_0 ) );
	m_pSunlight->setTexture( "txGBuffer0", m_pGBuffer[0] );
	m_pSunlight->setTexture( "txGBuffer1", m_pGBuffer[1] );
	m_pSunlight->setTexture( "txGBuffer2", m_pGBuffer[2] );

	// Prototype global illumination
	//m_pGI = new FullscreenQuad( pRendering );
	//m_pGI->setShaderset( m_pRenderSystem->loadShaderset( L"Shaders/GI.hlsl", "VS", "PS", SM_5_0 ) );
	//m_pGI->setTexture( "txGBuffer0", m_pGBuffer[0] );
	//m_pGI->setTexture( "txGBuffer1", m_pGBuffer[1] );
	//m_pGI->setTexture( "txGBuffer2", m_pGBuffer[2] );

	// Shadowmaps
	m_pShadowmap[0] = m_pRenderSystem->createTexture( 256*8, 256*8, R32_FLOAT );/*
	m_pShadowmap[1] = m_pRenderSystem->createTexture( 256*4, 256*4, R32_FLOAT ); 
	m_pShadowmap[2] = m_pRenderSystem->createTexture( 256*4, 256*4, R32_FLOAT );
	m_pShadowmap[3] = m_pRenderSystem->createTexture( 256*4, 256*4, R32_FLOAT );
	m_pShadowmap[4] = m_pRenderSystem->createTexture( 256*2, 256*2, R32_FLOAT );
	m_pShadowmap[5] = m_pRenderSystem->createTexture( 256*2, 256*2, R32_FLOAT );
	m_pShadowmap[6] = m_pRenderSystem->createTexture( 256*2, 256*2, R32_FLOAT );
	m_pShadowmap[7] = m_pRenderSystem->createTexture( 256*2, 256*2, R32_FLOAT );*/

	m_pRSMNormal[0] = m_pRenderSystem->createTexture( 256*2, 256*2, R16G16B16A16_FLOAT );
	m_pRSMColor[0] = m_pRenderSystem->createTexture( 256*2, 256*2, R8G8B8A8_UNORM );

	m_pSMEmpty = m_pRenderSystem->createTexture( 1, 1, R32_FLOAT ); 
	
	float ClearColor[4] = { D3D11_FLOAT32_MAX, 1.0f, 1.0f, 1.0f };
	m_pRenderSystem->clearTexture( m_pShadowmap[0], ClearColor );
	m_pRenderSystem->clearTexture( m_pSMEmpty, ClearColor );

	//m_pTestTexture = m_pRenderSystem->loadTexture( L"Media/781484020226.jpg" );
	//m_pGBuffer[0] = m_pTestTexture;

	// Water surface
	//m_pWater = new Entity_Water( m_pRenderSystem );
	//m_pWater->setName( L"Water" );
	//m_pWater->transform()->translate(0.0f,0.0f,0.0f);
	//addEntity(m_pWater);

	// Skybox
	DX11Material_Skybox* mat = new DX11Material_Skybox( m_pRenderSystem );
	mat->setCubemap( m_pRenderSystem->loadTexture( L"Media/cubemap0.dds" ) );
	Mesh* mesh = m_pRenderSystem->loadMesh( "Media/Meshes/unit_cube_inverted.obj" );
	mesh->setMaterial( mat );
	Entity_StaticProp* skybox = new Entity_StaticProp( m_pRenderSystem, mesh );
	skybox->setName( L"Skybox" );
	skybox->transform()->setScale( 2000.0f, 2000.0f, 2000.0f );
	skybox->transform()->setPosition( 0.0f, 0.0f, 0.0f );
	addEntity( skybox );

	// Projector
	Camera3D* projector = new Camera3D();
	projector->setPosition( Vector3(10.0f, 10.0f, 5.0f) );
	projector->setLookAt( Vector3( 10.0f, 15.0f, 0.0f ) );
	projector->setUpVector( Vector3( 0.0f, 0.0f, 1.0f ) );
	projector->setProjection( XM_PI*0.35f, 1 / (float)1, 0.1f, 1000.0f );
	projector->update( 0.0f );

	//m_pWater->setGridProjectorCamera( projector );

	loadMeshes();
	loadPrefabs();
}

void Scene::setPhysicsSystem( PhysicsSystem* ptr )
{ 
	m_pPhysicsSystem = ptr;
}

void Scene::setRenderSystem( RenderSystem* pRenderSystem )
{
	m_pRenderSystem = pRenderSystem;
}

void Scene::setCamera( Camera3D* camera )
{
	m_pActiveCamera = camera;

	if (m_pWater)
		m_pWater->setGridProjectorCamera( camera );
};

void Scene::addEntity( Entity* ent )
{
	ent->onAddToScene(this); 
	m_pEntities.push_back(ent);
}

void Scene::update( float deltaTime )
{
	// Update everything
	// TODO : parallelize?
	setDeltaTime( deltaTime );
	m_Time += deltaTime;

	// Step physics
	Timer timer;
	if (m_pPhysicsSystem)
		m_pPhysicsSystem->step( deltaTime );
	t_physics = timer.getMiliseconds();

	// Container may dinamically change sizes (an objects's update() spawning/destroying entities, etc)
	// If we used iterators, they could get invalidated
	// TODO : handle case where an element in the container gets destroyed
	for( std::vector<Entity*>::size_type i = 0; i < m_pEntities.size(); ++i )
	{
		m_pEntities[i]->update(m_DeltaTime);
	}
	for ( std::vector<PointLight*>::iterator it2 = m_pPointLights.begin(); it2 != m_pPointLights.end(); ++it2 )
	{
		(*it2)->update();
		(*it2)->cull( &m_pActiveCamera->getFrustum() );
	}
	for ( std::vector<SpotLight*>::iterator it2 = m_pSpotLights.begin(); it2 != m_pSpotLights.end(); ++it2 )
	{
		(*it2)->update();
		(*it2)->cull( &m_pActiveCamera->getFrustum() );
	}

	Timer oceanTimer;
	//m_pWater->update( m_DeltaTime );
	t_ocean = oceanTimer.getMiliseconds();
}

void Scene::render()
{
	Timer sceneTimer, t;
	t_cull = 0.0f;

	//
	// Render shadowmaps
	//
	if (!m_pSpotLights.empty())
	{
		float clear[] = { 10000.0f, 1.0f, 1.0f, 1.0f };
		int i = 0;

		for ( std::vector<SpotLight*>::iterator it2 = m_pSpotLights.begin(); it2 != m_pSpotLights.end(); ++it2 )
		{
			if ((*it2)->isVisible()&&(*it2)->isEnabled())
			{
				if ((*it2)->isCastingShadow() && i < 8)
				{
					m_pRenderSystem->clearTexture( m_pShadowmap[i], clear );
					m_pRenderSystem->setRenderTarget( m_pShadowmap[i] );

					Camera3D shadowCamera;
					shadowCamera.setViewProjectionMatrix( (*it2)->getProjectionCamera()->getViewProjectionMatrix() );
					shadowCamera.setFrustum( &(*it2)->getFrustum() );

					for( std::vector<Entity*>::iterator it = m_pEntities.begin(); it != m_pEntities.end(); ++it )
					{
						sceneTimer.start();
						(*it)->cull( &(*it2)->getFrustum() );
						t_cull += sceneTimer.getMiliseconds();
						(*it)->renderShadowmap(&shadowCamera);
					}

					(*it2)->setShadowmap( m_pShadowmap[i] );
				}
				else
					(*it2)->setShadowmap( m_pSMEmpty );

			}
			i++;
		}
	}

	//
	// Render main scene
	//
	m_pRenderSystem->setCamera( m_pActiveCamera );
	//m_pRenderSystem->setMultipleRenderTargets( 3, m_pGBuffer );
	m_pRenderSystem->setRenderTarget( m_pHDRRenderTarget );
	//m_pRenderSystem->setBackbufferAsRenderTarget();

	for( std::vector<Entity*>::iterator it = m_pEntities.begin(); it != m_pEntities.end(); ++it )
	{
		sceneTimer.start();
		XNA::Frustum frustum = m_pActiveCamera->getFrustum();/*
		frustum.LeftSlope /= 2;
		frustum.BottomSlope /= 2;
		frustum.RightSlope /= 2;
		frustum.TopSlope /= 2;*/
		(*it)->clearLights();
		(*it)->cull( &frustum );
		for ( std::vector<PointLight*>::iterator it2 = m_pPointLights.begin(); it2 != m_pPointLights.end(); ++it2 )
		{
			if ((*it2)->isEnabled()&&(*it2)->isVisible())
				(*it)->cullLight( *it2 );
		}
		for ( std::vector<SpotLight*>::iterator it2 = m_pSpotLights.begin(); it2 != m_pSpotLights.end(); ++it2 )
		{
			if ((*it2)->isEnabled()&&(*it2)->isVisible())
				(*it)->cullLight( *it2 );
		}
		t_cull += sceneTimer.getMiliseconds();
		(*it)->render();
		//(*it)->renderBoundingBox();
	}

	// Fullscreen pass to backbuffer
	m_pRenderSystem->setBackbufferAsRenderTarget();
	//m_pFullscreenQuad->render();
	m_pTonemappingPass->render(false);

	t_scenerender = t.getMiliseconds();
}

void Scene::parallelRender()
{
	Timer sceneTimer, t;
	t_cull = 0.0f;

	//
	// Render shadowmaps
	//
	if (!m_pSpotLights.empty())
	{
		float clear[] = { 1.0f, 1.0f, 1.0f, 1.0f };
		int i = 0;

		for ( std::vector<SpotLight*>::iterator it2 = m_pSpotLights.begin(); it2 != m_pSpotLights.end(); ++it2 )
		{
			if ((*it2)->isVisible()&&(*it2)->isEnabled())
			{
				if ((*it2)->isCastingShadow() && 8)
				{
					m_pRenderSystem->clearTexture( m_pShadowmap[i], clear );
					m_pRenderSystem->setRenderTarget( m_pShadowmap[i] );

					Camera3D shadowCamera;
					shadowCamera.setViewProjectionMatrix( (*it2)->getProjectionCamera()->getViewProjectionMatrix() );
					shadowCamera.setFrustum( &(*it2)->getFrustum() );

					// lambda
					tbb::parallel_for_each( m_pEntities.begin(), m_pEntities.end(), [&](Entity* ptr)
					{
						sceneTimer.start();
						ptr->cull( &(*it2)->getFrustum() );
						t_cull += sceneTimer.getMiliseconds();
						ptr->renderShadowmap((*it2)->getProjectionCamera());
					});

					(*it2)->setShadowmap( m_pShadowmap[i] );
				}
				else
					(*it2)->setShadowmap( m_pSMEmpty );

			}
			i++;
		}
	}

	//
	// Render main scene
	//
	float clearColor[4] = { 0.0f, 0.125f, 0.3f, 1.0f }; //red,green,blue,alpha
	m_pRenderSystem->clearTexture( m_pHDRRenderTarget, clearColor );
	m_pRenderSystem->clearTexture( m_pGBuffer[0], clearColor );
	m_pRenderSystem->clearTexture( m_pGBuffer[1], clearColor );
	m_pRenderSystem->clearTexture( m_pGBuffer[2], clearColor );

	m_pRenderSystem->setCamera( m_pActiveCamera );
	Texture2D* MRT[] = { m_pHDRRenderTarget, m_pGBuffer[0], m_pGBuffer[1], m_pGBuffer[2], };
	m_pRenderSystem->setMultipleRenderTargets( 4, MRT );
	//m_pRenderSystem->setRenderTarget( m_pHDRRenderTarget );
	//m_pRenderSystem->setBackbufferAsRenderTarget();


	//for( std::vector<Entity*>::iterator it = m_pEntities.begin(); it != m_pEntities.end(); ++it )
	//{
		//Entity* ptr = *it;
	// lambda
	tbb::parallel_for_each( m_pEntities.begin(), m_pEntities.end(), [&](Entity* ptr)
	{
		XNA::Frustum frustum = m_pActiveCamera->getFrustum();/*
		frustum.LeftSlope /= 2;
		frustum.BottomSlope /= 2;
		frustum.RightSlope /= 2;
		frustum.TopSlope /= 2;*/
		ptr->clearLights();
		ptr->cull( &frustum );
		for ( std::vector<PointLight*>::iterator it2 = m_pPointLights.begin(); it2 != m_pPointLights.end(); ++it2 )
		{
			if ((*it2)->isEnabled()&&(*it2)->isVisible())
				ptr->cullLight( *it2 );
		}
		for ( std::vector<SpotLight*>::iterator it2 = m_pSpotLights.begin(); it2 != m_pSpotLights.end(); ++it2 )
		{
			if ((*it2)->isEnabled()&&(*it2)->isVisible())
				ptr->cullLight( *it2 );
		}
		//t_cull += sceneTimer.getMiliseconds();
		ptr->render();
		//ptr->renderBoundingBox();
	});
	
	m_pRenderSystem->setRenderTarget( m_pHDRRenderTarget );
	//m_pGBuffer[0] = m_pTestTexture;
	// Render lights
	for ( std::vector<SpotLight*>::iterator it2 = m_pSpotLights.begin(); it2 != m_pSpotLights.end(); ++it2 )
	{
		if ((*it2)->isEnabled()&&(*it2)->isVisible())
		{
			LightVolume volume( m_pRenderSystem, *it2 );
			//volume.setShadowmap( (*it2)->getShadowmap() );
			//volume.setCookie( (*it2)->getCookie() );
			volume.setGBuffer( m_pGBuffer );
			volume.render();
		}
	}

	// Fullscreen pass to backbuffer
	m_pRenderSystem->setBackbufferAsRenderTarget();
	//m_pFullscreenQuad->render();
	m_pTonemappingPass->render(false);

	t_scenerender = t.getMiliseconds();
}

void Scene::renderDeferred()
{
	Timer sceneTimer, t;
	t_cull = 0.0f;

	//
	// Render main scene into gbuffer
	//
	float clearColor[4] = { 0.0f, 0.125f, 0.3f, 1.0f }; //red,green,blue,alpha
	m_pRenderSystem->clearTexture( m_pHDRRenderTarget, clearColor );
	m_pRenderSystem->clearTexture( m_pGBuffer[0], clearColor );
	m_pRenderSystem->clearTexture( m_pGBuffer[1], clearColor );
	m_pRenderSystem->clearTexture( m_pGBuffer[2], clearColor );

	m_pRenderSystem->setCamera( m_pActiveCamera );
	Texture2D* MRT[] = { m_pHDRRenderTarget, m_pGBuffer[0], m_pGBuffer[1], m_pGBuffer[2], };
	m_pRenderSystem->setMultipleRenderTargets( 4, MRT );
	//m_pRenderSystem->setMultipleRenderTargets( 3, m_pGBuffer );
	//m_pRenderSystem->setRenderTarget( m_pHDRRenderTarget );
	//m_pRenderSystem->setBackbufferAsRenderTarget();


	// Render all entities
	std::for_each( m_pEntities.begin(), m_pEntities.end(), [&](Entity* ptr)
	{
		XNA::Frustum frustum = m_pActiveCamera->getFrustum();

		//t_cull += sceneTimer.getMiliseconds();
		ptr->cull( &frustum );
		//ptr->setCulled( false );
		ptr->render();
		//ptr->renderBoundingBox();
	});

	//m_pWater->render();

	//
	// Render lights
	//
	if (!m_pSpotLights.empty())
	{
		// It is crucial to clear shadowmaps to the max possible float value. 
		float clear0[] = { D3DX_16F_MAX, D3DX_16F_MAX, D3DX_16F_MAX, D3DX_16F_MAX };
		float clear1[]	= { 0.0f, 0.0f, 0.0f, 1.0f, };

		for ( std::vector<SpotLight*>::iterator it2 = m_pSpotLights.begin(); it2 != m_pSpotLights.end(); ++it2 )
		{
			if ((*it2)->isVisible()&&(*it2)->isEnabled())
			{
				// Render shadowmap/RSM
				if ((*it2)->isCastingShadow())
				{
					if ((*it2)->isRSMEnabled())
					{
						// Render Reflective Shadow Maps
						m_pRenderSystem->clearTexture( m_pShadowmap[0], clear0 );
						m_pRenderSystem->clearTexture( m_pRSMNormal[0], clear1 );
						m_pRenderSystem->clearTexture( m_pRSMColor[0], clear1 );
						Texture2D* MRT[] = { m_pShadowmap[0], m_pRSMNormal[0], m_pRSMColor[0], };
						m_pRenderSystem->setMultipleRenderTargets( 3, MRT );
					}
					else
					{
						// Render just the shadowmap
						m_pRenderSystem->clearTexture( m_pShadowmap[0], clear0 );
						m_pRenderSystem->setRenderTarget( m_pShadowmap[0] );
					}

					// Cull and render each scene entity into RSM/shadowmap
					std::for_each( m_pEntities.begin(), m_pEntities.end(), [&](Entity* ptr)
					{
						sceneTimer.start();
						ptr->cull( &(*it2)->getFrustum() );
						t_cull += sceneTimer.getMiliseconds();
						if ((*it2)->isRSMEnabled())
							ptr->renderRSM((*it2)->getProjectionCamera(), (*it2) );
						else
							ptr->renderShadowmap((*it2)->getProjectionCamera());
					});

					if ((*it2)->isRSMEnabled())
					{
						(*it2)->setShadowmap( m_pShadowmap[0] );
						(*it2)->setRSMNormal( m_pRSMNormal[0] );
						(*it2)->setRSMColor( m_pRSMColor[0] );
						m_pGI->setTexture( "txShadowmap", m_pShadowmap[0] );
						m_pGI->setTexture( "txRSMNormal", m_pRSMNormal[0] );
						m_pGI->setTexture( "txRSMColor", m_pRSMColor[0] );
					}
					else
					{
						(*it2)->setShadowmap( m_pShadowmap[0] );
					}
				}
				else
				{
					(*it2)->setShadowmap( m_pSMEmpty );
				}

				// Render light volume
				m_pRenderSystem->setRenderTarget( m_pHDRRenderTarget );
				LightVolume volume( m_pRenderSystem, *it2 );
				volume.setGBuffer( m_pGBuffer );
				volume.render();
				
				// Render GI!
				//if ((*it2)->isRSMEnabled())
					//m_pGI->render(true);
			}
		}
	}
	
	m_pRenderSystem->setRenderTarget( m_pHDRRenderTarget );
	
	/*ShaderParamBlock shaderParams;
	shaderParams.assign( "vLightVector", 0, &Vector4( Vector3(-0.8f, 0.0f, 1.0f).normalisedCopy() ) );
	shaderParams.assign( "vColor", 0, &Vector4( 1.0f, 1.0f, 0.75f, 1.0f ) );
	shaderParams.assign( "vEyePos", 0, &XMFLOAT4(m_pActiveCamera->getPosition().x, m_pActiveCamera->getPosition().y, m_pActiveCamera->getPosition().z, 1.0f) );
	shaderParams.assign( "InvViewProjection", 0, &m_pRenderSystem->getCamera()->getViewProjectionMatrix().inverse().transpose() );
	m_pSunlight->setShaderParams( shaderParams );
	m_pSunlight->render( true );
	*/
	
	m_pRenderSystem->downsampleTexture( m_pHalfRes, m_pGBuffer[1] );
	m_pRenderSystem->clearTexture( m_pSSAOBuffer, clearColor );
	m_pRenderSystem->setRenderTarget( m_pSSAOBuffer );

	// TODO : make it easier to get basic stuff like resolution
	RenderSystemConfig config = m_pRenderSystem->getConfig();
	ShaderParamBlock shaderParams;
	shaderParams.assign( "NoiseScale", 0, &Vector4( config.Width/2.0, config.Height/2.0, 0.0f, 0.0f ) );
	//shaderParams.assign( "ScreenDimensions", 0, &Vector4( config.Width/10.0f, config.Height/10.0f, 0.0f, 0.0f ) );
	shaderParams.assign( "View", 0, &m_pActiveCamera->getViewMatrix() );
	shaderParams.assign( "Projection", 0, &m_pActiveCamera->getProjectionMatrix() );
	shaderParams.assign( "InvProjection", 0, &m_pActiveCamera->getProjectionMatrix().inverse() );
	shaderParams.assign( "fFarPlane", 0, m_pActiveCamera->getFarPlane() );
	shaderParams.assign( "FovAndAspect", 0, &Vector4( m_pActiveCamera->getFov(), m_pActiveCamera->getAspect(), 0.0f, 0.0f ) );
	m_pSSAOPass->setShaderParams(shaderParams);
	m_pSSAOPass->render( false );

	m_pRenderSystem->setRenderTarget( m_pHDRRenderTarget );
	//ShaderParamBlock shaderParams2;
	//shaderParams2.assign( "ScreenDimensions", 0, &Vector4( config.Width/4.0f, config.Height/4.0f, 0.0f, 0.0f ) );
	//m_pSSAOCombinePass->setShaderParams(shaderParams2);
	m_pSSAOCombinePass->render( true );
	
	// Fullscreen pass to backbuffer
	m_pRenderSystem->setBackbufferAsRenderTarget();
	m_pTonemappingPass->render( false );

	t_scenerender = t.getMiliseconds();
}

void Scene::parallelRenderDeferred()
{
	Timer sceneTimer, t;
	t_cull = 0.0f;

	//
	// Render main scene into gbuffer
	//
	float clearColor[4] = { 0.0f, 0.125f, 0.3f, 1.0f }; //red,green,blue,alpha
	m_pRenderSystem->clearTexture( m_pHDRRenderTarget, clearColor );
	m_pRenderSystem->clearTexture( m_pGBuffer[0], clearColor );
	m_pRenderSystem->clearTexture( m_pGBuffer[1], clearColor );
	m_pRenderSystem->clearTexture( m_pGBuffer[2], clearColor );

	m_pRenderSystem->setCamera( m_pActiveCamera );
	Texture2D* MRT[] = { m_pHDRRenderTarget, m_pGBuffer[0], m_pGBuffer[1], m_pGBuffer[2], };
	m_pRenderSystem->setMultipleRenderTargets( 4, MRT );
	//m_pRenderSystem->setMultipleRenderTargets( 3, m_pGBuffer );
	//m_pRenderSystem->setRenderTarget( m_pHDRRenderTarget );
	//m_pRenderSystem->setBackbufferAsRenderTarget();


	// Render all entities
	tbb::parallel_for_each( m_pEntities.begin(), m_pEntities.end(), [&](Entity* ptr)
	{
		XNA::Frustum frustum = m_pActiveCamera->getFrustum();

		//t_cull += sceneTimer.getMiliseconds();
		ptr->cull( &frustum );
		ptr->render();
		//ptr->renderBoundingBox();
	});

	//m_pWater->render();

	//
	// Render lights
	//
	if (!m_pSpotLights.empty())
	{
		// It is crucial to clear shadowmaps to the max possible float value. 
		float clear0[] = { D3DX_16F_MAX, D3DX_16F_MAX, D3DX_16F_MAX, D3DX_16F_MAX };
		float clear1[]	= { 0.0f, 0.0f, 0.0f, 1.0f, };

		for ( std::vector<SpotLight*>::iterator it2 = m_pSpotLights.begin(); it2 != m_pSpotLights.end(); ++it2 )
		{
			if ((*it2)->isVisible()&&(*it2)->isEnabled())
			{
				// Render shadowmap/RSM
				if ((*it2)->isCastingShadow())
				{
					if ((*it2)->isRSMEnabled())
					{
						// Render Reflective Shadow Maps
						m_pRenderSystem->clearTexture( m_pShadowmap[0], clear0 );
						m_pRenderSystem->clearTexture( m_pRSMNormal[0], clear1 );
						m_pRenderSystem->clearTexture( m_pRSMColor[0], clear1 );
						Texture2D* MRT[] = { m_pShadowmap[0], m_pRSMNormal[0], m_pRSMColor[0], };
						m_pRenderSystem->setMultipleRenderTargets( 3, MRT );
					}
					else
					{
						// Render just the shadowmap
						m_pRenderSystem->clearTexture( m_pShadowmap[0], clear0 );
						m_pRenderSystem->setRenderTarget( m_pShadowmap[0] );
					}

					// Cull and render each scene entity into RSM/shadowmap
					tbb::parallel_for_each( m_pEntities.begin(), m_pEntities.end(), [&](Entity* ptr)
					{
						sceneTimer.start();
						ptr->cull( &(*it2)->getFrustum() );
						t_cull += sceneTimer.getMiliseconds();
						if ((*it2)->isRSMEnabled())
							ptr->renderRSM((*it2)->getProjectionCamera(), (*it2) );
						else
							ptr->renderShadowmap((*it2)->getProjectionCamera());
					});

					if ((*it2)->isRSMEnabled())
					{
						(*it2)->setShadowmap( m_pShadowmap[0] );
						(*it2)->setRSMNormal( m_pRSMNormal[0] );
						(*it2)->setRSMColor( m_pRSMColor[0] );
						m_pGI->setTexture( "txShadowmap", m_pShadowmap[0] );
						m_pGI->setTexture( "txRSMNormal", m_pRSMNormal[0] );
						m_pGI->setTexture( "txRSMColor", m_pRSMColor[0] );
					}
					else
					{
						(*it2)->setShadowmap( m_pShadowmap[0] );
					}
				}
				else
				{
					(*it2)->setShadowmap( m_pSMEmpty );
				}

				// Render light volume
				m_pRenderSystem->setRenderTarget( m_pHDRRenderTarget );
				LightVolume volume( m_pRenderSystem, *it2 );
				volume.setGBuffer( m_pGBuffer );
				volume.render();
				
				// Render GI!
				//if ((*it2)->isRSMEnabled())
					//m_pGI->render(true);
			}
		}
	}
	
	m_pRenderSystem->setRenderTarget( m_pHDRRenderTarget );
	
	/*ShaderParamBlock shaderParams;
	shaderParams.assign( "vLightVector", 0, &Vector4( Vector3(-0.8f, 0.0f, 1.0f).normalisedCopy() ) );
	shaderParams.assign( "vColor", 0, &Vector4( 1.0f, 1.0f, 0.75f, 1.0f ) );
	shaderParams.assign( "vEyePos", 0, &XMFLOAT4(m_pActiveCamera->getPosition().x, m_pActiveCamera->getPosition().y, m_pActiveCamera->getPosition().z, 1.0f) );
	shaderParams.assign( "InvViewProjection", 0, &m_pRenderSystem->getCamera()->getViewProjectionMatrix().inverse().transpose().intoXMFLOAT4X4() );
	m_pSunlight->setShaderParams( shaderParams );
	m_pSunlight->render( true );*/
	

	//m_pSSAOPass->render( true );

	// Fullscreen pass to backbuffer
	m_pRenderSystem->setBackbufferAsRenderTarget();
	m_pTonemappingPass->render(false);

	t_scenerender = t.getMiliseconds();
}

void Scene::loadCustomObjects()
{
	// DDO Helmet
	Entity_StaticProp* helmet = new Entity_StaticProp( m_pRenderSystem, m_pMeshes[L"mesh_ddo_helmet"] );
	helmet->setName( L"DDO Helmet" );
	helmet->transform()->setScale( 0.015f, 0.015f, 0.015f );
	helmet->transform()->setPosition( -10.0, 6.0f, 1.0f );
	helmet->transform()->setOrientation( Quaternion( PI/2.0f, Vector3(1.0f,0.0f,0.0f ) ) );
	helmet->transform()->rotate( Quaternion( PI/2.0f, Vector3(0.0f,1.0f,0.0f) ) );
	addEntity( helmet );

	// DDO AKS74U
	Entity_StaticProp* aks = new Entity_StaticProp( m_pRenderSystem, m_pMeshes[L"mesh_ddo_aks"] );
	aks->setName( L"DDO AKS74U" );
	aks->transform()->setScale( 0.15f, 0.15f, 0.15f );
	aks->transform()->setPosition( -10.0, 4.0f, 1.0f );
	aks->transform()->setOrientation( Quaternion( PI/2.0f, Vector3(1.0f,0.0f,0.0f ) ) );
	aks->transform()->rotate( Quaternion( PI/2.0f, Vector3(0.0f,1.0f,0.0f) ) );
	addEntity( aks );

	// Sphere
	Entity_StaticProp* sphere = new Entity_StaticProp( m_pRenderSystem, m_pMeshes[L"mesh_sphere"] );
	sphere->setName( L"Sphere" );
	sphere->transform()->setPosition( -10.0, 2.0f, 1.0f );
	addEntity( sphere );

	// Water
	//Entity_Water* water = new Entity_Water( m_pRenderSystem );
	//water->transform()->setPosition( 0.0f, -20.0f, 0.0f );
	//addEntity( water );
}

void Scene::loadFromFile( std::wstring filename )
{
	const int MAX_LINES = 2048;
	const int MAX_CHARS_PER_LINE = 512;
	const int MAX_TOKENS_PER_LINE = 20;
	const char* const DELIMITER = " ";

	std::wifstream stream;
	stream.open( filename );
	if (!stream.good())
	{
		DEBUG_OUTPUT( __FUNCTION__ );
		DEBUG_OUTPUT( " : Could not open file '" );
		DEBUG_OUTPUT( wstring_to_string(filename).c_str() );
		DEBUG_OUTPUT( "'." );
		return; // exit if file not found
	}

	if (m_pPhysicsSystem)
		m_pPhysicsSystem->getWorld()->lock();

	Entity* ent = NULL;
	Transform* transform = NULL;
	SpotLight* currentSpotLight = NULL;
	PointLight* currentPointLight = NULL;
	hkpRigidBody* rigidBody = NULL;
	bool entityInstantiated = false;
	int n = 0;
	while( !stream.eof() )
	{
		std::wstring line;
		wchar_t buf[MAX_CHARS_PER_LINE];
		stream.getline( buf, MAX_CHARS_PER_LINE );
		line = buf;
		n++;

		// parse the line
		wchar_t str[512], class_str[512], cookie_str[512];
		std::wstring class_string, cookie_string;
		float x,y,z,w;
		int r,g,b;
		float falloff;
		if (swscanf(line.c_str(),L"Object %s",&str)==1)
		{
			//std::wcout<<str<<std::endl;
			rigidBody = NULL;
		}
		else if (swscanf(line.c_str(),L"NGclass = %s",&class_str)==1)
		{
			class_string = class_str;
			
			if (class_string==L"ent_Tumba01")
			{
				currentSpotLight = NULL;
				currentPointLight = NULL;
				ent = new Entity_StaticProp(m_pRenderSystem, m_pMeshes[L"mesh_metro_tumba01"]);
				ent->setName(str);
				addEntity(ent);
				transform = ent->transform();
				rigidBody = NULL;
			}
			else if (class_string==L"ent_Lantai")
			{
				currentSpotLight = NULL;
				currentPointLight = NULL;
				//DX11Material_BlinnPhong* mat = new DX11Material_BlinnPhong( m_pRenderSystem );
				DX11Material_Deferred* mat = new DX11Material_Deferred( m_pRenderSystem );
				mat->setDiffusemap( m_pRenderSystem->loadTexture( L"Media/tile34_revol_floor2.1024c.bmp" ) );
				mat->setNormalmap( m_pRenderSystem->loadTexture( L"Media/tile34_revol_floor_normal.1024c.bmp" ) );
				mat->setSpecularMap( m_pRenderSystem->loadTexture( L"Media/tile34_revol_floor_gloss.1024c.bmp" ) );/*
				mat->setSpecularMap( m_pRenderSystem->loadTexture( L"Media/grey.bmp" ) );
				mat->setDiffusemap( m_pRenderSystem->createTextureFromFile( L"Media/wood34_wallconstr_01.2048.bmp" ) );
				mat->setNormalmap( m_pRenderSystem->createTextureFromFile( L"Media/wood34_wallconstr_01_normal.2048.bmp" ) );
				mat->setSpecularMap( m_pRenderSystem->createTextureFromFile( L"Media/wood34_wallconstr_01_bump.2048.bmp" ) );/*
				mat->setDiffusemap( m_pRenderSystem->createTextureFromFile( L"Media/metal_locker.2048.bmp" ) );
				mat->setNormalmap( m_pRenderSystem->createTextureFromFile( L"Media/metal_locker_normal.2048.bmp" ) );
				mat->setSpecularMap( m_pRenderSystem->createTextureFromFile( L"Media/metal_locker_bump.2048.bmp" ) );
				mat->setDiffusemap( m_pRenderSystem->createTextureFromFile( L"Media/Concrete/Metro_0047.bmp" ) );
				mat->setNormalmap( m_pRenderSystem->createTextureFromFile( L"Media/Concrete/Metro_0047n.bmp" ) );
				mat->setSpecularMap( m_pRenderSystem->createTextureFromFile( L"Media/Concrete/Metro_0047s.bmp" ) );
				mat->setDiffusemap( m_pRenderSystem->createTextureFromFile( L"Media/Concrete/Metro_0039.bmp" ) );
				mat->setNormalmap( m_pRenderSystem->createTextureFromFile( L"Media/Concrete/Metro_0039n.bmp" ) );
				mat->setSpecularMap( m_pRenderSystem->createTextureFromFile( L"Media/Concrete/Metro_0039s.bmp" ) );*/

				Mesh* mesh = m_pRenderSystem->createPlaneMesh( XMFLOAT2(10.0f,10.0f), XMFLOAT2(4.0f,4.0f) );
				mesh->setMaterial(mat);
				hkpRigidBody* rb = createBoxRigidBody( Vector3(5.0f,5.0f,0.5f), 0 );
				Transform rigidBodyRelative;
				rigidBodyRelative.setPosition(0.0f,0.0f,-0.5f);
				ent = new Entity_Prop(m_pRenderSystem, m_pPhysicsSystem, mesh, rb, rigidBodyRelative);
				ent->setName(str);
				addEntity(ent);
				transform = ent->transform();
				rigidBody = rb;
			}
			else if (class_string==L"ent_Langitlangit")
			{
				currentSpotLight = NULL;
				currentPointLight = NULL;
				//DX11Material_BlinnPhong* mat = new DX11Material_BlinnPhong( m_pRenderSystem );
				DX11Material_Deferred* mat = new DX11Material_Deferred( m_pRenderSystem );
				mat->setDiffusemap( m_pRenderSystem->loadTexture( L"Media/concrete_opalubka_3.1024.bmp" ) );
				mat->setNormalmap( m_pRenderSystem->loadTexture( L"Media/concrete_opalubka_3_normal.1024.bmp" ) );
				mat->setSpecularMap( m_pRenderSystem->loadTexture( L"Media/concrete_opalubka_3_bump.1024.bmp" ) );
				
				ent = new Entity_Plane(m_pRenderSystem,mat,XMFLOAT2(10.0f,10.0f),XMFLOAT2(3.33f,3.33f));
				ent->setName(str);
				addEntity(ent);
				transform = ent->transform();
				rigidBody = NULL;
			}
			else if (class_string==L"ent_Tembok")
			{
				currentSpotLight = NULL;
				currentPointLight = NULL;
				//DX11Material_BlinnPhong* mat = new DX11Material_BlinnPhong( m_pRenderSystem );
				DX11Material_Deferred* mat = new DX11Material_Deferred( m_pRenderSystem );
				mat->setDiffusemap( m_pRenderSystem->loadTexture( L"Media/wall_stucaturka_stena1.1024c.bmp" ) );
				mat->setNormalmap( m_pRenderSystem->loadTexture( L"Media/wall_stucaturka_stena1_normal.2048.bmp" ) );
				mat->setSpecularMap( m_pRenderSystem->loadTexture( L"Media/wall_stucaturka_stena1_bump.2048.bmp" ) );

				//ent = new Entity_Plane( m_pRenderSystem, mat, XMFLOAT2(10.0f,3.0f),XMFLOAT2(-3.33f,-1.0f) );
				Mesh* mesh = m_pRenderSystem->createPlaneMesh( XMFLOAT2(10.0f,3.0f),XMFLOAT2(-3.33f,-1.0f) );
				mesh->setMaterial(mat);
				hkpRigidBody* rb = createBoxRigidBody( Vector3(5.0f,1.5f,0.1f), 0 );
				ent = new Entity_Prop(m_pRenderSystem, m_pPhysicsSystem, mesh, rb);
				ent->setName(str);
				addEntity(ent);
				transform = ent->transform();
				rigidBody = rb;
			}
			else if (class_string==L"SpotLight")
			{
				currentPointLight = NULL;
				currentSpotLight = new SpotLight();
				transform = currentSpotLight->transform();
				addSpotLight( currentSpotLight );
				rigidBody = NULL;
			}
			else if (class_string==L"PointLight")
			{
				currentSpotLight = NULL;
				currentPointLight = new PointLight();
				transform = currentPointLight->transform();
				addPointLight( currentPointLight );
				rigidBody = NULL;
			}
			else if (m_pMeshes.find(class_string)!=m_pMeshes.end())
			{
				currentSpotLight = NULL;
				currentPointLight = NULL;
				Mesh* mesh = m_pMeshes[class_string];
				ent = new Entity_StaticProp( m_pRenderSystem, mesh );
				ent->setName(str);
				addEntity(ent);
				transform = ent->transform();
				rigidBody = NULL;
			}
			else if (m_pPrefabs.find(class_string)!=m_pPrefabs.end())
			{
				currentSpotLight = NULL;
				currentPointLight = NULL;
				Entity_Prop* entity = new Entity_Prop(*m_pPrefabs[class_string]);	// Copy constructor
				entity->setName(str);
				addEntity(entity);
				transform = entity->transform();
				rigidBody = entity->m_pRigidBody;
			}
			else
			{
				transform = NULL;
				rigidBody = NULL;
			}
		}
		else if (swscanf(line.c_str(),L"rotation = (quat %f %f %f %f)",&x,&y,&z,&w)==4)
		{
			if (transform)
				transform->setOrientation(w,x,y,z);
			if (rigidBody)
				rigidBody->setRotation(hkQuaternion(x,y,z,w));
		}
		else if (swscanf(line.c_str(),L"position = [%f,%f,%f]",&x,&y,&z)==3)
		{
			if (transform)
				transform->setPosition(x,y,z);
			if (rigidBody)
				rigidBody->setPosition(hkVector4(x,y,z));
		}
		else if (swscanf(line.c_str(),L"scale = [%f,%f,%f]",&x,&y,&z)==3)
		{
			if (transform)
				transform->setScale(x,y,z);
			if (rigidBody)
			{
				transform->setScale(1.0f,1.0f,1.0f);
				/*
				hkArray<hkpShapeScalingUtility::ShapePair> shapes;
				// Scale havok's shape representation
				hkpShape* originalShape = (hkpShape*)rigidBody->getCollidable()->getShape();
				hkpShape* scaledShape = hkpShapeScalingUtility::scaleShape( originalShape, x, &shapes );
				rigidBody->setShape( scaledShape );
				rigidBody->updateShape();
				
				if (!rigidBody->isFixedOrKeyframed())
				{
					// Recompute inertia tensor
					hkMassProperties massProperties;
					hkpInertiaTensorComputer::computeShapeVolumeMassProperties(scaledShape,rigidBody->getMass(), massProperties );
					rigidBody->setCenterOfMassLocal(massProperties.m_centerOfMass);
					rigidBody->setInertiaLocal(massProperties.m_inertiaTensor);
				}*/
			}
		}
		// Lights
		else if (swscanf(line.c_str(),L"target = [%f,%f,%f]",&x,&y,&z)==3)
		{
			if (currentSpotLight)
				currentSpotLight->pointTo( Vector3(x,y,z), 0.0f );
		}
		else if (swscanf(line.c_str(),L"color = (color %i %i %i)",&r,&g,&b)==3)
		{
			if (currentSpotLight)
				currentSpotLight->setColor( Vector3(r/255.0f,g/255.0f,b/255.0f) );
			else if (currentPointLight)
				currentPointLight->setColor( Vector3(r/255.0f,g/255.0f,b/255.0f) );
		}
		else if (swscanf(line.c_str(),L"radius = %f",&x)==1)
		{
			if (currentSpotLight)
				currentSpotLight->setRadius(x);
			else if (currentPointLight)
				currentPointLight->setRadius(x);
		}
		else if (swscanf(line.c_str(),L"intensity = %f",&x)==1)
		{
			if (currentSpotLight)
				currentSpotLight->setIntensity(x*15.0f);
			else if (currentPointLight)
				currentPointLight->setIntensity(x*15.0f);
		}
		else if (swscanf(line.c_str(),L"falloff = %f",&x)==1)
		{
			falloff = x;
		}
		else if (swscanf(line.c_str(),L"aspect = %f",&x)==1)
		{
			if (currentSpotLight)
				currentSpotLight->setCone(Vector2((falloff*x)*D3DX_PI/180.0f,(falloff/x)*D3DX_PI/180.0f));
		}
		else if (swscanf(line.c_str(),L"cookie = %s",&cookie_str)==1)
		{
			cookie_string = cookie_str;
			if (currentSpotLight)
			{
				Texture2D* tex = m_pRenderSystem->loadTexture( cookie_string );
				if (tex)
					currentSpotLight->setCookie( tex );
				else
					currentSpotLight->setCookie( m_pRenderSystem->loadTexture( L"Media/notexture.bmp" ) );
			}
		}
	}
	
	if (m_pPhysicsSystem)
		m_pPhysicsSystem->getWorld()->unlock();
	stream.close();
}

void Scene::loadMeshes()
{
	
	RenderSystem *ptr = m_pRenderSystem;
	
	//--metro_kursi
	//props_bibl_stul
	//D:/Metro Last Light/content/textures/props/props_bibl_stul.512

	DX11Material_DiffuseDetailbump* mat0 = new DX11Material_DiffuseDetailbump(ptr);
	mat0->setDiffusemap( ptr->loadTexture( L"Media/props_bibl_stul.512.bmp" ) );
	mat0->setNormalmap( ptr->loadTexture( L"Media/props_bibl_stul_normal.1024.bmp" ) );
	mat0->setDetailNormalmap( ptr->loadTexture( L"Media/Tex_0010_5.dds" ) );
	mat0->setDetailNormalStrength( 2.0f );
	mat0->setDetailTiling( 10.0f );
	mat0->setSpecularIntensity( 0.34f );
	mat0->setSpecularPower( 32.0f );

	Mesh* pMesh = ptr->loadMesh( "Media/Meshes/props_bibl_stul.obj" );
	pMesh->setMaterial( mat0 );
	m_pMeshes[L"props_bibl_stul"] = pMesh;


	//props_chair_01
	//D:/Metro Last Light/content/textures/props/props_chair_01.512
	//D:/Metro Last Light/content/textures/props/props_chair_01.512

	mat0 = new DX11Material_DiffuseDetailbump(ptr);
	mat0->setDiffusemap( ptr->loadTexture( L"Media/props_chair_01.512.bmp" ) );
	mat0->setNormalmap( ptr->loadTexture( L"Media/props_chair_01_normal.512.bmp" ) );
	mat0->setDetailNormalmap( ptr->loadTexture( L"Media/Tex_0010_5.dds" ) );
	mat0->setDetailNormalStrength( 3.0f );
	mat0->setDetailTiling( 8.0f );
	mat0->setSpecularIntensity( 0.14f );
	mat0->setSpecularPower( 8.0f );

	pMesh = ptr->loadMesh( "Media/Meshes/props_chair_01.obj" );
	pMesh->setMaterial( mat0 );
	m_pMeshes[L"props_chair_01"] = pMesh;

	//props_chair_school_a
	//D:/Metro Last Light/content/textures/props/props_chair_school.512

	mat0 = new DX11Material_DiffuseDetailbump(ptr);
	mat0->setDiffusemap( ptr->loadTexture( L"Media/props_chair_school.512.bmp" ) );
	mat0->setNormalmap( ptr->loadTexture( L"Media/props_chair_school_normal.1024.bmp" ) );
	mat0->setDetailNormalmap( ptr->loadTexture( L"Media/Tex_0010_5.dds" ) );
	mat0->setDetailNormalStrength( 2.0f );
	mat0->setDetailTiling( 10.0f );
	mat0->setSpecularIntensity( 0.34f );
	mat0->setSpecularPower( 32.0f );

	pMesh = ptr->loadMesh( "Media/Meshes/props_chair_school_a.obj" );
	pMesh->setMaterial( mat0 );
	m_pMeshes[L"props_chair_school_a"] = pMesh;


	//props_chair_school_b
	//D:/Metro Last Light/content/textures/props/props_chair_school.512
	mat0 = new DX11Material_DiffuseDetailbump(ptr);
	mat0->setDiffusemap( ptr->loadTexture( L"Media/props_chair_school.512.bmp" ) );
	mat0->setNormalmap( ptr->loadTexture( L"Media/props_chair_school_normal.1024.bmp" ) );
	mat0->setDetailNormalmap( ptr->loadTexture( L"Media/Tex_0010_5.dds" ) );
	mat0->setDetailNormalStrength( 2.0f );
	mat0->setDetailTiling( 10.0f );
	mat0->setSpecularIntensity( 0.34f );
	mat0->setSpecularPower( 32.0f );

	pMesh = ptr->loadMesh( "Media/Meshes/props_chair_school_b.obj" );
	pMesh->setMaterial( mat0 );
	m_pMeshes[L"props_chair_school_b"] = pMesh;

	//--metro_lockers
	//metro_locker_a ~ metro_locker_e
	//D:/Metro Last Light/content/textures/metal/metal_locker.512
	DX11Material_Deferred *mat0b = new DX11Material_Deferred(ptr);
	//DX11Material_BlinnPhong *mat0b = new DX11Material_BlinnPhong(ptr);
	mat0b->setDiffusemap( ptr->loadTexture( L"Media/metal_locker.2048.bmp" ) );
	mat0b->setNormalmap( ptr->loadTexture( L"Media/metal_locker_normal.2048.bmp" ) );
	mat0b->setSpecularMap( ptr->loadTexture( L"Media/metal_locker_bump.2048.bmp" ) );
	//mat0->setDetailNormalmap( ptr->loadTexture( L"Media/Tex_0010_5.dds" ) );
	//mat0->setDetailNormalStrength( 2.0f );
	//mat0->setDetailTiling( 10.0f );
	//mat0->setSpecularIntensity( 0.34f );
	//mat0->setSpecularPower( 32.0f );
	mat0b->setSpecularIntensity( 0.5f );
	//mat0b->setSpecularPower( 1024.0f );
	mat0b->setGlossiness( 1024.0f );

	pMesh = ptr->loadMesh( "Media/Meshes/metro_locker_a.obj" );
	pMesh->setMaterial( mat0b );
	m_pMeshes[L"metro_locker_a"] = pMesh;

	pMesh = ptr->loadMesh( "Media/Meshes/metro_locker_b.obj" );
	pMesh->setMaterial( mat0b );
	m_pMeshes[L"metro_locker_b"] = pMesh;

	pMesh = ptr->loadMesh( "Media/Meshes/metro_locker_c.obj" );
	pMesh->setMaterial( mat0b );
	m_pMeshes[L"metro_locker_c"] = pMesh;

	pMesh = ptr->loadMesh( "Media/Meshes/metro_locker_d.obj" );
	pMesh->setMaterial( mat0b );
	m_pMeshes[L"metro_locker_d"] = pMesh;

	pMesh = ptr->loadMesh( "Media/Meshes/metro_locker_e.obj" );
	pMesh->setMaterial( mat0b );
	m_pMeshes[L"metro_locker_e"] = pMesh;

	pMesh = ptr->loadMesh( "Media/Meshes/metro_locker_f.obj" );
	pMesh->setMaterial( mat0b );
	m_pMeshes[L"metro_locker_f"] = pMesh;


	//--Clutter_paper_cardboard
	//Clutter_paper_cardboard_a ~ Clutter_paper_cardboard_e
	//Tex_0013_1.dds
	{
		DX11Material_Deferred* mat0 = new DX11Material_Deferred(ptr);
		mat0->setDiffusemap( ptr->loadTexture( L"Media/Prop_Pack_V1/Clutter_paper_cardboard/Tex_0013_1.dds" ) );
		mat0->setNormalmap( ptr->loadTexture( L"Media/Prop_Pack_V1/Clutter_paper_cardboard/normal.bmp" ) );
		//mat0->setDetailNormalmap( ptr->loadTexture( L"Media/Tex_0010_5.dds" ) );
		//mat0->setDetailTiling( 6.0f );
		//mat0->setDetailNormalStrength( 3.0f );
		mat0->setSpecularIntensity( 0.1f );
		//mat0->setSpecularPower( 12.0f );

		pMesh = ptr->loadMesh( "Media/Meshes/Clutter_paper_cardboard_a.obj" );
		pMesh->setMaterial( mat0 );
		m_pMeshes[L"Clutter_paper_cardboard_a"] = pMesh;

		pMesh = ptr->loadMesh( "Media/Meshes/Clutter_paper_cardboard_b.obj" );
		pMesh->setMaterial( mat0 );
		m_pMeshes[L"Clutter_paper_cardboard_b"] = pMesh;

		pMesh = ptr->loadMesh( "Media/Meshes/Clutter_paper_cardboard_c.obj" );
		pMesh->setMaterial( mat0 );
		m_pMeshes[L"Clutter_paper_cardboard_c"] = pMesh;

		pMesh = ptr->loadMesh( "Media/Meshes/Clutter_paper_cardboard_d.obj" );
		pMesh->setMaterial( mat0 );
		m_pMeshes[L"Clutter_paper_cardboard_d"] = pMesh;

		pMesh = ptr->loadMesh( "Media/Meshes/Clutter_paper_cardboard_e.obj" );
		pMesh->setMaterial( mat0 );
		m_pMeshes[L"Clutter_paper_cardboard_e"] = pMesh;
	}

	//--Pallet_Single
	//Pallet_Single
	//Tex_0030_1.dds
	{
		DX11Material_Deferred *mat = new DX11Material_Deferred(ptr);
		mat->setDiffusemap( ptr->loadTexture( L"Media/Prop_Pack_V1/Pallet_Single/Tex_0030_1.dds" ) );
		mat->setNormalmap( ptr->loadTexture( L"Media/flat.bmp" ) );
		//mat->setDetailNormalmap( ptr->loadTexture( L"Media/Tex_0010_5.dds" ) );
		//mat->setDetailTiling( 6.0f );
		//mat->setDetailNormalStrength( 3.0f );
		mat->setSpecularIntensity( 0.1f );
		//mat->setSpecularPower( 12.0f );

		pMesh = ptr->loadMesh( "Media/Meshes/Pallet_Single.obj" );
		pMesh->setMaterial( mat0 );
		m_pMeshes[L"Pallet_Single"] = pMesh;
	}

		//--props_m_scaf
		//props_m_scaf_a
		//D:/Metro Last Light/content/textures/props/props_m_scaf01.512
		//D:/Metro Last Light/content/textures/props/props_m_scaf02.512
		//props_m_scaf_b
		//D:/Metro Last Light/content/textures/props/props_m_scaf01.512
		//D:/Metro Last Light/content/textures/props/props_m_scaf02.512
	{
		DX11Material_Deferred *mat_b0 = new DX11Material_Deferred(ptr);
		//mat0 = new Material_DiffuseDetailbump(ptr);
		mat_b0->setDiffusemap( ptr->loadTexture( L"Media/props_m_scaf01.512.bmp" ) );
		mat_b0->setNormalmap( ptr->loadTexture( L"Media/props_m_scaf01_normal.512.bmp" ) );
		mat_b0->setSpecularMap( ptr->loadTexture( L"Media/props_m_scaf01_bump.512.bmp" ) );
		//mat0->setDetailNormalmap( ptr->loadTexture( L"Media/Tex_0010_5.dds" ) );
		//mat0->setDetailNormalStrength( 2.0f );
		//mat0->setDetailTiling( 10.0f );
		mat_b0->setSpecularIntensity( 0.7f );
		//mat_b0->setSpecularPower( 512.0f );

		DX11Material_Deferred* mat_b1 = new DX11Material_Deferred(ptr);
		mat_b1->setDiffusemap( ptr->loadTexture( L"Media/props_m_scaf02.512.bmp" ) );
		mat_b1->setNormalmap( ptr->loadTexture( L"Media/props_m_scaf02_normal.1024.bmp" ) );
		mat_b1->setSpecularMap( ptr->loadTexture( L"Media/props_m_scaf02_bump.1024.bmp" ) );
		//mat1->setDetailNormalmap( ptr->loadTexture( L"Media/Tex_0010_5.dds" ) );
		//mat1->setDetailNormalStrength( 2.0f );
		//mat1->setDetailTiling( 10.0f );
		mat_b1->setSpecularIntensity( 0.7f );
		//mat_b1->setSpecularPower( 512.0f );

		pMesh = ptr->loadMesh( "Media/Meshes/props_m_scaf_a.obj" );
		pMesh->setMaterial( mat_b0 );
		pMesh->getSubmesh(1)->setMaterial( mat_b1 );
		m_pMeshes[L"props_m_scaf_a"] = pMesh;

		pMesh = ptr->loadMesh( "Media/Meshes/props_m_scaf_b.obj" );
		pMesh->setMaterial( mat_b0 );
		pMesh->getSubmesh(1)->setMaterial( mat_b1 );
		m_pMeshes[L"props_m_scaf_b"] = pMesh;
	}

	//--metro_boxes02
	//wood_box_a
	//D:/Metro Last Light/content/textures/props/props_books_01.512
	//D:/Metro Last Light/content/textures/wood/wood_box_2.512
	//wood_box_b
	//D:/Metro Last Light/content/textures/wood/wood_box_2.512
	//wood_box_c
	//D:/Metro Last Light/content/textures/props/props_books_01.512
	//D:/Metro Last Light/content/textures/wood/wood_box.512
	//wood_box_d
	//D:/Metro Last Light/content/textures/props/props_books_01.512
	//D:/Metro Last Light/content/textures/wood/wood_box.512
	//wood_box_e
	//D:/Metro Last Light/content/textures/props/props_books_01.512
	//D:/Metro Last Light/content/textures/wood/wood_box.512
	{
		DX11Material_Deferred* mat0 = new DX11Material_Deferred(ptr);
		mat0->setDiffusemap( ptr->loadTexture( L"Media/props_books_01.512.bmp" ) );
		mat0->setNormalmap( ptr->loadTexture( L"Media/props_books_01_normal.512.bmp" ) );
		//mat0->setDetailNormalmap( ptr->loadTexture( L"Media/Tex_0010_5.dds" ) );
		//mat0->setDetailNormalStrength( 2.0f );
		//mat0->setDetailTiling( 10.0f );
		mat0->setSpecularIntensity( 0.34f );
		mat0->setGlossiness( 32.0f );
		//mat0->setSpecularPower( 32.0f );

		//DX11Material_DiffuseDetailbump* mat1 = new DX11Material_DiffuseDetailbump(ptr);
		DX11Material_Deferred* mat1 = new DX11Material_Deferred(ptr);
		mat1->setDiffusemap( ptr->loadTexture( L"Media/wood_box_2.1024.bmp" ) );
		mat1->setNormalmap( ptr->loadTexture( L"Media/wood_box_2_normal.1024.bmp" ) );
		//mat1->setDetailNormalmap( ptr->loadTexture( L"Media/Tex_0010_5.dds" ) );
		//mat1->setDetailNormalStrength( 2.0f );
		//mat1->setDetailTiling( 10.0f );
		mat1->setSpecularIntensity( 0.34f );
		mat1->setGlossiness( 32.0f );
		//mat1->setSpecularPower( 32.0f );

		pMesh = ptr->loadMesh( "Media/Meshes/wood_box_a.obj" );
		pMesh->setMaterial( mat0 );
		pMesh->getSubmesh(0)->setMaterial( mat1 );
		m_pMeshes[L"wood_box_a"] = pMesh;

		pMesh = ptr->loadMesh( "Media/Meshes/wood_box_b.obj" );
		pMesh->setMaterial( mat1 );
		m_pMeshes[L"wood_box_b"] = pMesh;

		pMesh = ptr->loadMesh( "Media/Meshes/wood_box_c.obj" );
		pMesh->setMaterial( mat0 );
		pMesh->getSubmesh(1)->setMaterial( mat1 );
		m_pMeshes[L"wood_box_c"] = pMesh;

		pMesh = ptr->loadMesh( "Media/Meshes/wood_box_d.obj" );
		pMesh->setMaterial( mat0 );
		pMesh->getSubmesh(1)->setMaterial( mat1 );
		m_pMeshes[L"wood_box_d"] = pMesh;

		pMesh = ptr->loadMesh( "Media/Meshes/wood_box_e.obj" );
		pMesh->setMaterial( mat0 );
		pMesh->getSubmesh(1)->setMaterial( mat1 );
		m_pMeshes[L"wood_box_e"] = pMesh;
	}


	//--metro_strongboxes
	//props_met_skaff_a~props_met_skaff_b
	//D:/Metro Last Light/content/textures/props/props_met_skaff.512
	{
		DX11Material_Deferred *matb0 = new DX11Material_Deferred(ptr);
		matb0->setDiffusemap( ptr->loadTexture( L"Media/props_met_skaff.1024.bmp" ) );
		matb0->setNormalmap( ptr->loadTexture( L"Media/props_met_skaff_normal.1024.bmp" ) );
		matb0->setSpecularMap( ptr->loadTexture( L"Media/props_met_skaff_bump.1024.bmp" ) );
		//mat0->setDetailNormalmap( ptr->loadTexture( L"Media/Tex_0010_5.dds" ) );
		//mat0->setDetailNormalStrength( 2.0f );
		//mat0->setDetailTiling( 10.0f );
		matb0->setSpecularIntensity( 0.7f );
		//matb0->setSpecularPower( 2048.0f );

		pMesh = ptr->loadMesh( "Media/Meshes/props_met_skaff_a.obj" );
		pMesh->setMaterial( matb0 );
		m_pMeshes[L"props_met_skaff_a"] = pMesh;

		pMesh = ptr->loadMesh( "Media/Meshes/props_met_skaff_b.obj" );
		pMesh->setMaterial( matb0 );
		m_pMeshes[L"props_met_skaff_b"] = pMesh;

		pMesh = ptr->loadMesh( "Media/Meshes/props_met_skaff_c.obj" );
		pMesh->setMaterial( matb0 );
		m_pMeshes[L"props_met_skaff_c"] = pMesh;

		pMesh = ptr->loadMesh( "Media/Meshes/props_met_skaff_d.obj" );
		pMesh->setMaterial( matb0 );
		m_pMeshes[L"props_met_skaff_d"] = pMesh;
	}

	// kardus
	// kardus_a ~ kardus_j
	{
		//DX11Material_DiffuseDetailbump* mat = new DX11Material_DiffuseDetailbump(ptr);
		DX11Material_Deferred* mat = new DX11Material_Deferred(ptr);
		mat->setDiffusemap( ptr->loadTexture( L"Media/props_korobka.1024.bmp" ) );
		mat->setNormalmap( ptr->loadTexture( L"Media/props_korobka_normal.1024.bmp" ) );
		mat->setSpecularMap( ptr->loadTexture( L"Media/grey.bmp" ) );
		//mat->setDetailNormalmap( ptr->loadTexture( L"Media/Tex_0010_5.dds" ) );
		//mat->setDetailNormalStrength( 2.0f );
		//mat->setDetailTiling( 5.0f );
		mat->setSpecularIntensity( 0.24f );
		mat->setGlossiness( 1024.0f );
		//mat->setSpecularPower( 16.0f );

		Mesh* mesh = ptr->loadMesh( "Media/Meshes/kardus_a.obj" );
		mesh->setMaterial( mat );
		m_pMeshes[L"kardus_a"] = mesh;

		mesh = ptr->loadMesh( "Media/Meshes/kardus_b.obj" );
		mesh->setMaterial( mat );
		m_pMeshes[L"kardus_b"] = mesh;

		mesh = ptr->loadMesh( "Media/Meshes/kardus_c.obj" );
		mesh->setMaterial( mat );
		m_pMeshes[L"kardus_c"] = mesh;

		mesh = ptr->loadMesh( "Media/Meshes/kardus_d.obj" );
		mesh->setMaterial( mat );
		m_pMeshes[L"kardus_d"] = mesh;

		mesh = ptr->loadMesh( "Media/Meshes/kardus_e.obj" );
		mesh->setMaterial( mat );
		m_pMeshes[L"kardus_e"] = mesh;

		mesh = ptr->loadMesh( "Media/Meshes/kardus_f.obj" );
		mesh->setMaterial( mat );
		m_pMeshes[L"kardus_f"] = mesh;

		mesh = ptr->loadMesh( "Media/Meshes/kardus_g.obj" );
		mesh->setMaterial( mat );
		m_pMeshes[L"kardus_g"] = mesh;

		mesh = ptr->loadMesh( "Media/Meshes/kardus_h.obj" );
		mesh->setMaterial( mat );
		m_pMeshes[L"kardus_h"] = mesh;

		//DX11Material_DiffuseDetailbump* mat1 = new DX11Material_DiffuseDetailbump(ptr);
		DX11Material_Deferred* mat1 = new DX11Material_Deferred(ptr);
		mat1->setDiffusemap( ptr->loadTexture( L"Media/wood_box_2.1024.bmp" ) );
		mat1->setNormalmap( ptr->loadTexture( L"Media/wood_box_2_normal.1024.bmp" ) );
		//mat1->setDetailNormalmap( ptr->loadTexture( L"Media/Tex_0010_5.dds" ) );
		//mat1->setDetailNormalStrength( 3.0f );
		//mat1->setDetailTiling( 20.0f );
		mat1->setSpecularIntensity( 0.14f );
		//mat1->setSpecularPower( 16.0f );

		mesh = ptr->loadMesh( "Media/Meshes/kardus_i.obj" );
		mesh->setMaterial( mat1 );
		m_pMeshes[L"kardus_i"] = mesh;

		mesh = ptr->loadMesh( "Media/Meshes/kardus_j.obj" );
		mesh->setMaterial( mat1 );
		m_pMeshes[L"kardus_j"] = mesh;
	}

	// mesh: "metro_kasur"
	// prefab: "prop_metro_kasur"
	{
		//DX11Material_DiffuseDetailbump* mat0 = new DX11Material_DiffuseDetailbump(ptr);
		DX11Material_Deferred* mat0 = new DX11Material_Deferred(ptr);
		mat0->setDiffusemap( ptr->loadTexture( L"Media/metal34_props_01.1024.dds" ) );
		mat0->setNormalmap( ptr->loadTexture( L"Media/metal34_props_01_normal.2048.bmp" ) );
		//mat0->setDetailNormalmap( ptr->loadTexture( L"Media/Tex_0010_5.dds" ) );
		//mat0->setDetailNormalStrength( 5.0f );
		//mat0->setDetailTiling( 12.0f );
		mat0->setSpecularIntensity( 0.3f );
		mat0->setGlossiness( 32.0f );
		//mat0->setSpecularPower( 32.0f );

		//DX11Material_DiffuseDetailbump* mat1 = new DX11Material_DiffuseDetailbump(ptr);
		DX11Material_Deferred* mat1 = new DX11Material_Deferred(ptr);
		mat1->setDiffusemap( ptr->loadTexture( L"Media/props34_san_kol.2048.crn.bmp" ) );
		mat1->setNormalmap( ptr->loadTexture( L"Media/props34_san_kol_normal.2048.crn.bmp" ) );
		//mat1->setDetailNormalmap( ptr->loadTexture( L"Media/Tex_0010_5.dds" ) );
		//mat1->setDetailNormalStrength( 5.0f );
		//mat1->setDetailTiling( 12.0f );
		mat1->setSpecularIntensity( 0.3f );
		mat1->setGlossiness( 32.0f );
		//mat1->setSpecularPower( 32.0f );

		//DX11Material_DiffuseDetailbump* mat2 = new DX11Material_DiffuseDetailbump(ptr);
		DX11Material_Deferred* mat2 = new DX11Material_Deferred(ptr);
		mat2->setDiffusemap( ptr->loadTexture( L"Media/props_mattress.512.dds" ) );
		mat2->setNormalmap( ptr->loadTexture( L"Media/props_mattress_normal.512.bmp" ) );
		//mat2->setDetailNormalmap( ptr->loadTexture( L"Media/Tex_0010_5.dds" ) );
		//mat2->setDetailNormalStrength( 2.0f );
		//mat2->setDetailTiling( 8.0f );
		mat2->setSpecularIntensity( 0.15f );
		mat2->setGlossiness( 16.0f );
		//mat2->setSpecularPower( 16.0f );

		//DX11Material_DiffuseDetailbump* mat3 = new DX11Material_DiffuseDetailbump(ptr);
		DX11Material_Deferred* mat3 = new DX11Material_Deferred(ptr);
		mat3->setDiffusemap( ptr->loadTexture( L"Media/props_fabric.2048.dds" ) );
		mat3->setNormalmap( ptr->loadTexture( L"Media/props_fabric_normal.2048.bmp" ) );
		//mat3->setDetailNormalmap( ptr->loadTexture( L"Media/Tex_0010_5.dds" ) );
		//mat3->setDetailNormalStrength( 1.0f );
		//mat3->setDetailTiling( 12.0f );
		mat3->setSpecularIntensity( 0.15f );
		mat3->setGlossiness( 16.0f );
		//mat3->setSpecularPower( 16.0f );

		Mesh* mesh = ptr->loadMesh( "Media/metro_kasur.obj" );
		mesh->setMaterial( mat2 );
		mesh->getSubmesh(2)->setMaterial( mat3 );
		mesh->getSubmesh(3)->setMaterial( mat0 );
		m_pMeshes[L"metro_kasur"] = mesh;
	}

	// Prefab : prop_metro_kursiroda
	// Mesh : metro_kursiroda
	{
		//DX11Material_DiffuseDetailbump* pMaterial = new DX11Material_DiffuseDetailbump(ptr);
		DX11Material_Deferred* mat = new DX11Material_Deferred(ptr);
		mat->setDiffusemap( ptr->loadTexture( L"Media/props34_san_kol.2048.crn.bmp" ) );
		mat->setNormalmap( ptr->loadTexture( L"Media/props34_san_kol_normal.2048.crn.bmp" ) );
		//mat->setDetailNormalmap( ptr->loadTexture( L"Media/Tex_0010_5.dds" ) );
		//mat->setDetailNormalStrength( 3.0f );
		//mat->setDetailTiling( 12.0f );
		mat->setSpecularIntensity( 0.22f );
		mat->setGlossiness( 12.0f );
		//mat->setSpecularPower( 12.0f );

		Mesh* mesh = ptr->loadMesh( "Media/metro_kursiroda.3DS" );
		mesh->setMaterial( mat );

		m_pMeshes[L"metro_kursiroda"] = mesh;
	}

	// Prefab : prop_metro_tumba01
	// Mesh : mesh_metro_tumba01
	{
		DX11Material_Deferred* mat0 = new DX11Material_Deferred(ptr);
		mat0->setDiffusemap( ptr->loadTexture( L"Media/props_shkaf_01.1024.bmp" ) );
		mat0->setNormalmap( ptr->loadTexture( L"Media/props_shkaf_01_normal.1024.bmp" ) );
		//mat0->setDetailNormalmap( ptr->loadTexture( L"Media/Tex_0010_5.dds" ) );
		//mat0->setDetailNormalStrength( 2.0f );
		//mat0->setDetailTiling( 10.0f );
		//mat0->setSpecularIntensity( 0.34f );
		//mat0->setSpecularPower( 32.0f );

		DX11Material_Deferred* mat1 = new DX11Material_Deferred(ptr);
		mat1->setDiffusemap( ptr->loadTexture( L"Media/props_shkaf_01.1024.bmp" ) );
		mat1->setNormalmap( ptr->loadTexture( L"Media/props_shkaf_01_normal.1024.bmp" ) );
		//mat1->setDetailNormalmap( ptr->loadTexture( L"Media/Tex_0010_5.dds" ) );
		//mat1->setDetailNormalStrength( 2.0f );
		//mat1->setDetailTiling( 10.0f );
		//mat1->setSpecularIntensity( 0.34f );
		//mat1->setSpecularPower( 32.0f );

		Mesh* mesh = ptr->loadMesh( "Media/metro_tumba01.obj" );
		mesh->setMaterial( mat0 );
		mesh->getSubmesh(1)->setMaterial( mat1 );

		m_pMeshes[L"mesh_metro_tumba01"] = mesh;
	}
	
	// Prefab : prop_ddo_helmet
	// Mesh : mesh_ddo_helmet
	{
		DX11Material_DeferredIBL* mat = new DX11Material_DeferredIBL(ptr);
		mat->setDiffusemap( ptr->loadTexture( L"Media/DDO_SDK_Helmet/albedo.jpg" ) );
		//mat->setDiffusemap( ptr->loadTexture( L"Media/grey.bmp" ) );
		mat->setNormalmap( ptr->loadTexture( L"Media/DDO_SDK_Helmet/normal.jpg" ) );
		mat->setSpecularMap( ptr->loadTexture( L"Media/DDO_SDK_Helmet/specular.jpg" ) );
		mat->setAOMap( ptr->loadTexture( L"Media/DDO_SDK_Helmet/ao.jpg" ) );
		//mat->setSpecularMap( ptr->loadTexture( L"Media/test_specular1.bmp" ) );
		//mat->setDetailNormalmap( ptr->loadTexture( L"Media/Tex_0010_5.dds" ) );
		//mat->setDetailNormalStrength( 3.0f );
		//mat->setDetailTiling( 12.0f );
		mat->setSpecularIntensity( 1.0f );
		mat->setGlossiness( 2048.0f );
		//mat->setSpecularPower( 12.0f );

		Image* faces[6];
		int size = 256;
		faces[0] = new Image( size, size, Color(255,0,0,255) );
		for(int i=0; i<size; i++)
		{
			for(int j=0; j<size; j++)
			{
				//faces[0]->operator()(i,j) = Color(cosf(sqrtf(i*i+j*j))*127+127, 0, 0, 255);
				faces[0]->operator()(i,j) = 0;//getSkyColor( Vector3(1.0f,j/(float)size,(size-i)/(float)size).normalisedCopy() );
			}
		}
		faces[1] = new Image( size, size, Color(255,255,0,255) );
		for(int i=0; i<size; i++)
		{
			for(int j=0; j<size; j++)
			{
				//faces[1]->operator()(i,j) = Color(cosf(sqrtf(i*i+j*j))*127+127, 0, cosf(sqrtf(i*i+j*j))*127+127, 255);
				//faces[1]->operator()(i,j) = 0;//getSkyColor( Vector3(-1.0f,j/(float)size,i/(float)size).normalisedCopy() );
			}
		}
		faces[2] = new Image( size, size, Color(0,255,0,255) );
		for(int i=0; i<size; i++)
		{
			for(int j=0; j<size; j++)
			{
				//faces[2]->operator()(i,j) = Color(0, cosf(sqrtf(i*i+j*j))*127+127, 0, 255);
				faces[2]->operator()(i,j) = 127+127*(cosf(sqrtf(((size/2-i)/4.0f)*((size/2-i)/4.0f)+((size/2-j)/4.0f)*((size/2-j)/4.0f))));//getSkyColor( Vector3(i/(float)size,1.0f,j/(float)size).normalisedCopy() );
			}
		}
		faces[3] = new Image( size, size, Color(255,255,0,255) );
		for(int i=0; i<size; i++)
		{
			for(int j=0; j<size; j++)
			{
				//faces[3]->operator()(i,j) = Color(cosf(sqrtf(i*i+j*j))*127+127, cosf(sqrtf(i*i+j*j))*127+127, 0, 255);
				//faces[3]->operator()(i,j) = 0;//getSkyColor( Vector3(i/(float)size,-1.0f,(size-j)/(float)size).normalisedCopy() );
			}
		}
		faces[4] = new Image( size, size, Color(0,0,255,255) );
		for(int i=0; i<size; i++)
		{
			for(int j=0; j<size; j++)
			{
				//faces[4]->operator()(i,j) = Color(0, 0, cosf(sqrtf(i*i+j*j))*127+127, 255);
				//faces[4]->operator()(i,j) = 0;//getSkyColor( Vector3(i/(float)size,j/(float)size,1.0f).normalisedCopy() );
			}
		}
		faces[5] = new Image( size, size, Color(255,0,255,255) );
		for(int i=0; i<size; i++)
		{
			for(int j=0; j<size; j++)
			{
				//faces[5]->operator()(i,j) = Color(cosf(sqrtf(i*i+j*j))*127+127, 0, cosf(sqrtf(i*i+j*j))*127+127, 255);
				//faces[5]->operator()(i,j) = 0;//getSkyColor( Vector3(i/(float)size,j/(float)size,-1.0f).normalisedCopy() );
			}
		}
		//mat->setIBL( ptr->createCubemap( faces ) );
		mat->setIBL( ptr->loadTexture( L"Media/cubemap0.dds" ) );

		Mesh* mesh = ptr->loadMesh( "Media/DDO_SDK_Helmet/HelmetMeshTriCentered.obj" );
		mesh->setMaterial( mat );

		m_pMeshes[L"mesh_ddo_helmet"] = mesh;
	}

	// Prefab : prop_ddo_aks
	// Mesh : mesh_ddo_ks
	{
		DX11Material_DeferredIBL* mat = new DX11Material_DeferredIBL(ptr);
		mat->setDiffusemap( ptr->loadTexture( L"Media/DDO_SDK_AKS/_polySurface42_d.jpg" ) );
		//mat->setDiffusemap( ptr->loadTexture( L"Media/grey.bmp" ) );
		mat->setNormalmap( ptr->loadTexture( L"Media/DDO_SDK_AKS/_polySurface42_n.jpg" ) );
		mat->setSpecularMap( ptr->loadTexture( L"Media/DDO_SDK_AKS/_polySurface42_g.jpg" ) );
		mat->setAOMap( ptr->loadTexture( L"Media/DDO_SDK_AKS/_polySurface42_o.jpg" ) );
		//mat->setSpecularMap( ptr->loadTexture( L"Media/test_specular1.bmp" ) );
		//mat->setDetailNormalmap( ptr->loadTexture( L"Media/Tex_0010_5.dds" ) );
		//mat->setDetailNormalStrength( 3.0f );
		//mat->setDetailTiling( 12.0f );
		mat->setSpecularIntensity( 1.0f );
		mat->setGlossiness( 2048.0f );

		mat->setIBL( ptr->loadTexture( L"Media/cubemap0.dds" ) );

		Mesh* mesh = ptr->loadMesh( "Media/DDO_SDK_AKS/Low.obj" );
		mesh->setMaterial( mat );

		m_pMeshes[L"mesh_ddo_aks"] = mesh;
	}

	// Sphere for IBL testing
	// Prefab : prop_sphere
	// Mesh : mesh_sphere
	{
		DX11Material_DeferredIBL* mat = new DX11Material_DeferredIBL(ptr);
		//mat->setDiffusemap( ptr->loadTexture( L"Media/DDO_SDK_Helmet/albedo.jpg" ) );
		mat->setDiffusemap( ptr->loadTexture( L"Media/redplastic.bmp" ) );
		mat->setNormalmap( ptr->loadTexture( L"Media/flat.bmp" ) );
		mat->setSpecularMap( ptr->loadTexture( L"Media/test_specular2.bmp" ) );
		mat->setAOMap( ptr->loadTexture( L"Media/white.bmp" ) );
		mat->setSpecularIntensity( 1.0f );
		mat->setGlossiness( 2048.0f );
		mat->setIBL( ptr->loadTexture( L"Media/cubemap0.dds" ) );

		Mesh* mesh = ptr->loadMesh( "Media/Meshes/sphere64.obj" );
		mesh->setMaterial( mat );

		m_pMeshes[L"mesh_sphere"] = mesh;
	}
}

void Scene::loadPrefabs()
{
	//////////////////
	// Kardus
	//////////////////
	hkpRigidBody* rigidBody = createBoxRigidBody( Vector3(0.3307f, 0.3313f, 0.3064f), 1.0f );
	Mesh* mesh = m_pMeshes[L"kardus_a"];
	Entity_Prop* prefab = new Entity_Prop( m_pRenderSystem, m_pPhysicsSystem, mesh, rigidBody );
	m_pPrefabs[L"prop_kardus_a"] = prefab;

	rigidBody = createBoxRigidBody( Vector3(0.3110f, 0.3796f, 0.2614f), 1.0f );
	mesh = m_pMeshes[L"kardus_b"];
	prefab = new Entity_Prop( m_pRenderSystem, m_pPhysicsSystem, mesh, rigidBody );
	m_pPrefabs[L"prop_kardus_b"] = prefab;

	rigidBody = createBoxRigidBody( Vector3(0.4516f, 0.3906f, 0.3237f), 1.0f );
	mesh = m_pMeshes[L"kardus_c"];
	prefab = new Entity_Prop( m_pRenderSystem, m_pPhysicsSystem, mesh, rigidBody );
	m_pPrefabs[L"prop_kardus_c"] = prefab;

	rigidBody = createBoxRigidBody( Vector3(0.2784f, 0.3841f, 0.2129f), 1.0f );
	mesh = m_pMeshes[L"kardus_d"];
	prefab = new Entity_Prop( m_pRenderSystem, m_pPhysicsSystem, mesh, rigidBody );
	m_pPrefabs[L"prop_kardus_d"] = prefab;

	rigidBody = createBoxRigidBody( Vector3(0.3469f, 0.3995f, 0.2017f), 1.0f );
	mesh = m_pMeshes[L"kardus_e"];
	prefab = new Entity_Prop( m_pRenderSystem, m_pPhysicsSystem, mesh, rigidBody );
	m_pPrefabs[L"prop_kardus_e"] = prefab;

	rigidBody = createBoxRigidBody( Vector3(0.4792f, 0.4000f, 0.2221f), 1.0f );
	mesh = m_pMeshes[L"kardus_f"];
	prefab = new Entity_Prop( m_pRenderSystem, m_pPhysicsSystem, mesh, rigidBody );
	m_pPrefabs[L"prop_kardus_f"] = prefab;

	rigidBody = createBoxRigidBody( Vector3(0.2772f, 0.4186f, 0.1188f), 1.0f );
	mesh = m_pMeshes[L"kardus_g"];
	prefab = new Entity_Prop( m_pRenderSystem, m_pPhysicsSystem, mesh, rigidBody );
	m_pPrefabs[L"prop_kardus_g"] = prefab;

	rigidBody = createBoxRigidBody( Vector3(0.2765f, 0.4182f, 0.1187f), 1.0f );
	mesh = m_pMeshes[L"kardus_h"];
	prefab = new Entity_Prop( m_pRenderSystem, m_pPhysicsSystem, mesh, rigidBody );
	m_pPrefabs[L"prop_kardus_h"] = prefab;

	rigidBody = createBoxRigidBody( Vector3(0.3509f, 0.3646f, 0.2085f), 1.0f );
	mesh = m_pMeshes[L"kardus_i"];
	prefab = new Entity_Prop( m_pRenderSystem, m_pPhysicsSystem, mesh, rigidBody );
	m_pPrefabs[L"prop_kardus_i"] = prefab;

	rigidBody = createBoxRigidBody( Vector3(0.3172f, 0.3165f, 0.1623f), 1.0f );
	mesh = m_pMeshes[L"kardus_j"];
	prefab = new Entity_Prop( m_pRenderSystem, m_pPhysicsSystem, mesh, rigidBody );
	m_pPrefabs[L"prop_kardus_j"] = prefab;

	//////////////////
	// Kasur
	//////////////////
	{
	if (m_pPhysicsSystem)
	{
		hkRootLevelContainer* container = m_HavokLoader.load("Media/Havok/metro_bed01_physics_XML.hkt");
		if (container != HK_NULL)
		{
			hkpPhysicsData* data = static_cast<hkpPhysicsData*>(container->findObjectByType(hkpPhysicsDataClass.getName()));
			rigidBody = data->findRigidBodyByName("Box003");
		}
	}
	else
		rigidBody = NULL;

	mesh = m_pMeshes[L"metro_kasur"];
	Transform transform;
	transform.setPosition(0.0f,0.0f,0.336f);
	prefab = new Entity_Prop( m_pRenderSystem, m_pPhysicsSystem, mesh, rigidBody, transform );
	m_pPrefabs[L"prop_metro_kasur"] = prefab;
	}
	//////////////////
	// Kursi roda
	//////////////////
	{
	if (m_pPhysicsSystem)
	{
		hkRootLevelContainer* container = m_HavokLoader.load("Media/Havok/metro_kursiroda_physics_XML.hkt");
		if (container != HK_NULL)
		{
			hkpPhysicsData* data = static_cast<hkpPhysicsData*>(container->findObjectByType(hkpPhysicsDataClass.getName()));
			rigidBody = data->findRigidBodyByName("parent_hull");
		}
	}
	else
		rigidBody = NULL;

	mesh = m_pMeshes[L"metro_kursiroda"];
	Transform transform;
	transform.setPosition(0.0f,0.0f,0.0f);
	prefab = new Entity_Prop( m_pRenderSystem, m_pPhysicsSystem, mesh, rigidBody, transform );
	m_pPrefabs[L"prop_metro_kursiroda"] = prefab;
	}
}

hkpRigidBody* Scene::createBoxRigidBody( Vector3 halfExtents, float mass )
{
	if (m_pPhysicsSystem)
	{
		return m_pPhysicsSystem->createBoxRigidBody( halfExtents, mass );
	}
	return NULL;
}

Color getSkyColor(Vector3 e)
{
	e.z = max(e.z,0.0);
	Color ret;
	ret.r = pow(1.0-e.z,2.0)*255;
	ret.g = (1.0-e.z)*255;
	ret.b = (0.6+(1.0-e.z)*0.4)*255;
	return ret;
}