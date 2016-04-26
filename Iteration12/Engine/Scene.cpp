#pragma once

#include "Scene.h"
#include "LightVolume.h"
#include "Entity_Water.h"
#include "Camera3D.h"
#include "TextureCube.h"

void Scene::Init( RenderSystem* pRendering, PhysicsSystem* pPhysics )
{
	SetRenderSystem( pRendering );
	SetPhysicsSystem( pPhysics );

	// G-buffers
	RenderSystemConfig config = m_pRenderSystem->GetConfig();
	m_pGBuffer[0] = m_pRenderSystem->CreateTexture( config.Height, config.Width, R8G8B8A8_UNORM );
	m_pGBuffer[1] = m_pRenderSystem->CreateTexture( config.Height, config.Width, R32G32B32A32_FLOAT );
	m_pGBuffer[2] = m_pRenderSystem->CreateTexture( config.Height, config.Width, R8G8B8A8_UNORM );

	// HDR render target
	m_pHDRRenderTarget = m_pRenderSystem->CreateTexture( config.Height, config.Width, R16G16B16A16_FLOAT );

	// Fullscreen passes
	m_pFullscreenQuad = new FullscreenQuad( pRendering );
	m_pFullscreenQuad->SetShaderset( m_pRenderSystem->LoadShaderset( L"Shaders/Deferred.hlsl", "VS", "PS", SM_5_0 ) );
	m_pFullscreenQuad->SetTexture( "gbuffer0", m_pGBuffer[0] );
	m_pFullscreenQuad->SetTexture( "gbuffer1", m_pGBuffer[1] );
	m_pFullscreenQuad->SetTexture( "gbuffer2", m_pGBuffer[2] );

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
	Texture2D *noiseTexture = m_pRenderSystem->CreateTexture( &noise, R32G32_FLOAT );

	// Half-res gbuffer[1] (normals and depth)
	m_pHalfRes = m_pRenderSystem->CreateTexture( config.Height/4, config.Width/4, R32G32B32A32_FLOAT );

	// Half-res SSAO buffer
	m_pSSAOBuffer = m_pRenderSystem->CreateTexture( config.Height/4, config.Width/4, R16_UNORM );

	m_pSSAOPass = new FullscreenQuad( pRendering );
	m_pSSAOPass->SetShaderset( m_pRenderSystem->LoadShaderset( L"Shaders/SSAO.hlsl", "VS", "PS", SM_5_0 ) );
	m_pSSAOPass->SetTexture( "gbuffer0", m_pGBuffer[0] );
	m_pSSAOPass->SetTexture( "gbuffer1", m_pHalfRes );
	m_pSSAOPass->SetTexture( "texNoise", noiseTexture );

	m_pSSAOCombinePass = new FullscreenQuad( pRendering );
	m_pSSAOCombinePass->SetShaderset( m_pRenderSystem->LoadShaderset( L"Shaders/SSAOCombine.hlsl", "VS", "PS", SM_5_0 ) );
	m_pSSAOCombinePass->SetTexture( "gbuffer0", m_pGBuffer[0] );
	m_pSSAOCombinePass->SetTexture( "gbuffer1", m_pGBuffer[1] );
	m_pSSAOCombinePass->SetTexture( "texSSAO", m_pSSAOBuffer );

	// Compile tonemapping shader with diferrent settings
	m_pTonemappingShaderset[0] = m_pRenderSystem->LoadShaderset( L"Shaders/Tonemapping.hlsl", "VS", "PS", SM_5_0  );

	std::vector<ShaderMacro> macros;
	macros.push_back( ShaderMacro("TONEMAPPING_OPERATOR", "1") );
	m_pTonemappingShaderset[1] = m_pRenderSystem->LoadShaderset( L"Shaders/Tonemapping.hlsl", "VS", "PS", SM_5_0, &macros, false );

	macros[0].Name = "TONEMAPPING_OPERATOR";
	macros[0].Definition = "2";
	m_pTonemappingShaderset[2] = m_pRenderSystem->LoadShaderset( L"Shaders/Tonemapping.hlsl", "VS", "PS", SM_5_0, &macros, false );

	macros[0].Name = "TONEMAPPING_OPERATOR";
	macros[0].Definition = "3";
	m_pTonemappingShaderset[3] = m_pRenderSystem->LoadShaderset( L"Shaders/Tonemapping.hlsl", "VS", "PS", SM_5_0, &macros, false );

	m_pTonemappingPass = new FullscreenQuad( m_pRenderSystem );
	m_pTonemappingPass->SetShaderset( m_pTonemappingShaderset[0] );
	m_pTonemappingPass->SetTexture( "texSource", m_pHDRRenderTarget );
	ShaderParamBlock shaderParams;
	shaderParams.assign( "fExposure", 0, 1.0f );
	m_pTonemappingPass->SetShaderParams(shaderParams);

	m_pSunlight = new FullscreenQuad( pRendering );
	m_pSunlight->SetShaderset( m_pRenderSystem->LoadShaderset( L"Shaders/DeferredSunlight.hlsl", "VS", "PS", SM_5_0 ) );
	m_pSunlight->SetTexture( "txGBuffer0", m_pGBuffer[0] );
	m_pSunlight->SetTexture( "txGBuffer1", m_pGBuffer[1] );
	m_pSunlight->SetTexture( "txGBuffer2", m_pGBuffer[2] );

	// Prototype global illumination
	//m_pGI = new FullscreenQuad( pRendering );
	//m_pGI->SetShaderset( m_pRenderSystem->LoadShaderset( L"Shaders/GI.hlsl", "VS", "PS", SM_5_0 ) );
	//m_pGI->SetTexture( "txGBuffer0", m_pGBuffer[0] );
	//m_pGI->SetTexture( "txGBuffer1", m_pGBuffer[1] );
	//m_pGI->SetTexture( "txGBuffer2", m_pGBuffer[2] );

	// Shadowmaps
	m_pShadowmap[0] = m_pRenderSystem->CreateTexture( 256*8, 256*8, R32_FLOAT );/*
	m_pShadowmap[1] = m_pRenderSystem->CreateTexture( 256*4, 256*4, R32_FLOAT ); 
	m_pShadowmap[2] = m_pRenderSystem->CreateTexture( 256*4, 256*4, R32_FLOAT );
	m_pShadowmap[3] = m_pRenderSystem->CreateTexture( 256*4, 256*4, R32_FLOAT );
	m_pShadowmap[4] = m_pRenderSystem->CreateTexture( 256*2, 256*2, R32_FLOAT );
	m_pShadowmap[5] = m_pRenderSystem->CreateTexture( 256*2, 256*2, R32_FLOAT );
	m_pShadowmap[6] = m_pRenderSystem->CreateTexture( 256*2, 256*2, R32_FLOAT );
	m_pShadowmap[7] = m_pRenderSystem->CreateTexture( 256*2, 256*2, R32_FLOAT );*/

	m_pRSMNormal[0] = m_pRenderSystem->CreateTexture( 256*2, 256*2, R16G16B16A16_FLOAT );
	m_pRSMColor[0] = m_pRenderSystem->CreateTexture( 256*2, 256*2, R8G8B8A8_UNORM );

	m_pSMEmpty = m_pRenderSystem->CreateTexture( 1, 1, R32_FLOAT ); 
	
	float ClearColor[4] = { D3D11_FLOAT32_MAX, 1.0f, 1.0f, 1.0f };
	m_pRenderSystem->ClearTexture( m_pShadowmap[0], ClearColor );
	m_pRenderSystem->ClearTexture( m_pSMEmpty, ClearColor );

	//m_pTestTexture = m_pRenderSystem->LoadTexture( L"Media/781484020226.jpg" );
	//m_pGBuffer[0] = m_pTestTexture;

	// Water surface
	//m_pWater = new Entity_Water( m_pRenderSystem );
	//m_pWater->setName( L"Water" );
	//m_pWater->transform()->Translate(0.0f,0.0f,0.0f);
	//AddEntity(m_pWater);

	// Skybox
	DX11Material_Skybox* material = new DX11Material_Skybox( m_pRenderSystem );
	material->SetCubemap( m_pRenderSystem->LoadTexture( L"Media/cubemap0.dds" ) );
	Mesh* mesh = m_pRenderSystem->LoadMesh( "Media/Meshes/unit_cube_inverted.obj" );
	mesh->SetMaterial( material );
	Entity_StaticProp* skybox = new Entity_StaticProp( m_pRenderSystem, mesh );
	skybox->SetName( L"Skybox" );
	skybox->Transformation()->SetScale( 2000.0f, 2000.0f, 2000.0f );
	skybox->Transformation()->SetPosition( 0.0f, 0.0f, 0.0f );
	AddEntity( skybox );

	LoadMeshes();
	LoadPrefabs();
}

void Scene::SetPhysicsSystem( PhysicsSystem* ptr )
{ 
	m_pPhysicsSystem = ptr;
}

void Scene::SetRenderSystem( RenderSystem* pRenderSystem )
{
	m_pRenderSystem = pRenderSystem;
}

void Scene::SetCamera( Camera3D* camera )
{
	m_pActiveCamera = camera;

	if (m_pWater)
		m_pWater->SetGridProjectorCamera( camera );
};

void Scene::AddEntity( Entity* ent )
{
	ent->OnAddToScene(this); 
	m_pEntities.push_back(ent);
}

// TODO : parallelize?
void Scene::Update( float deltaTime )
{
	// Update everything
	SetDeltaTime( deltaTime );
	m_Time += deltaTime;

	// Step physics
	Timer timer;
	if (m_pPhysicsSystem)
		m_pPhysicsSystem->Step( deltaTime );
	t_physics = timer.GetMiliseconds();

	// Container may dinamically change sizes (an objects's update() spawning/destroying entities, etc)
	// If we used iterators, they could get invalidated
	// TODO : handle case where an element in the container gets destroyed
	for( std::vector<Entity*>::size_type i = 0; i < m_pEntities.size(); ++i )
	{
		m_pEntities[i]->Update(m_DeltaTime);
	}
	for ( std::vector<PointLight*>::iterator it2 = m_pPointLights.begin(); it2 != m_pPointLights.end(); ++it2 )
	{
		(*it2)->Update();
		(*it2)->Cull( &m_pActiveCamera->GetFrustum() );
	}
	for ( std::vector<SpotLight*>::iterator it2 = m_pSpotLights.begin(); it2 != m_pSpotLights.end(); ++it2 )
	{
		(*it2)->Update();
		(*it2)->Cull( &m_pActiveCamera->GetFrustum() );
	}

	Timer oceanTimer;
	//m_pWater->update( m_DeltaTime );
	t_ocean = oceanTimer.GetMiliseconds();
}

void Scene::Render()
{
	Timer sceneTimer, t;
	t_Cull = 0.0f;

	//
	// Render shadowmaps
	//
	if (!m_pSpotLights.empty())
	{
		float clear[] = { 10000.0f, 1.0f, 1.0f, 1.0f };
		int i = 0;

		for ( std::vector<SpotLight*>::iterator it2 = m_pSpotLights.begin(); it2 != m_pSpotLights.end(); ++it2 )
		{
			if ((*it2)->IsVisible()&&(*it2)->IsEnabled())
			{
				if ((*it2)->IsCastingShadow() && i < 8)
				{
					m_pRenderSystem->ClearTexture( m_pShadowmap[i], clear );
					m_pRenderSystem->SetRenderTarget( m_pShadowmap[i] );

					Camera3D shadowCamera;
					shadowCamera.SetViewProjectionMatrix( (*it2)->GetProjectionCamera()->GetViewProjectionMatrix() );
					shadowCamera.SetFrustum( &(*it2)->GetFrustum() );

					for( std::vector<Entity*>::iterator it = m_pEntities.begin(); it != m_pEntities.end(); ++it )
					{
						sceneTimer.Start();
						(*it)->Cull( &(*it2)->GetFrustum() );
						t_Cull += sceneTimer.GetMiliseconds();
						(*it)->RenderShadowmap(&shadowCamera);
					}

					(*it2)->SetShadowmap( m_pShadowmap[i] );
				}
				else
					(*it2)->SetShadowmap( m_pSMEmpty );

			}
			i++;
		}
	}

	//
	// Render main scene
	//
	m_pRenderSystem->SetCamera( m_pActiveCamera );
	//m_pRenderSystem->SetMultipleRenderTargets( 3, m_pGBuffer );
	m_pRenderSystem->SetRenderTarget( m_pHDRRenderTarget );
	//m_pRenderSystem->SetBackbufferAsRenderTarget();

	for( std::vector<Entity*>::iterator it = m_pEntities.begin(); it != m_pEntities.end(); ++it )
	{
		sceneTimer.Start();
		XNA::Frustum frustum = m_pActiveCamera->GetFrustum();
		(*it)->ClearLights();
		(*it)->Cull( &frustum );
		for ( std::vector<PointLight*>::iterator it2 = m_pPointLights.begin(); it2 != m_pPointLights.end(); ++it2 )
		{
			if ((*it2)->IsEnabled()&&(*it2)->IsVisible())
				(*it)->CullLight( *it2 );
		}
		for ( std::vector<SpotLight*>::iterator it2 = m_pSpotLights.begin(); it2 != m_pSpotLights.end(); ++it2 )
		{
			if ((*it2)->IsEnabled()&&(*it2)->IsVisible())
				(*it)->CullLight( *it2 );
		}
		t_Cull += sceneTimer.GetMiliseconds();
		(*it)->Render();
		//(*it)->renderBoundingBox();
	}

	// Fullscreen pass to backbuffer
	m_pRenderSystem->SetBackbufferAsRenderTarget();
	//m_pFullscreenQuad->render();
	m_pTonemappingPass->Render(false);

	t_scenerender = t.GetMiliseconds();
}

void Scene::ParallelRender()
{
	Timer sceneTimer, t;
	t_Cull = 0.0f;

	//
	// Render shadowmaps
	//
	if (!m_pSpotLights.empty())
	{
		float clear[] = { 1.0f, 1.0f, 1.0f, 1.0f };
		int i = 0;

		for ( std::vector<SpotLight*>::iterator it2 = m_pSpotLights.begin(); it2 != m_pSpotLights.end(); ++it2 )
		{
			if ((*it2)->IsVisible()&&(*it2)->IsEnabled())
			{
				if ((*it2)->IsCastingShadow() && 8)
				{
					m_pRenderSystem->ClearTexture( m_pShadowmap[i], clear );
					m_pRenderSystem->SetRenderTarget( m_pShadowmap[i] );

					Camera3D shadowCamera;
					shadowCamera.SetViewProjectionMatrix( (*it2)->GetProjectionCamera()->GetViewProjectionMatrix() );
					shadowCamera.SetFrustum( &(*it2)->GetFrustum() );

					// lambda
					tbb::parallel_for_each( m_pEntities.begin(), m_pEntities.end(), [&](Entity* ptr)
					{
						sceneTimer.Start();
						ptr->Cull( &(*it2)->GetFrustum() );
						t_Cull += sceneTimer.GetMiliseconds();
						ptr->RenderShadowmap((*it2)->GetProjectionCamera());
					});

					(*it2)->SetShadowmap( m_pShadowmap[i] );
				}
				else
					(*it2)->SetShadowmap( m_pSMEmpty );

			}
			i++;
		}
	}

	//
	// Render main scene
	//
	float clearColor[4] = { 0.0f, 0.125f, 0.3f, 1.0f }; //red,green,blue,alpha
	m_pRenderSystem->ClearTexture( m_pHDRRenderTarget, clearColor );
	m_pRenderSystem->ClearTexture( m_pGBuffer[0], clearColor );
	m_pRenderSystem->ClearTexture( m_pGBuffer[1], clearColor );
	m_pRenderSystem->ClearTexture( m_pGBuffer[2], clearColor );

	m_pRenderSystem->SetCamera( m_pActiveCamera );
	Texture2D* MRT[] = { m_pHDRRenderTarget, m_pGBuffer[0], m_pGBuffer[1], m_pGBuffer[2], };
	m_pRenderSystem->SetMultipleRenderTargets( 4, MRT );
	//m_pRenderSystem->SetRenderTarget( m_pHDRRenderTarget );
	//m_pRenderSystem->SetBackbufferAsRenderTarget();


	//for( std::vector<Entity*>::iterator it = m_pEntities.begin(); it != m_pEntities.end(); ++it )
	//{
		//Entity* ptr = *it;
	// lambda
	tbb::parallel_for_each( m_pEntities.begin(), m_pEntities.end(), [&](Entity* ptr)
	{
		XNA::Frustum frustum = m_pActiveCamera->GetFrustum();/*
		frustum.LeftSlope /= 2;
		frustum.BottomSlope /= 2;
		frustum.RightSlope /= 2;
		frustum.TopSlope /= 2;*/
		ptr->ClearLights();
		ptr->Cull( &frustum );
		for ( std::vector<PointLight*>::iterator it2 = m_pPointLights.begin(); it2 != m_pPointLights.end(); ++it2 )
		{
			if ((*it2)->IsEnabled()&&(*it2)->IsVisible())
				ptr->CullLight( *it2 );
		}
		for ( std::vector<SpotLight*>::iterator it2 = m_pSpotLights.begin(); it2 != m_pSpotLights.end(); ++it2 )
		{
			if ((*it2)->IsEnabled()&&(*it2)->IsVisible())
				ptr->CullLight( *it2 );
		}
		//t_Cull += sceneTimer.GetMiliseconds();
		ptr->Render();
		//ptr->renderBoundingBox();
	});
	
	m_pRenderSystem->SetRenderTarget( m_pHDRRenderTarget );
	//m_pGBuffer[0] = m_pTestTexture;
	// Render lights
	for ( std::vector<SpotLight*>::iterator it2 = m_pSpotLights.begin(); it2 != m_pSpotLights.end(); ++it2 )
	{
		if ((*it2)->IsEnabled()&&(*it2)->IsVisible())
		{
			LightVolume volume( m_pRenderSystem, *it2 );
			//volume.setShadowmap( (*it2)->getShadowmap() );
			//volume.setCookie( (*it2)->getCookie() );
			volume.SetGBuffer( m_pGBuffer );
			volume.Render();
		}
	}

	// Fullscreen pass to backbuffer
	m_pRenderSystem->SetBackbufferAsRenderTarget();
	//m_pFullscreenQuad->render();
	m_pTonemappingPass->Render(false);

	t_scenerender = t.GetMiliseconds();
}

void Scene::RenderDeferred()
{
	Timer sceneTimer, t;
	t_Cull = 0.0f;

	//
	// Render main scene into gbuffer
	//
	float clearColor[4] = { 0.0f, 0.125f, 0.3f, 1.0f }; //red,green,blue,alpha
	m_pRenderSystem->ClearTexture( m_pHDRRenderTarget, clearColor );
	m_pRenderSystem->ClearTexture( m_pGBuffer[0], clearColor );
	m_pRenderSystem->ClearTexture( m_pGBuffer[1], clearColor );
	m_pRenderSystem->ClearTexture( m_pGBuffer[2], clearColor );

	m_pRenderSystem->SetCamera( m_pActiveCamera );
	Texture2D* MRT[] = { m_pHDRRenderTarget, m_pGBuffer[0], m_pGBuffer[1], m_pGBuffer[2], };
	m_pRenderSystem->SetMultipleRenderTargets( 4, MRT );
	//m_pRenderSystem->SetMultipleRenderTargets( 3, m_pGBuffer );
	//m_pRenderSystem->SetRenderTarget( m_pHDRRenderTarget );
	//m_pRenderSystem->SetBackbufferAsRenderTarget();


	// Render all entities
	std::for_each( m_pEntities.begin(), m_pEntities.end(), [&](Entity* ptr)
	{
		XNA::Frustum frustum = m_pActiveCamera->GetFrustum();

		//t_Cull += sceneTimer.GetMiliseconds();
		ptr->Cull( &frustum );
		//ptr->SetCulled( false );
		ptr->Render();
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
			if ((*it2)->IsVisible()&&(*it2)->IsEnabled())
			{
				// Render shadowmap/RSM
				if ((*it2)->IsCastingShadow())
				{
					if ((*it2)->IsRSMEnabled())
					{
						// Render Reflective Shadow Maps
						m_pRenderSystem->ClearTexture( m_pShadowmap[0], clear0 );
						m_pRenderSystem->ClearTexture( m_pRSMNormal[0], clear1 );
						m_pRenderSystem->ClearTexture( m_pRSMColor[0], clear1 );
						Texture2D* MRT[] = { m_pShadowmap[0], m_pRSMNormal[0], m_pRSMColor[0], };
						m_pRenderSystem->SetMultipleRenderTargets( 3, MRT );
					}
					else
					{
						// Render just the shadowmap
						m_pRenderSystem->ClearTexture( m_pShadowmap[0], clear0 );
						m_pRenderSystem->SetRenderTarget( m_pShadowmap[0] );
					}

					// Cull and render each scene entity into RSM/shadowmap
					std::for_each( m_pEntities.begin(), m_pEntities.end(), [&](Entity* ptr)
					{
						sceneTimer.Start();
						ptr->Cull( &(*it2)->GetFrustum() );
						t_Cull += sceneTimer.GetMiliseconds();
						if ((*it2)->IsRSMEnabled())
							ptr->RenderRSM((*it2)->GetProjectionCamera(), (*it2) );
						else
							ptr->RenderShadowmap((*it2)->GetProjectionCamera());
					});

					if ((*it2)->IsRSMEnabled())
					{
						(*it2)->SetShadowmap( m_pShadowmap[0] );
						(*it2)->SetRSMNormal( m_pRSMNormal[0] );
						(*it2)->SetRSMColor( m_pRSMColor[0] );
						m_pGI->SetTexture( "txShadowmap", m_pShadowmap[0] );
						m_pGI->SetTexture( "txRSMNormal", m_pRSMNormal[0] );
						m_pGI->SetTexture( "txRSMColor", m_pRSMColor[0] );
					}
					else
					{
						(*it2)->SetShadowmap( m_pShadowmap[0] );
					}
				}
				else
				{
					(*it2)->SetShadowmap( m_pSMEmpty );
				}

				// Render light volume
				m_pRenderSystem->SetRenderTarget( m_pHDRRenderTarget );
				LightVolume volume( m_pRenderSystem, *it2 );
				volume.SetGBuffer( m_pGBuffer );
				volume.Render();
				
				// Render GI!
				//if ((*it2)->isRSMEnabled())
					//m_pGI->render(true);
			}
		}
	}
	
	m_pRenderSystem->SetRenderTarget( m_pHDRRenderTarget );
	
	/*ShaderParamBlock shaderParams;
	shaderParams.assign( "vLightVector", 0, &Vector4( Vector3(-0.8f, 0.0f, 1.0f).normalisedCopy() ) );
	shaderParams.assign( "vColor", 0, &Vector4( 1.0f, 1.0f, 0.75f, 1.0f ) );
	shaderParams.assign( "vEyePos", 0, &XMFLOAT4(m_pActiveCamera->GetPosition().x, m_pActiveCamera->GetPosition().y, m_pActiveCamera->GetPosition().z, 1.0f) );
	shaderParams.assign( "InvViewProjection", 0, &m_pRenderSystem->GetCamera()->getViewProjectionMatrix().inverse().transpose() );
	m_pSunlight->SetShaderParams( shaderParams );
	m_pSunlight->render( true );
	*/
	
	m_pRenderSystem->DownsampleTexture( m_pHalfRes, m_pGBuffer[1] );
	m_pRenderSystem->ClearTexture( m_pSSAOBuffer, clearColor );
	m_pRenderSystem->SetRenderTarget( m_pSSAOBuffer );

	// TODO : make it easier to get basic stuff like resolution
	RenderSystemConfig config = m_pRenderSystem->GetConfig();
	ShaderParamBlock shaderParams;
	shaderParams.assign( "NoiseScale", 0, &Vector4( config.Width/2.0, config.Height/2.0, 0.0f, 0.0f ) );
	//shaderParams.assign( "ScreenDimensions", 0, &Vector4( config.Width/10.0f, config.Height/10.0f, 0.0f, 0.0f ) );
	shaderParams.assign( "View", 0, &m_pActiveCamera->GetViewMatrix() );
	shaderParams.assign( "Projection", 0, &m_pActiveCamera->GetProjectionMatrix() );
	shaderParams.assign( "InvProjection", 0, &m_pActiveCamera->GetProjectionMatrix().inverse() );
	shaderParams.assign( "fFarPlane", 0, m_pActiveCamera->GetFarPlane() );
	shaderParams.assign( "FovAndAspect", 0, &Vector4( m_pActiveCamera->GetFov(), m_pActiveCamera->GetAspect(), 0.0f, 0.0f ) );
	m_pSSAOPass->SetShaderParams(shaderParams);
	m_pSSAOPass->Render( false );

	m_pRenderSystem->SetRenderTarget( m_pHDRRenderTarget );
	//ShaderParamBlock shaderParams2;
	//shaderParams2.assign( "ScreenDimensions", 0, &Vector4( config.Width/4.0f, config.Height/4.0f, 0.0f, 0.0f ) );
	//m_pSSAOCombinePass->SetShaderParams(shaderParams2);
	m_pSSAOCombinePass->Render( true );
	
	// Fullscreen pass to backbuffer
	m_pRenderSystem->SetBackbufferAsRenderTarget();
	m_pTonemappingPass->Render( false );

	t_scenerender = t.GetMiliseconds();
}

void Scene::ParallelRenderDeferred()
{
	Timer sceneTimer, t;
	t_Cull = 0.0f;

	//
	// Render main scene into gbuffer
	//
	float clearColor[4] = { 0.0f, 0.125f, 0.3f, 1.0f }; //red,green,blue,alpha
	m_pRenderSystem->ClearTexture( m_pHDRRenderTarget, clearColor );
	m_pRenderSystem->ClearTexture( m_pGBuffer[0], clearColor );
	m_pRenderSystem->ClearTexture( m_pGBuffer[1], clearColor );
	m_pRenderSystem->ClearTexture( m_pGBuffer[2], clearColor );

	m_pRenderSystem->SetCamera( m_pActiveCamera );
	Texture2D* MRT[] = { m_pHDRRenderTarget, m_pGBuffer[0], m_pGBuffer[1], m_pGBuffer[2], };
	m_pRenderSystem->SetMultipleRenderTargets( 4, MRT );
	//m_pRenderSystem->SetMultipleRenderTargets( 3, m_pGBuffer );
	//m_pRenderSystem->SetRenderTarget( m_pHDRRenderTarget );
	//m_pRenderSystem->SetBackbufferAsRenderTarget();


	// Render all entities
	tbb::parallel_for_each( m_pEntities.begin(), m_pEntities.end(), [&](Entity* ptr)
	{
		XNA::Frustum frustum = m_pActiveCamera->GetFrustum();

		//t_Cull += sceneTimer.GetMiliseconds();
		ptr->Cull( &frustum );
		ptr->Render();
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
			if ((*it2)->IsVisible()&&(*it2)->IsEnabled())
			{
				// Render shadowmap/RSM
				if ((*it2)->IsCastingShadow())
				{
					if ((*it2)->IsRSMEnabled())
					{
						// Render Reflective Shadow Maps
						m_pRenderSystem->ClearTexture( m_pShadowmap[0], clear0 );
						m_pRenderSystem->ClearTexture( m_pRSMNormal[0], clear1 );
						m_pRenderSystem->ClearTexture( m_pRSMColor[0], clear1 );
						Texture2D* MRT[] = { m_pShadowmap[0], m_pRSMNormal[0], m_pRSMColor[0], };
						m_pRenderSystem->SetMultipleRenderTargets( 3, MRT );
					}
					else
					{
						// Render just the shadowmap
						m_pRenderSystem->ClearTexture( m_pShadowmap[0], clear0 );
						m_pRenderSystem->SetRenderTarget( m_pShadowmap[0] );
					}

					// Cull and render each scene entity into RSM/shadowmap
					tbb::parallel_for_each( m_pEntities.begin(), m_pEntities.end(), [&](Entity* ptr)
					{
						sceneTimer.Start();
						ptr->Cull( &(*it2)->GetFrustum() );
						t_Cull += sceneTimer.GetMiliseconds();
						if ((*it2)->IsRSMEnabled())
							ptr->RenderRSM((*it2)->GetProjectionCamera(), (*it2) );
						else
							ptr->RenderShadowmap((*it2)->GetProjectionCamera());
					});

					if ((*it2)->IsRSMEnabled())
					{
						(*it2)->SetShadowmap( m_pShadowmap[0] );
						(*it2)->SetRSMNormal( m_pRSMNormal[0] );
						(*it2)->SetRSMColor( m_pRSMColor[0] );
						m_pGI->SetTexture( "txShadowmap", m_pShadowmap[0] );
						m_pGI->SetTexture( "txRSMNormal", m_pRSMNormal[0] );
						m_pGI->SetTexture( "txRSMColor", m_pRSMColor[0] );
					}
					else
					{
						(*it2)->SetShadowmap( m_pShadowmap[0] );
					}
				}
				else
				{
					(*it2)->SetShadowmap( m_pSMEmpty );
				}

				// Render light volume
				m_pRenderSystem->SetRenderTarget( m_pHDRRenderTarget );
				LightVolume volume( m_pRenderSystem, *it2 );
				volume.SetGBuffer( m_pGBuffer );
				volume.Render();
				
				// Render GI!
				//if ((*it2)->isRSMEnabled())
					//m_pGI->render(true);
			}
		}
	}
	
	m_pRenderSystem->SetRenderTarget( m_pHDRRenderTarget );
	
	/*ShaderParamBlock shaderParams;
	shaderParams.assign( "vLightVector", 0, &Vector4( Vector3(-0.8f, 0.0f, 1.0f).normalisedCopy() ) );
	shaderParams.assign( "vColor", 0, &Vector4( 1.0f, 1.0f, 0.75f, 1.0f ) );
	shaderParams.assign( "vEyePos", 0, &XMFLOAT4(m_pActiveCamera->GetPosition().x, m_pActiveCamera->GetPosition().y, m_pActiveCamera->GetPosition().z, 1.0f) );
	shaderParams.assign( "InvViewProjection", 0, &m_pRenderSystem->GetCamera()->getViewProjectionMatrix().inverse().transpose().intoXMFLOAT4X4() );
	m_pSunlight->SetShaderParams( shaderParams );
	m_pSunlight->render( true );*/

	//m_pSSAOPass->render( true );

	// Fullscreen pass to backbuffer
	m_pRenderSystem->SetBackbufferAsRenderTarget();
	m_pTonemappingPass->Render(false);

	t_scenerender = t.GetMiliseconds();
}

void Scene::LoadCustomObjects()
{
	// DDO Helmet
	Entity_StaticProp* helmet = new Entity_StaticProp( m_pRenderSystem, m_pMeshes[L"mesh_ddo_helmet"] );
	helmet->SetName( L"DDO Helmet" );
	helmet->Transformation()->SetScale( 0.015f, 0.015f, 0.015f );
	helmet->Transformation()->SetPosition( -10.0, 6.0f, 1.0f );
	helmet->Transformation()->SetOrientation( Quaternion( PI/2.0f, Vector3(1.0f,0.0f,0.0f ) ) );
	helmet->Transformation()->Rotate( Quaternion( PI/2.0f, Vector3(0.0f,1.0f,0.0f) ) );
	AddEntity( helmet );

	// DDO AKS74U
	Entity_StaticProp* aks = new Entity_StaticProp( m_pRenderSystem, m_pMeshes[L"mesh_ddo_aks"] );
	aks->SetName( L"DDO AKS74U" );
	aks->Transformation()->SetScale( 0.15f, 0.15f, 0.15f );
	aks->Transformation()->SetPosition( -10.0, 4.0f, 1.0f );
	aks->Transformation()->SetOrientation( Quaternion( PI/2.0f, Vector3(1.0f,0.0f,0.0f ) ) );
	aks->Transformation()->Rotate( Quaternion( PI/2.0f, Vector3(0.0f,1.0f,0.0f) ) );
	AddEntity( aks );

	// Sphere
	Entity_StaticProp* sphere = new Entity_StaticProp( m_pRenderSystem, m_pMeshes[L"mesh_sphere"] );
	sphere->SetName( L"Sphere" );
	sphere->Transformation()->SetPosition( -10.0, 2.0f, 1.0f );
	AddEntity( sphere );

	// Water
	//Entity_Water* water = new Entity_Water( m_pRenderSystem );
	//water->Transformation()->SetPosition( 0.0f, -20.0f, 0.0f );
	//AddEntity( water );
}

void Scene::LoadFromFile( std::wstring filename )
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
		m_pPhysicsSystem->GetWorld()->lock();

	Entity* entity = NULL;
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
				entity = new Entity_StaticProp(m_pRenderSystem, m_pMeshes[L"mesh_metro_tumba01"]);
				entity->SetName(str);
				AddEntity(entity);
				transform = entity->Transformation();
				rigidBody = NULL;
			}
			else if (class_string==L"ent_Lantai")
			{
				currentSpotLight = NULL;
				currentPointLight = NULL;
				//DX11Material_BlinnPhong* mat = new DX11Material_BlinnPhong( m_pRenderSystem );
				DX11Material_Deferred* mat = new DX11Material_Deferred( m_pRenderSystem );
				mat->SetDiffusemap( m_pRenderSystem->LoadTexture( L"Media/tile34_revol_floor2.1024c.bmp" ) );
				mat->SetNormalmap( m_pRenderSystem->LoadTexture( L"Media/tile34_revol_floor_normal.1024c.bmp" ) );
				mat->SetSpecularMap( m_pRenderSystem->LoadTexture( L"Media/tile34_revol_floor_gloss.1024c.bmp" ) );/*
				mat->SetSpecularMap( m_pRenderSystem->LoadTexture( L"Media/grey.bmp" ) );
				mat->SetDiffusemap( m_pRenderSystem->CreateTextureFromFile( L"Media/wood34_wallconstr_01.2048.bmp" ) );
				mat->SetNormalmap( m_pRenderSystem->CreateTextureFromFile( L"Media/wood34_wallconstr_01_normal.2048.bmp" ) );
				mat->SetSpecularMap( m_pRenderSystem->CreateTextureFromFile( L"Media/wood34_wallconstr_01_bump.2048.bmp" ) );/*
				mat->SetDiffusemap( m_pRenderSystem->CreateTextureFromFile( L"Media/metal_locker.2048.bmp" ) );
				mat->SetNormalmap( m_pRenderSystem->CreateTextureFromFile( L"Media/metal_locker_normal.2048.bmp" ) );
				mat->SetSpecularMap( m_pRenderSystem->CreateTextureFromFile( L"Media/metal_locker_bump.2048.bmp" ) );
				mat->SetDiffusemap( m_pRenderSystem->CreateTextureFromFile( L"Media/Concrete/Metro_0047.bmp" ) );
				mat->SetNormalmap( m_pRenderSystem->CreateTextureFromFile( L"Media/Concrete/Metro_0047n.bmp" ) );
				mat->SetSpecularMap( m_pRenderSystem->CreateTextureFromFile( L"Media/Concrete/Metro_0047s.bmp" ) );
				mat->SetDiffusemap( m_pRenderSystem->CreateTextureFromFile( L"Media/Concrete/Metro_0039.bmp" ) );
				mat->SetNormalmap( m_pRenderSystem->CreateTextureFromFile( L"Media/Concrete/Metro_0039n.bmp" ) );
				mat->SetSpecularMap( m_pRenderSystem->CreateTextureFromFile( L"Media/Concrete/Metro_0039s.bmp" ) );*/

				Mesh* mesh = m_pRenderSystem->CreatePlaneMesh( XMFLOAT2(10.0f,10.0f), XMFLOAT2(4.0f,4.0f) );
				mesh->SetMaterial(mat);
				hkpRigidBody* rb = CreateBoxRigidBody( Vector3(5.0f,5.0f,0.5f), 0 );
				Transform rigidBodyRelative;
				rigidBodyRelative.SetPosition(0.0f,0.0f,-0.5f);
				entity = new Entity_Prop(m_pRenderSystem, m_pPhysicsSystem, mesh, rb, rigidBodyRelative);
				entity->SetName(str);
				AddEntity(entity);
				transform = entity->Transformation();
				rigidBody = rb;
			}
			else if (class_string==L"ent_Langitlangit")
			{
				currentSpotLight = NULL;
				currentPointLight = NULL;
				//DX11Material_BlinnPhong* mat = new DX11Material_BlinnPhong( m_pRenderSystem );
				DX11Material_Deferred* mat = new DX11Material_Deferred( m_pRenderSystem );
				mat->SetDiffusemap( m_pRenderSystem->LoadTexture( L"Media/concrete_opalubka_3.1024.bmp" ) );
				mat->SetNormalmap( m_pRenderSystem->LoadTexture( L"Media/concrete_opalubka_3_normal.1024.bmp" ) );
				mat->SetSpecularMap( m_pRenderSystem->LoadTexture( L"Media/concrete_opalubka_3_bump.1024.bmp" ) );
				
				entity = new Entity_Plane(m_pRenderSystem,mat,XMFLOAT2(10.0f,10.0f),XMFLOAT2(3.33f,3.33f));
				entity->SetName(str);
				AddEntity(entity);
				transform = entity->Transformation();
				rigidBody = NULL;
			}
			else if (class_string==L"ent_Tembok")
			{
				currentSpotLight = NULL;
				currentPointLight = NULL;
				//DX11Material_BlinnPhong* mat = new DX11Material_BlinnPhong( m_pRenderSystem );
				DX11Material_Deferred* mat = new DX11Material_Deferred( m_pRenderSystem );
				mat->SetDiffusemap( m_pRenderSystem->LoadTexture( L"Media/wall_stucaturka_stena1.1024c.bmp" ) );
				mat->SetNormalmap( m_pRenderSystem->LoadTexture( L"Media/wall_stucaturka_stena1_normal.2048.bmp" ) );
				mat->SetSpecularMap( m_pRenderSystem->LoadTexture( L"Media/wall_stucaturka_stena1_bump.2048.bmp" ) );

				//ent = new Entity_Plane( m_pRenderSystem, mat, XMFLOAT2(10.0f,3.0f),XMFLOAT2(-3.33f,-1.0f) );
				Mesh* mesh = m_pRenderSystem->CreatePlaneMesh( XMFLOAT2(10.0f,3.0f),XMFLOAT2(-3.33f,-1.0f) );
				mesh->SetMaterial(mat);
				hkpRigidBody* rb = CreateBoxRigidBody( Vector3(5.0f,1.5f,0.1f), 0 );
				entity = new Entity_Prop(m_pRenderSystem, m_pPhysicsSystem, mesh, rb);
				entity->SetName(str);
				AddEntity(entity);
				transform = entity->Transformation();
				rigidBody = rb;
			}
			else if (class_string==L"SpotLight")
			{
				currentPointLight = NULL;
				currentSpotLight = new SpotLight();
				transform = currentSpotLight->Transformation();
				AddSpotLight( currentSpotLight );
				rigidBody = NULL;
			}
			else if (class_string==L"PointLight")
			{
				currentSpotLight = NULL;
				currentPointLight = new PointLight();
				transform = currentPointLight->Transformation();
				AddPointLight( currentPointLight );
				rigidBody = NULL;
			}
			else if (m_pMeshes.find(class_string)!=m_pMeshes.end())
			{
				currentSpotLight = NULL;
				currentPointLight = NULL;
				Mesh* mesh = m_pMeshes[class_string];
				entity = new Entity_StaticProp( m_pRenderSystem, mesh );
				entity->SetName(str);
				AddEntity(entity);
				transform = entity->Transformation();
				rigidBody = NULL;
			}
			else if (m_pPrefabs.find(class_string)!=m_pPrefabs.end())
			{
				currentSpotLight = NULL;
				currentPointLight = NULL;
				Entity_Prop* entity_ = new Entity_Prop(*m_pPrefabs[class_string]);	// Copy constructor
				entity_->SetName(str);
				AddEntity(entity_);
				transform = entity_->Transformation();
				rigidBody = entity_->m_pRigidBody;
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
				transform->SetOrientation(w,x,y,z);
			if (rigidBody)
				rigidBody->setRotation(hkQuaternion(x,y,z,w));
		}
		else if (swscanf(line.c_str(),L"position = [%f,%f,%f]",&x,&y,&z)==3)
		{
			if (transform)
				transform->SetPosition(x,y,z);
			if (rigidBody)
				rigidBody->setPosition(hkVector4(x,y,z));
		}
		else if (swscanf(line.c_str(),L"scale = [%f,%f,%f]",&x,&y,&z)==3)
		{
			if (transform)
				transform->SetScale(x,y,z);
			if (rigidBody)
			{
				transform->SetScale(1.0f,1.0f,1.0f);
				/*
				hkArray<hkpShapeScalingUtility::ShapePair> shapes;
				// Scale havok's shape rePresentation
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
				currentSpotLight->PointTo( Vector3(x,y,z), 0.0f );
		}
		else if (swscanf(line.c_str(),L"color = (color %i %i %i)",&r,&g,&b)==3)
		{
			if (currentSpotLight)
				currentSpotLight->SetColor( Vector3(r/255.0f,g/255.0f,b/255.0f) );
			else if (currentPointLight)
				currentPointLight->SetColor( Vector3(r/255.0f,g/255.0f,b/255.0f) );
		}
		else if (swscanf(line.c_str(),L"radius = %f",&x)==1)
		{
			if (currentSpotLight)
				currentSpotLight->SetRadius(x);
			else if (currentPointLight)
				currentPointLight->SetRadius(x);
		}
		else if (swscanf(line.c_str(),L"intensity = %f",&x)==1)
		{
			if (currentSpotLight)
				currentSpotLight->SetIntensity(x*15.0f);
			else if (currentPointLight)
				currentPointLight->SetIntensity(x*15.0f);
		}
		else if (swscanf(line.c_str(),L"falloff = %f",&x)==1)
		{
			falloff = x;
		}
		else if (swscanf(line.c_str(),L"aspect = %f",&x)==1)
		{
			if (currentSpotLight)
				currentSpotLight->SetCone(Vector2((falloff*x)*D3DX_PI/180.0f,(falloff/x)*D3DX_PI/180.0f));
		}
		else if (swscanf(line.c_str(),L"cookie = %s",&cookie_str)==1)
		{
			cookie_string = cookie_str;
			if (currentSpotLight)
			{
				Texture2D* tex = m_pRenderSystem->LoadTexture( cookie_string );
				if (tex)
					currentSpotLight->SetCookie( tex );
				else
					currentSpotLight->SetCookie( m_pRenderSystem->LoadTexture( L"Media/notexture.bmp" ) );
			}
		}
	}
	
	if (m_pPhysicsSystem)
		m_pPhysicsSystem->GetWorld()->unlock();
	stream.close();
}

void Scene::LoadMeshes()
{
	
	RenderSystem *ptr = m_pRenderSystem;
	
	//--metro_kursi
	//props_bibl_stul
	//D:/Metro Last Light/content/textures/props/props_bibl_stul.512

	DX11Material_DiffuseDetailbump* mat0 = new DX11Material_DiffuseDetailbump(ptr);
	mat0->SetDiffusemap( ptr->LoadTexture( L"Media/props_bibl_stul.512.bmp" ) );
	mat0->SetNormalmap( ptr->LoadTexture( L"Media/props_bibl_stul_normal.1024.bmp" ) );
	mat0->SetDetailNormalmap( ptr->LoadTexture( L"Media/Tex_0010_5.dds" ) );
	mat0->SetDetailNormalStrength( 2.0f );
	mat0->SetDetailTiling( 10.0f );
	mat0->SetSpecularIntensity( 0.34f );
	mat0->SetSpecularPower( 32.0f );

	Mesh* pMesh = ptr->LoadMesh( "Media/Meshes/props_bibl_stul.obj" );
	pMesh->SetMaterial( mat0 );
	m_pMeshes[L"props_bibl_stul"] = pMesh;


	//props_chair_01
	//D:/Metro Last Light/content/textures/props/props_chair_01.512
	//D:/Metro Last Light/content/textures/props/props_chair_01.512

	mat0 = new DX11Material_DiffuseDetailbump(ptr);
	mat0->SetDiffusemap( ptr->LoadTexture( L"Media/props_chair_01.512.bmp" ) );
	mat0->SetNormalmap( ptr->LoadTexture( L"Media/props_chair_01_normal.512.bmp" ) );
	mat0->SetDetailNormalmap( ptr->LoadTexture( L"Media/Tex_0010_5.dds" ) );
	mat0->SetDetailNormalStrength( 3.0f );
	mat0->SetDetailTiling( 8.0f );
	mat0->SetSpecularIntensity( 0.14f );
	mat0->SetSpecularPower( 8.0f );

	pMesh = ptr->LoadMesh( "Media/Meshes/props_chair_01.obj" );
	pMesh->SetMaterial( mat0 );
	m_pMeshes[L"props_chair_01"] = pMesh;

	//props_chair_school_a
	//D:/Metro Last Light/content/textures/props/props_chair_school.512

	mat0 = new DX11Material_DiffuseDetailbump(ptr);
	mat0->SetDiffusemap( ptr->LoadTexture( L"Media/props_chair_school.512.bmp" ) );
	mat0->SetNormalmap( ptr->LoadTexture( L"Media/props_chair_school_normal.1024.bmp" ) );
	mat0->SetDetailNormalmap( ptr->LoadTexture( L"Media/Tex_0010_5.dds" ) );
	mat0->SetDetailNormalStrength( 2.0f );
	mat0->SetDetailTiling( 10.0f );
	mat0->SetSpecularIntensity( 0.34f );
	mat0->SetSpecularPower( 32.0f );

	pMesh = ptr->LoadMesh( "Media/Meshes/props_chair_school_a.obj" );
	pMesh->SetMaterial( mat0 );
	m_pMeshes[L"props_chair_school_a"] = pMesh;


	//props_chair_school_b
	//D:/Metro Last Light/content/textures/props/props_chair_school.512
	mat0 = new DX11Material_DiffuseDetailbump(ptr);
	mat0->SetDiffusemap( ptr->LoadTexture( L"Media/props_chair_school.512.bmp" ) );
	mat0->SetNormalmap( ptr->LoadTexture( L"Media/props_chair_school_normal.1024.bmp" ) );
	mat0->SetDetailNormalmap( ptr->LoadTexture( L"Media/Tex_0010_5.dds" ) );
	mat0->SetDetailNormalStrength( 2.0f );
	mat0->SetDetailTiling( 10.0f );
	mat0->SetSpecularIntensity( 0.34f );
	mat0->SetSpecularPower( 32.0f );

	pMesh = ptr->LoadMesh( "Media/Meshes/props_chair_school_b.obj" );
	pMesh->SetMaterial( mat0 );
	m_pMeshes[L"props_chair_school_b"] = pMesh;

	//--metro_lockers
	//metro_locker_a ~ metro_locker_e
	//D:/Metro Last Light/content/textures/metal/metal_locker.512
	DX11Material_Deferred *mat0b = new DX11Material_Deferred(ptr);
	//DX11Material_BlinnPhong *mat0b = new DX11Material_BlinnPhong(ptr);
	mat0b->SetDiffusemap( ptr->LoadTexture( L"Media/metal_locker.2048.bmp" ) );
	mat0b->SetNormalmap( ptr->LoadTexture( L"Media/metal_locker_normal.2048.bmp" ) );
	mat0b->SetSpecularMap( ptr->LoadTexture( L"Media/metal_locker_bump.2048.bmp" ) );
	//mat0->SetDetailNormalmap( ptr->LoadTexture( L"Media/Tex_0010_5.dds" ) );
	//mat0->SetDetailNormalStrength( 2.0f );
	//mat0->SetDetailTiling( 10.0f );
	//mat0->SetSpecularIntensity( 0.34f );
	//mat0->SetSpecularPower( 32.0f );
	mat0b->SetSpecularIntensity( 0.5f );
	//mat0b->SetSpecularPower( 1024.0f );
	mat0b->SetGlossiness( 1024.0f );

	pMesh = ptr->LoadMesh( "Media/Meshes/metro_locker_a.obj" );
	pMesh->SetMaterial( mat0b );
	m_pMeshes[L"metro_locker_a"] = pMesh;

	pMesh = ptr->LoadMesh( "Media/Meshes/metro_locker_b.obj" );
	pMesh->SetMaterial( mat0b );
	m_pMeshes[L"metro_locker_b"] = pMesh;

	pMesh = ptr->LoadMesh( "Media/Meshes/metro_locker_c.obj" );
	pMesh->SetMaterial( mat0b );
	m_pMeshes[L"metro_locker_c"] = pMesh;

	pMesh = ptr->LoadMesh( "Media/Meshes/metro_locker_d.obj" );
	pMesh->SetMaterial( mat0b );
	m_pMeshes[L"metro_locker_d"] = pMesh;

	pMesh = ptr->LoadMesh( "Media/Meshes/metro_locker_e.obj" );
	pMesh->SetMaterial( mat0b );
	m_pMeshes[L"metro_locker_e"] = pMesh;

	pMesh = ptr->LoadMesh( "Media/Meshes/metro_locker_f.obj" );
	pMesh->SetMaterial( mat0b );
	m_pMeshes[L"metro_locker_f"] = pMesh;


	//--Clutter_paper_cardboard
	//Clutter_paper_cardboard_a ~ Clutter_paper_cardboard_e
	//Tex_0013_1.dds
	{
		DX11Material_Deferred* mat0 = new DX11Material_Deferred(ptr);
		mat0->SetDiffusemap( ptr->LoadTexture( L"Media/Prop_Pack_V1/Clutter_paper_cardboard/Tex_0013_1.dds" ) );
		mat0->SetNormalmap( ptr->LoadTexture( L"Media/Prop_Pack_V1/Clutter_paper_cardboard/normal.bmp" ) );
		//mat0->SetDetailNormalmap( ptr->LoadTexture( L"Media/Tex_0010_5.dds" ) );
		//mat0->SetDetailTiling( 6.0f );
		//mat0->SetDetailNormalStrength( 3.0f );
		mat0->SetSpecularIntensity( 0.1f );
		//mat0->SetSpecularPower( 12.0f );

		pMesh = ptr->LoadMesh( "Media/Meshes/Clutter_paper_cardboard_a.obj" );
		pMesh->SetMaterial( mat0 );
		m_pMeshes[L"Clutter_paper_cardboard_a"] = pMesh;

		pMesh = ptr->LoadMesh( "Media/Meshes/Clutter_paper_cardboard_b.obj" );
		pMesh->SetMaterial( mat0 );
		m_pMeshes[L"Clutter_paper_cardboard_b"] = pMesh;

		pMesh = ptr->LoadMesh( "Media/Meshes/Clutter_paper_cardboard_c.obj" );
		pMesh->SetMaterial( mat0 );
		m_pMeshes[L"Clutter_paper_cardboard_c"] = pMesh;

		pMesh = ptr->LoadMesh( "Media/Meshes/Clutter_paper_cardboard_d.obj" );
		pMesh->SetMaterial( mat0 );
		m_pMeshes[L"Clutter_paper_cardboard_d"] = pMesh;

		pMesh = ptr->LoadMesh( "Media/Meshes/Clutter_paper_cardboard_e.obj" );
		pMesh->SetMaterial( mat0 );
		m_pMeshes[L"Clutter_paper_cardboard_e"] = pMesh;
	}

	//--Pallet_Single
	//Pallet_Single
	//Tex_0030_1.dds
	{
		DX11Material_Deferred *mat = new DX11Material_Deferred(ptr);
		mat->SetDiffusemap( ptr->LoadTexture( L"Media/Prop_Pack_V1/Pallet_Single/Tex_0030_1.dds" ) );
		mat->SetNormalmap( ptr->LoadTexture( L"Media/flat.bmp" ) );
		//mat->SetDetailNormalmap( ptr->LoadTexture( L"Media/Tex_0010_5.dds" ) );
		//mat->SetDetailTiling( 6.0f );
		//mat->SetDetailNormalStrength( 3.0f );
		mat->SetSpecularIntensity( 0.1f );
		//mat->SetSpecularPower( 12.0f );

		pMesh = ptr->LoadMesh( "Media/Meshes/Pallet_Single.obj" );
		pMesh->SetMaterial( mat0 );
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
		mat_b0->SetDiffusemap( ptr->LoadTexture( L"Media/props_m_scaf01.512.bmp" ) );
		mat_b0->SetNormalmap( ptr->LoadTexture( L"Media/props_m_scaf01_normal.512.bmp" ) );
		mat_b0->SetSpecularMap( ptr->LoadTexture( L"Media/props_m_scaf01_bump.512.bmp" ) );
		//mat0->SetDetailNormalmap( ptr->LoadTexture( L"Media/Tex_0010_5.dds" ) );
		//mat0->SetDetailNormalStrength( 2.0f );
		//mat0->SetDetailTiling( 10.0f );
		mat_b0->SetSpecularIntensity( 0.7f );
		//mat_b0->SetSpecularPower( 512.0f );

		DX11Material_Deferred* mat_b1 = new DX11Material_Deferred(ptr);
		mat_b1->SetDiffusemap( ptr->LoadTexture( L"Media/props_m_scaf02.512.bmp" ) );
		mat_b1->SetNormalmap( ptr->LoadTexture( L"Media/props_m_scaf02_normal.1024.bmp" ) );
		mat_b1->SetSpecularMap( ptr->LoadTexture( L"Media/props_m_scaf02_bump.1024.bmp" ) );
		//mat1->SetDetailNormalmap( ptr->LoadTexture( L"Media/Tex_0010_5.dds" ) );
		//mat1->SetDetailNormalStrength( 2.0f );
		//mat1->SetDetailTiling( 10.0f );
		mat_b1->SetSpecularIntensity( 0.7f );
		//mat_b1->SetSpecularPower( 512.0f );

		pMesh = ptr->LoadMesh( "Media/Meshes/props_m_scaf_a.obj" );
		pMesh->SetMaterial( mat_b0 );
		pMesh->GetSubmesh(1)->SetMaterial( mat_b1 );
		m_pMeshes[L"props_m_scaf_a"] = pMesh;

		pMesh = ptr->LoadMesh( "Media/Meshes/props_m_scaf_b.obj" );
		pMesh->SetMaterial( mat_b0 );
		pMesh->GetSubmesh(1)->SetMaterial( mat_b1 );
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
		mat0->SetDiffusemap( ptr->LoadTexture( L"Media/props_books_01.512.bmp" ) );
		mat0->SetNormalmap( ptr->LoadTexture( L"Media/props_books_01_normal.512.bmp" ) );
		//mat0->SetDetailNormalmap( ptr->LoadTexture( L"Media/Tex_0010_5.dds" ) );
		//mat0->SetDetailNormalStrength( 2.0f );
		//mat0->SetDetailTiling( 10.0f );
		mat0->SetSpecularIntensity( 0.34f );
		mat0->SetGlossiness( 32.0f );
		//mat0->SetSpecularPower( 32.0f );

		//DX11Material_DiffuseDetailbump* mat1 = new DX11Material_DiffuseDetailbump(ptr);
		DX11Material_Deferred* mat1 = new DX11Material_Deferred(ptr);
		mat1->SetDiffusemap( ptr->LoadTexture( L"Media/wood_box_2.1024.bmp" ) );
		mat1->SetNormalmap( ptr->LoadTexture( L"Media/wood_box_2_normal.1024.bmp" ) );
		//mat1->SetDetailNormalmap( ptr->LoadTexture( L"Media/Tex_0010_5.dds" ) );
		//mat1->SetDetailNormalStrength( 2.0f );
		//mat1->SetDetailTiling( 10.0f );
		mat1->SetSpecularIntensity( 0.34f );
		mat1->SetGlossiness( 32.0f );
		//mat1->SetSpecularPower( 32.0f );

		pMesh = ptr->LoadMesh( "Media/Meshes/wood_box_a.obj" );
		pMesh->SetMaterial( mat0 );
		pMesh->GetSubmesh(0)->SetMaterial( mat1 );
		m_pMeshes[L"wood_box_a"] = pMesh;

		pMesh = ptr->LoadMesh( "Media/Meshes/wood_box_b.obj" );
		pMesh->SetMaterial( mat1 );
		m_pMeshes[L"wood_box_b"] = pMesh;

		pMesh = ptr->LoadMesh( "Media/Meshes/wood_box_c.obj" );
		pMesh->SetMaterial( mat0 );
		pMesh->GetSubmesh(1)->SetMaterial( mat1 );
		m_pMeshes[L"wood_box_c"] = pMesh;

		pMesh = ptr->LoadMesh( "Media/Meshes/wood_box_d.obj" );
		pMesh->SetMaterial( mat0 );
		pMesh->GetSubmesh(1)->SetMaterial( mat1 );
		m_pMeshes[L"wood_box_d"] = pMesh;

		pMesh = ptr->LoadMesh( "Media/Meshes/wood_box_e.obj" );
		pMesh->SetMaterial( mat0 );
		pMesh->GetSubmesh(1)->SetMaterial( mat1 );
		m_pMeshes[L"wood_box_e"] = pMesh;
	}


	//--metro_strongboxes
	//props_met_skaff_a~props_met_skaff_b
	//D:/Metro Last Light/content/textures/props/props_met_skaff.512
	{
		DX11Material_Deferred *matb0 = new DX11Material_Deferred(ptr);
		matb0->SetDiffusemap( ptr->LoadTexture( L"Media/props_met_skaff.1024.bmp" ) );
		matb0->SetNormalmap( ptr->LoadTexture( L"Media/props_met_skaff_normal.1024.bmp" ) );
		matb0->SetSpecularMap( ptr->LoadTexture( L"Media/props_met_skaff_bump.1024.bmp" ) );
		//mat0->SetDetailNormalmap( ptr->LoadTexture( L"Media/Tex_0010_5.dds" ) );
		//mat0->SetDetailNormalStrength( 2.0f );
		//mat0->SetDetailTiling( 10.0f );
		matb0->SetSpecularIntensity( 0.7f );
		//matb0->SetSpecularPower( 2048.0f );

		pMesh = ptr->LoadMesh( "Media/Meshes/props_met_skaff_a.obj" );
		pMesh->SetMaterial( matb0 );
		m_pMeshes[L"props_met_skaff_a"] = pMesh;

		pMesh = ptr->LoadMesh( "Media/Meshes/props_met_skaff_b.obj" );
		pMesh->SetMaterial( matb0 );
		m_pMeshes[L"props_met_skaff_b"] = pMesh;

		pMesh = ptr->LoadMesh( "Media/Meshes/props_met_skaff_c.obj" );
		pMesh->SetMaterial( matb0 );
		m_pMeshes[L"props_met_skaff_c"] = pMesh;

		pMesh = ptr->LoadMesh( "Media/Meshes/props_met_skaff_d.obj" );
		pMesh->SetMaterial( matb0 );
		m_pMeshes[L"props_met_skaff_d"] = pMesh;
	}

	// kardus
	// kardus_a ~ kardus_j
	{
		//DX11Material_DiffuseDetailbump* mat = new DX11Material_DiffuseDetailbump(ptr);
		DX11Material_Deferred* mat = new DX11Material_Deferred(ptr);
		mat->SetDiffusemap( ptr->LoadTexture( L"Media/props_korobka.1024.bmp" ) );
		mat->SetNormalmap( ptr->LoadTexture( L"Media/props_korobka_normal.1024.bmp" ) );
		mat->SetSpecularMap( ptr->LoadTexture( L"Media/grey.bmp" ) );
		//mat->SetDetailNormalmap( ptr->LoadTexture( L"Media/Tex_0010_5.dds" ) );
		//mat->SetDetailNormalStrength( 2.0f );
		//mat->SetDetailTiling( 5.0f );
		mat->SetSpecularIntensity( 0.24f );
		mat->SetGlossiness( 1024.0f );
		//mat->SetSpecularPower( 16.0f );

		Mesh* mesh = ptr->LoadMesh( "Media/Meshes/kardus_a.obj" );
		mesh->SetMaterial( mat );
		m_pMeshes[L"kardus_a"] = mesh;

		mesh = ptr->LoadMesh( "Media/Meshes/kardus_b.obj" );
		mesh->SetMaterial( mat );
		m_pMeshes[L"kardus_b"] = mesh;

		mesh = ptr->LoadMesh( "Media/Meshes/kardus_c.obj" );
		mesh->SetMaterial( mat );
		m_pMeshes[L"kardus_c"] = mesh;

		mesh = ptr->LoadMesh( "Media/Meshes/kardus_d.obj" );
		mesh->SetMaterial( mat );
		m_pMeshes[L"kardus_d"] = mesh;

		mesh = ptr->LoadMesh( "Media/Meshes/kardus_e.obj" );
		mesh->SetMaterial( mat );
		m_pMeshes[L"kardus_e"] = mesh;

		mesh = ptr->LoadMesh( "Media/Meshes/kardus_f.obj" );
		mesh->SetMaterial( mat );
		m_pMeshes[L"kardus_f"] = mesh;

		mesh = ptr->LoadMesh( "Media/Meshes/kardus_g.obj" );
		mesh->SetMaterial( mat );
		m_pMeshes[L"kardus_g"] = mesh;

		mesh = ptr->LoadMesh( "Media/Meshes/kardus_h.obj" );
		mesh->SetMaterial( mat );
		m_pMeshes[L"kardus_h"] = mesh;

		//DX11Material_DiffuseDetailbump* mat1 = new DX11Material_DiffuseDetailbump(ptr);
		DX11Material_Deferred* mat1 = new DX11Material_Deferred(ptr);
		mat1->SetDiffusemap( ptr->LoadTexture( L"Media/wood_box_2.1024.bmp" ) );
		mat1->SetNormalmap( ptr->LoadTexture( L"Media/wood_box_2_normal.1024.bmp" ) );
		//mat1->SetDetailNormalmap( ptr->LoadTexture( L"Media/Tex_0010_5.dds" ) );
		//mat1->SetDetailNormalStrength( 3.0f );
		//mat1->SetDetailTiling( 20.0f );
		mat1->SetSpecularIntensity( 0.14f );
		//mat1->SetSpecularPower( 16.0f );

		mesh = ptr->LoadMesh( "Media/Meshes/kardus_i.obj" );
		mesh->SetMaterial( mat1 );
		m_pMeshes[L"kardus_i"] = mesh;

		mesh = ptr->LoadMesh( "Media/Meshes/kardus_j.obj" );
		mesh->SetMaterial( mat1 );
		m_pMeshes[L"kardus_j"] = mesh;
	}

	// mesh: "metro_kasur"
	// prefab: "prop_metro_kasur"
	{
		//DX11Material_DiffuseDetailbump* mat0 = new DX11Material_DiffuseDetailbump(ptr);
		DX11Material_Deferred* mat0 = new DX11Material_Deferred(ptr);
		mat0->SetDiffusemap( ptr->LoadTexture( L"Media/metal34_props_01.1024.dds" ) );
		mat0->SetNormalmap( ptr->LoadTexture( L"Media/metal34_props_01_normal.2048.bmp" ) );
		//mat0->SetDetailNormalmap( ptr->LoadTexture( L"Media/Tex_0010_5.dds" ) );
		//mat0->SetDetailNormalStrength( 5.0f );
		//mat0->SetDetailTiling( 12.0f );
		mat0->SetSpecularIntensity( 0.3f );
		mat0->SetGlossiness( 32.0f );
		//mat0->SetSpecularPower( 32.0f );

		//DX11Material_DiffuseDetailbump* mat1 = new DX11Material_DiffuseDetailbump(ptr);
		DX11Material_Deferred* mat1 = new DX11Material_Deferred(ptr);
		mat1->SetDiffusemap( ptr->LoadTexture( L"Media/props34_san_kol.2048.crn.bmp" ) );
		mat1->SetNormalmap( ptr->LoadTexture( L"Media/props34_san_kol_normal.2048.crn.bmp" ) );
		//mat1->SetDetailNormalmap( ptr->LoadTexture( L"Media/Tex_0010_5.dds" ) );
		//mat1->SetDetailNormalStrength( 5.0f );
		//mat1->SetDetailTiling( 12.0f );
		mat1->SetSpecularIntensity( 0.3f );
		mat1->SetGlossiness( 32.0f );
		//mat1->SetSpecularPower( 32.0f );

		//DX11Material_DiffuseDetailbump* mat2 = new DX11Material_DiffuseDetailbump(ptr);
		DX11Material_Deferred* mat2 = new DX11Material_Deferred(ptr);
		mat2->SetDiffusemap( ptr->LoadTexture( L"Media/props_mattress.512.dds" ) );
		mat2->SetNormalmap( ptr->LoadTexture( L"Media/props_mattress_normal.512.bmp" ) );
		//mat2->SetDetailNormalmap( ptr->LoadTexture( L"Media/Tex_0010_5.dds" ) );
		//mat2->SetDetailNormalStrength( 2.0f );
		//mat2->SetDetailTiling( 8.0f );
		mat2->SetSpecularIntensity( 0.15f );
		mat2->SetGlossiness( 16.0f );
		//mat2->SetSpecularPower( 16.0f );

		//DX11Material_DiffuseDetailbump* mat3 = new DX11Material_DiffuseDetailbump(ptr);
		DX11Material_Deferred* mat3 = new DX11Material_Deferred(ptr);
		mat3->SetDiffusemap( ptr->LoadTexture( L"Media/props_fabric.2048.dds" ) );
		mat3->SetNormalmap( ptr->LoadTexture( L"Media/props_fabric_normal.2048.bmp" ) );
		//mat3->SetDetailNormalmap( ptr->LoadTexture( L"Media/Tex_0010_5.dds" ) );
		//mat3->SetDetailNormalStrength( 1.0f );
		//mat3->SetDetailTiling( 12.0f );
		mat3->SetSpecularIntensity( 0.15f );
		mat3->SetGlossiness( 16.0f );
		//mat3->SetSpecularPower( 16.0f );

		Mesh* mesh = ptr->LoadMesh( "Media/metro_kasur.obj" );
		mesh->SetMaterial( mat2 );
		mesh->GetSubmesh(2)->SetMaterial( mat3 );
		mesh->GetSubmesh(3)->SetMaterial( mat0 );
		m_pMeshes[L"metro_kasur"] = mesh;
	}

	// Prefab : prop_metro_kursiroda
	// Mesh : metro_kursiroda
	{
		//DX11Material_DiffuseDetailbump* pMaterial = new DX11Material_DiffuseDetailbump(ptr);
		DX11Material_Deferred* mat = new DX11Material_Deferred(ptr);
		mat->SetDiffusemap( ptr->LoadTexture( L"Media/props34_san_kol.2048.crn.bmp" ) );
		mat->SetNormalmap( ptr->LoadTexture( L"Media/props34_san_kol_normal.2048.crn.bmp" ) );
		//mat->SetDetailNormalmap( ptr->LoadTexture( L"Media/Tex_0010_5.dds" ) );
		//mat->SetDetailNormalStrength( 3.0f );
		//mat->SetDetailTiling( 12.0f );
		mat->SetSpecularIntensity( 0.22f );
		mat->SetGlossiness( 12.0f );
		//mat->SetSpecularPower( 12.0f );

		Mesh* mesh = ptr->LoadMesh( "Media/metro_kursiroda.3DS" );
		mesh->SetMaterial( mat );

		m_pMeshes[L"metro_kursiroda"] = mesh;
	}

	// Prefab : prop_metro_tumba01
	// Mesh : mesh_metro_tumba01
	{
		DX11Material_Deferred* mat0 = new DX11Material_Deferred(ptr);
		mat0->SetDiffusemap( ptr->LoadTexture( L"Media/props_shkaf_01.1024.bmp" ) );
		mat0->SetNormalmap( ptr->LoadTexture( L"Media/props_shkaf_01_normal.1024.bmp" ) );
		//mat0->SetDetailNormalmap( ptr->LoadTexture( L"Media/Tex_0010_5.dds" ) );
		//mat0->SetDetailNormalStrength( 2.0f );
		//mat0->SetDetailTiling( 10.0f );
		//mat0->SetSpecularIntensity( 0.34f );
		//mat0->SetSpecularPower( 32.0f );

		DX11Material_Deferred* mat1 = new DX11Material_Deferred(ptr);
		mat1->SetDiffusemap( ptr->LoadTexture( L"Media/props_shkaf_01.1024.bmp" ) );
		mat1->SetNormalmap( ptr->LoadTexture( L"Media/props_shkaf_01_normal.1024.bmp" ) );
		//mat1->SetDetailNormalmap( ptr->LoadTexture( L"Media/Tex_0010_5.dds" ) );
		//mat1->SetDetailNormalStrength( 2.0f );
		//mat1->SetDetailTiling( 10.0f );
		//mat1->SetSpecularIntensity( 0.34f );
		//mat1->SetSpecularPower( 32.0f );

		Mesh* mesh = ptr->LoadMesh( "Media/metro_tumba01.obj" );
		mesh->SetMaterial( mat0 );
		mesh->GetSubmesh(1)->SetMaterial( mat1 );

		m_pMeshes[L"mesh_metro_tumba01"] = mesh;
	}
	
	TextureCube* IBL = (TextureCube*)ptr->LoadTexture( L"Media/cubemap0.dds" );
	IBL->Prefilter();

	// Prefab : prop_ddo_helmet
	// Mesh : mesh_ddo_helmet
	{
		DX11Material_DeferredIBL* mat = new DX11Material_DeferredIBL(ptr);
		mat->SetDiffusemap( ptr->LoadTexture( L"Media/DDO_SDK_Helmet/albedo.jpg" ) );
		//mat->SetDiffusemap( ptr->LoadTexture( L"Media/grey.bmp" ) );
		mat->SetNormalmap( ptr->LoadTexture( L"Media/DDO_SDK_Helmet/normal.jpg" ) );
		mat->SetSpecularMap( ptr->LoadTexture( L"Media/DDO_SDK_Helmet/specular.jpg" ) );
		mat->SetAOMap( ptr->LoadTexture( L"Media/DDO_SDK_Helmet/ao.jpg" ) );
		//mat->SetSpecularMap( ptr->LoadTexture( L"Media/test_specular1.bmp" ) );
		//mat->SetDetailNormalmap( ptr->LoadTexture( L"Media/Tex_0010_5.dds" ) );
		//mat->SetDetailNormalStrength( 3.0f );
		//mat->SetDetailTiling( 12.0f );
		mat->SetSpecularIntensity( 1.0f );
		mat->SetGlossiness( 2048.0f );
		//mat->SetSpecularPower( 12.0f );

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
		//mat->SetIBL( ptr->CreateCubemap( faces ) );
		mat->SetIBL( IBL );

		Mesh* mesh = ptr->LoadMesh( "Media/DDO_SDK_Helmet/HelmetMeshTriCentered.obj" );
		mesh->SetMaterial( mat );

		m_pMeshes[L"mesh_ddo_helmet"] = mesh;
	}

	// Prefab : prop_ddo_aks
	// Mesh : mesh_ddo_ks
	{
		DX11Material_DeferredIBL* mat = new DX11Material_DeferredIBL(ptr);
		mat->SetDiffusemap( ptr->LoadTexture( L"Media/DDO_SDK_AKS/_polySurface42_d.jpg" ) );
		//mat->SetDiffusemap( ptr->LoadTexture( L"Media/grey.bmp" ) );
		mat->SetNormalmap( ptr->LoadTexture( L"Media/DDO_SDK_AKS/_polySurface42_n.jpg" ) );
		mat->SetSpecularMap( ptr->LoadTexture( L"Media/DDO_SDK_AKS/_polySurface42_g.jpg" ) );
		mat->SetAOMap( ptr->LoadTexture( L"Media/DDO_SDK_AKS/_polySurface42_o.jpg" ) );
		//mat->SetSpecularMap( ptr->LoadTexture( L"Media/test_specular1.bmp" ) );
		//mat->SetDetailNormalmap( ptr->LoadTexture( L"Media/Tex_0010_5.dds" ) );
		//mat->SetDetailNormalStrength( 3.0f );
		//mat->SetDetailTiling( 12.0f );
		mat->SetSpecularIntensity( 1.0f );
		mat->SetGlossiness( 2048.0f );

		mat->SetIBL( IBL );

		Mesh* mesh = ptr->LoadMesh( "Media/DDO_SDK_AKS/Low.obj" );
		mesh->SetMaterial( mat );

		m_pMeshes[L"mesh_ddo_aks"] = mesh;
	}

	// Sphere for IBL testing
	// Prefab : prop_sphere
	// Mesh : mesh_sphere
	{
		DX11Material_DeferredIBL* mat = new DX11Material_DeferredIBL(ptr);
		//mat->SetDiffusemap( ptr->LoadTexture( L"Media/DDO_SDK_Helmet/albedo.jpg" ) );
		mat->SetDiffusemap( ptr->LoadTexture( L"Media/redplastic.bmp" ) );
		mat->SetNormalmap( ptr->LoadTexture( L"Media/flat.bmp" ) );
		mat->SetSpecularMap( ptr->LoadTexture( L"Media/test_specular2.bmp" ) );
		mat->SetAOMap( ptr->LoadTexture( L"Media/white.bmp" ) );
		mat->SetSpecularIntensity( 1.0f );
		mat->SetGlossiness( 2048.0f );
		mat->SetIBL( IBL );

		Mesh* mesh = ptr->LoadMesh( "Media/Meshes/sphere64.obj" );
		mesh->SetMaterial( mat );

		m_pMeshes[L"mesh_sphere"] = mesh;
	}
}

void Scene::LoadPrefabs()
{
	//////////////////
	// Kardus
	//////////////////
	hkpRigidBody* rigidBody = CreateBoxRigidBody( Vector3(0.3307f, 0.3313f, 0.3064f), 1.0f );
	Mesh* mesh = m_pMeshes[L"kardus_a"];
	Entity_Prop* prefab = new Entity_Prop( m_pRenderSystem, m_pPhysicsSystem, mesh, rigidBody );
	m_pPrefabs[L"prop_kardus_a"] = prefab;

	rigidBody = CreateBoxRigidBody( Vector3(0.3110f, 0.3796f, 0.2614f), 1.0f );
	mesh = m_pMeshes[L"kardus_b"];
	prefab = new Entity_Prop( m_pRenderSystem, m_pPhysicsSystem, mesh, rigidBody );
	m_pPrefabs[L"prop_kardus_b"] = prefab;

	rigidBody = CreateBoxRigidBody( Vector3(0.4516f, 0.3906f, 0.3237f), 1.0f );
	mesh = m_pMeshes[L"kardus_c"];
	prefab = new Entity_Prop( m_pRenderSystem, m_pPhysicsSystem, mesh, rigidBody );
	m_pPrefabs[L"prop_kardus_c"] = prefab;

	rigidBody = CreateBoxRigidBody( Vector3(0.2784f, 0.3841f, 0.2129f), 1.0f );
	mesh = m_pMeshes[L"kardus_d"];
	prefab = new Entity_Prop( m_pRenderSystem, m_pPhysicsSystem, mesh, rigidBody );
	m_pPrefabs[L"prop_kardus_d"] = prefab;

	rigidBody = CreateBoxRigidBody( Vector3(0.3469f, 0.3995f, 0.2017f), 1.0f );
	mesh = m_pMeshes[L"kardus_e"];
	prefab = new Entity_Prop( m_pRenderSystem, m_pPhysicsSystem, mesh, rigidBody );
	m_pPrefabs[L"prop_kardus_e"] = prefab;

	rigidBody = CreateBoxRigidBody( Vector3(0.4792f, 0.4000f, 0.2221f), 1.0f );
	mesh = m_pMeshes[L"kardus_f"];
	prefab = new Entity_Prop( m_pRenderSystem, m_pPhysicsSystem, mesh, rigidBody );
	m_pPrefabs[L"prop_kardus_f"] = prefab;

	rigidBody = CreateBoxRigidBody( Vector3(0.2772f, 0.4186f, 0.1188f), 1.0f );
	mesh = m_pMeshes[L"kardus_g"];
	prefab = new Entity_Prop( m_pRenderSystem, m_pPhysicsSystem, mesh, rigidBody );
	m_pPrefabs[L"prop_kardus_g"] = prefab;

	rigidBody = CreateBoxRigidBody( Vector3(0.2765f, 0.4182f, 0.1187f), 1.0f );
	mesh = m_pMeshes[L"kardus_h"];
	prefab = new Entity_Prop( m_pRenderSystem, m_pPhysicsSystem, mesh, rigidBody );
	m_pPrefabs[L"prop_kardus_h"] = prefab;

	rigidBody = CreateBoxRigidBody( Vector3(0.3509f, 0.3646f, 0.2085f), 1.0f );
	mesh = m_pMeshes[L"kardus_i"];
	prefab = new Entity_Prop( m_pRenderSystem, m_pPhysicsSystem, mesh, rigidBody );
	m_pPrefabs[L"prop_kardus_i"] = prefab;

	rigidBody = CreateBoxRigidBody( Vector3(0.3172f, 0.3165f, 0.1623f), 1.0f );
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
	transform.SetPosition(0.0f,0.0f,0.336f);
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
	transform.SetPosition(0.0f,0.0f,0.0f);
	prefab = new Entity_Prop( m_pRenderSystem, m_pPhysicsSystem, mesh, rigidBody, transform );
	m_pPrefabs[L"prop_metro_kursiroda"] = prefab;
	}
}

hkpRigidBody* Scene::CreateBoxRigidBody( Vector3 halfExtents, float mass )
{
	if (m_pPhysicsSystem)
	{
		return m_pPhysicsSystem->CreateBoxRigidBody( halfExtents, mass );
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