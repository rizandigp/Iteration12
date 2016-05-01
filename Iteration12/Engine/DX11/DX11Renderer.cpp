#pragma once

#include "DX11Renderer.h"
#include "DX11Material.h"
#include "DX11Shaderset.h"

void DX11Renderer::SetRenderSystem( RenderSystem*	pRenderSystem )
{
	m_pRenderSystem = pRenderSystem;
	m_pShadowmapShader = pRenderSystem->LoadShaderset( L"Shaders/Shadowmap.hlsl", "VS", "PS", SM_5_0 );
	m_pRSMShader = pRenderSystem->LoadShaderset( L"Shaders/RSM.hlsl", "VS", "PS", SM_5_0 );
}

// TODO : optimize implementation
void DX11Renderer::SetMesh( Mesh* pMesh )
{
	if (m_pMesh!=pMesh && pMesh)
	{
		m_pMesh = pMesh;
		
		// Delete all debug render commands
		while (!m_pDebugRenderCommands.empty())
		{
			delete m_pDebugRenderCommands.back();  m_pDebugRenderCommands.pop_back();
		}

		// Reset render data
		m_SubmeshRenderData.clear();

		for( int i=0; i<m_pMesh->GetNumberOfSubmeshes(); i++ )
		{
			SubmeshRenderData renderData;
			renderData.bVisible = true;
			m_SubmeshRenderData.push_back(renderData);

			Submesh* submesh = m_pMesh->GetSubmesh( i );

			for (int multipass = 0; multipass < 4; multipass++)
			{
				// Main render commands
				DX11RenderCommand_Draw* command = new DX11RenderCommand_Draw();
				command->SetGeometryChunk( (DX11GeometryChunk*)submesh->GetGeometryChunk() );
				m_pSubmeshRenderCommands.push_back(std::vector<DX11RenderCommand_Draw*>());
				m_pSubmeshRenderCommands[i].push_back( command );
			}

			// Render commands for shadowmap rendering
			DX11RenderCommand_Draw* command = new DX11RenderCommand_Draw();
			command->SetShaderset( (DX11Shaderset*)m_pShadowmapShader );
			command->SetGeometryChunk( (DX11GeometryChunk*)submesh->GetGeometryChunk() );
			m_pShadowmapRenderCommands.push_back( command );

			// Render commands for debug OBB drawing
			DX11RenderCommand_Draw* rcDebug = new DX11RenderCommand_Draw();
			rcDebug->SetGeometryChunk( (DX11GeometryChunk*)m_pRenderSystem->CreateBoxWireframeMesh( XMFLOAT3(2.0f, 2.0f, 2.0f) )->GetSubmesh(0)->GetGeometryChunk() );
			rcDebug->SetShaderset( (DX11Shaderset*)m_pRenderSystem->LoadShaderset( L"Shaders/OneColor.hlsl", "VS", "PS", SM_5_0 ) );
			m_pDebugRenderCommands.push_back( rcDebug );
		}
	}

	m_pMesh = pMesh;
}

// TODO : optimize implementation
void DX11Renderer::Render( Transform* pTransform )
{
	if (m_pMesh)
	{
		for( int i=0; i<m_pMesh->GetNumberOfSubmeshes(); i++ )
		{
			if (m_SubmeshRenderData[i].bVisible)
			{
				Submesh* submesh = m_pMesh->GetSubmesh( i );
	
				Timer matTimer;
				UINT passes = 1;
				if (submesh->GetMaterial())
						passes = submesh->GetMaterial()->Bind( this, (RenderCommand**)&m_pSubmeshRenderCommands[i][0], &m_SubmeshRenderData[i], pTransform );
				// else
				// "No material" shader
	
				m_pRenderSystem->t_material += matTimer.GetMiliseconds();
					
				for( int ii=0; ii<passes; ii++ )
				{
					m_pRenderSystem->Submit( m_pSubmeshRenderCommands[i].at(ii) );
					m_pRenderSystem->drawcalls++;
				}
			}
		}
	}
}

void DX11Renderer::RenderShadowmap( Transform* pTransform, Camera3D* pShadowCamera )
{
	if (m_pMesh)
	{
		for( int i=0; i<m_pMesh->GetNumberOfSubmeshes(); i++ )
		{
			if (m_SubmeshRenderData[i].bVisible)
			{
				DX11RenderCommand_Draw* command = m_pShadowmapRenderCommands[i];

				ShaderParams params;
				params.initialize( m_pShadowmapShader );

				params.setParam( "World", 0, &pTransform->GetMatrix().transpose() );
				params.setParam( "SpotLightViewProjection", 0, &pShadowCamera->GetViewProjectionMatrix().transpose() );
				params.setParam( "SpotLightView", 0, &pShadowCamera->GetViewMatrix().transpose() );
				params.setParam( "fFarPlane", 0, pShadowCamera->GetFarPlane() );
			
				command->SetShaderParams( &params );
				command->SetShaderset( (DX11Shaderset*)m_pShadowmapShader );

				RenderState renderState;
				renderState.CullingMode = CULL_FRONT;
				command->SetRenderState(renderState);

				BlendState blendState;
				command->SetBlendState(blendState);

				m_pRenderSystem->Submit( command ); 
				m_pRenderSystem->drawcalls++;
			}
		}
	}
}

void DX11Renderer::RenderRSM( Transform* pTransform, Camera3D* pShadowCamera, SpotLight* pLightSource )
{
	if (m_pMesh)
	{
		for( int i=0; i<m_pMesh->GetNumberOfSubmeshes(); i++ )
		{
			if (m_SubmeshRenderData[i].bVisible)
			{
				DX11RenderCommand_Draw* command = m_pShadowmapRenderCommands[i];

				ShaderParams params;
				params.initialize( m_pRSMShader );

				params.setParam( "World", 0, &pTransform->GetMatrix().transpose() );
				params.setParam( "SpotLightViewProjection", 0, &pShadowCamera->GetViewProjectionMatrix().transpose() );
				params.setParam( "SpotLightView", 0, &pShadowCamera->GetViewMatrix().transpose() );
				params.setParam( "NormalMatrix", 0, &pTransform->GetMatrix().transpose() ) ;
				params.setParam( "vResolution", 0, &Vector4(pLightSource->GetCookie()->GetDimensions(),0.0f,0.0f) );
				params.setParam( "fFarPlane", 0, pShadowCamera->GetFarPlane() );
			
				command->SetShaderParams( &params );
				command->SetShaderset( (DX11Shaderset*)m_pRSMShader );
				command->SetTexture( "txCookie", pLightSource->GetCookie() );
				if (dynamic_cast<DX11Material_DeferredIBL*>(m_pMesh->GetSubmesh(i)->GetMaterial()))
				{
					command->SetTexture( "txDiffuse", dynamic_cast<DX11Material_DeferredIBL*>(m_pMesh->GetSubmesh(i)->GetMaterial())->GetDiffusemap() );
					command->SetTexture( "txNormal", dynamic_cast<DX11Material_DeferredIBL*>(m_pMesh->GetSubmesh(i)->GetMaterial())->GetNormalmap() );
				}
				else if (dynamic_cast<DX11Material_Deferred*>(m_pMesh->GetSubmesh(i)->GetMaterial()))
				{
					command->SetTexture( "txDiffuse", dynamic_cast<DX11Material_Deferred*>(m_pMesh->GetSubmesh(i)->GetMaterial())->GetDiffusemap() );
					command->SetTexture( "txNormal", dynamic_cast<DX11Material_Deferred*>(m_pMesh->GetSubmesh(i)->GetMaterial())->GetNormalmap() );
				}
				m_pRenderSystem->Submit( command ); 
				m_pRenderSystem->drawcalls++;
			}
		}
	}
}

void DX11Renderer::RenderOBB( Transform* pTransform )
{
	if (m_pMesh)
	{
		for( int i=0; i<m_pMesh->GetNumberOfSubmeshes(); i++ )
		{
			if (m_SubmeshRenderData[i].bVisible)
			{
				DX11RenderCommand_Draw* command = m_pDebugRenderCommands[i];
				
				Submesh* submesh = m_pMesh->GetSubmesh( i );
				XMVECTOR extents = XMLoadFloat3( &submesh->GetGeometryChunk()->GetAABB()->Extents );
				XMVECTOR scale = XMLoadFloat3( &pTransform->GetScale().intoXMFLOAT3() );
				XMVECTOR offset = XMLoadFloat3( &submesh->GetGeometryChunk()->GetAABB()->Center )*scale;
				XMVECTOR position = XMVector3Rotate( offset, XMLoadFloat4(&pTransform->GetOrientation().intoXMFLOAT4()) );
				scale *= extents;
				position += XMLoadFloat3( &pTransform->GetPosition().intoXMFLOAT3() );
	
				XMMATRIX transform = XMMatrixAffineTransformation( scale, XMVectorZero(), XMLoadFloat4(&pTransform->GetOrientation().intoXMFLOAT4()), position );

				ShaderParams params;
				params.initialize( command->GetShaderset() );

				params.setParam( "ViewProjection", 0, &m_pRenderSystem->GetCamera()->GetViewMatrix().transpose() );
				params.setParam( "World", 0, &XMMatrixTranspose(transform) );
				params.setParam( "vMeshColor", 0, &Vector4( 0.1f, 0.1f, 0.2f, 0.0f ) );

				command->SetShaderParams( &params );

				m_pRenderSystem->Submit( command ); 
			}
		}
	}
}


void DX11Renderer::AddLight( PointLight* pLight )
{
	if (m_pMesh)
	{
		for( int i=0; i<m_pMesh->GetNumberOfSubmeshes(); i++ )
		{
			if (m_SubmeshRenderData[i].bVisible)
			{
				m_SubmeshRenderData[i].AffectingPointLights.push_back( pLight );
			}
		}
	}
}

void DX11Renderer::AddLight( SpotLight* pLight )
{
	if (m_pMesh)
	{
		for( int i=0; i<m_pMesh->GetNumberOfSubmeshes(); i++ )
		{
			if (m_SubmeshRenderData[i].bVisible)
			{
				m_SubmeshRenderData[i].AffectingSpotLights.push_back( pLight );
			}
		}
	}
}


void DX11Renderer::ClearLights()
{
	if (m_pMesh)
	{
		for( int i=0; i<m_pMesh->GetNumberOfSubmeshes(); i++ )
		{
			m_SubmeshRenderData[i].AffectingPointLights.clear();
			m_SubmeshRenderData[i].AffectingSpotLights.clear();
		}
	}
}

void DX11Renderer::Cull( XNA::Frustum* frustum, Transform* pTransform )
{
	if (m_pMesh)
	{
		XNA::OrientedBox objectBox;
		for( int i=0; i<m_pMesh->GetNumberOfSubmeshes(); i++ )
		{
			Submesh* submesh = m_pMesh->GetSubmesh( i );

			XMVECTOR extents = XMLoadFloat3( &submesh->GetGeometryChunk()->GetAABB()->Extents );
			XMVECTOR scale = XMLoadFloat3( &pTransform->GetScale().intoXMFLOAT3() );
			XMVECTOR offset = XMLoadFloat3( &submesh->GetGeometryChunk()->GetAABB()->Center )*scale;
			XMVECTOR position = XMVector3Rotate( offset, XMLoadFloat4(&pTransform->GetOrientation().intoXMFLOAT4()) );
			position += XMLoadFloat3( &pTransform->GetPosition().intoXMFLOAT3() );
			XMStoreFloat3( &objectBox.Center, position );
			XMStoreFloat3( &objectBox.Extents, extents*scale );
			objectBox.Orientation = pTransform->GetOrientation().intoXMFLOAT4();
	
			if( XNA::IntersectOrientedBoxFrustum( &objectBox, frustum ) > 0 )
				m_SubmeshRenderData[i].bVisible = true;
			else
				m_SubmeshRenderData[i].bVisible = false;
		}
	}
}

void DX11Renderer::Cull( XNA::Sphere* sphere, Transform* pTransform )
{/*
	XNA::OrientedBox objectBox;
	for (std::vector< std::pair<D3D11RenderCommand_Draw*,bool > >::iterator it = m_pRenderCommands.begin(); it != m_pRenderCommands.end(); ++it)
	{
		objectBox.Center = pTransform->GetPosition();
		objectBox.Orientation = pTransform->getRotation();

		objectBox.Extents = XMFLOAT3(	it->first->GetGeometryChunk()->getAABB()->Extents.x*pTransform->GetScale().x,
										it->first->GetGeometryChunk()->getAABB()->Extents.y*pTransform->GetScale().y,
										it->first->GetGeometryChunk()->getAABB()->Extents.z*pTransform->GetScale().z	);

		if( XNA::IntersectSphereOrientedBox( sphere, &objectBox ) > 0 )
		it->second = true;
		else
		it->second = false;
	}*/
}

void DX11Renderer::CullLight( PointLight* light, Transform* pTransform )
{
	if (m_pMesh)
	{
		XNA::OrientedBox objectBox;
		XNA::Sphere lightSphere;
		for( int i=0; i<m_pMesh->GetNumberOfSubmeshes(); i++ )
		{
			if (m_SubmeshRenderData[i].bVisible)
			{
				Submesh* submesh = m_pMesh->GetSubmesh( i );

				XMVECTOR extents = XMLoadFloat3( &submesh->GetGeometryChunk()->GetAABB()->Extents );
				XMVECTOR scale = XMLoadFloat3( &pTransform->GetScale().intoXMFLOAT3() );
				XMVECTOR offset = XMLoadFloat3( &submesh->GetGeometryChunk()->GetAABB()->Center )*scale;
				XMVECTOR position = XMVector3Rotate( offset, XMLoadFloat4(&pTransform->GetOrientation().intoXMFLOAT4()) );
				position += XMLoadFloat3( &pTransform->GetPosition().intoXMFLOAT3() );
				XMStoreFloat3( &objectBox.Center, position );
				XMStoreFloat3( &objectBox.Extents, extents*scale );
				objectBox.Orientation = pTransform->GetOrientation().intoXMFLOAT4();

				lightSphere.Center = light->Transformation()->GetPosition().intoXMFLOAT3();
				lightSphere.Radius = light->GetRadius();

				if( XNA::IntersectSphereOrientedBox( &lightSphere, &objectBox ) )
					m_SubmeshRenderData[i].AffectingPointLights.push_back( light );
			}
		}
	}
}

void DX11Renderer::CullLight( SpotLight* light, Transform* pTransform )
{
	if (m_pMesh)
	{
		XNA::OrientedBox objectBox;
		XNA::Frustum lightFrustum = light->GetFrustum();
		for( int i=0; i<m_pMesh->GetNumberOfSubmeshes(); i++ )
		{
			if (m_SubmeshRenderData[i].bVisible)
			{
				Submesh* submesh = m_pMesh->GetSubmesh( i );

				XMVECTOR extents = XMLoadFloat3( &submesh->GetGeometryChunk()->GetAABB()->Extents );
				XMVECTOR scale = XMLoadFloat3( &pTransform->GetScale().intoXMFLOAT3() );
				XMVECTOR offset = XMLoadFloat3( &submesh->GetGeometryChunk()->GetAABB()->Center )*scale;
				XMVECTOR position = XMVector3Rotate( offset, XMLoadFloat4(&pTransform->GetOrientation().intoXMFLOAT4()) );
				position += XMLoadFloat3( &pTransform->GetPosition().intoXMFLOAT3() );
				XMStoreFloat3( &objectBox.Center, position );
				XMStoreFloat3( &objectBox.Extents, extents*scale );
				objectBox.Orientation = pTransform->GetOrientation().intoXMFLOAT4();

				if( XNA::IntersectOrientedBoxFrustum( &objectBox, &lightFrustum ) > 0 )
					m_SubmeshRenderData[i].AffectingSpotLights.push_back( light );
			}
		}
	}
}

void DX11Renderer::SetCulled( bool Culled )
{
	for(int i=0; i<m_pMesh->GetNumberOfSubmeshes(); i++)
	{
		m_SubmeshRenderData[i].bVisible = !Culled;
	}
}