#pragma once

#include "Renderer.h"

void Renderer::SetRenderSystem( RenderSystem*	pRenderSystem )
{
	m_pRenderSystem = pRenderSystem;
	m_pShadowmapShader = pRenderSystem->LoadShaderset( L"Shaders/Shadowmap.hlsl", "VS", "PS", SM_5_0 );
}

// TODO : optimize implementation
void Renderer::SetMesh( Mesh* pMesh )
{
	if (m_pMesh!=pMesh && pMesh)
	{
		m_pMesh = pMesh;


		while (!m_pDebugRenderCommands.empty())
		{
			delete m_pDebugRenderCommands.back();  m_pDebugRenderCommands.pop_back();
		}

		while (!m_PointLights.empty())
		{
			m_PointLights.back().clear();  m_PointLights.pop_back();
		}

		while (!m_SpotLights.empty())
		{
			m_SpotLights.back().clear();  m_PointLights.pop_back();
		}

		m_bVisible.clear();

		for( int i=0; i<m_pMesh->GetNumberOfSubmeshes(); i++ )
		{
			m_bVisible.push_back( true );
			Submesh* submesh = m_pMesh->GetSubmesh( i );

			m_PointLights.push_back( std::vector<PointLight*>() );
			m_SpotLights.push_back( std::vector<SpotLight*>() );

			// Main rendering
			for (int multipass = 0; multipass < 4; multipass++)
			{
				D3D11RenderCommand_Draw* RC = new D3D11RenderCommand_Draw();
				RC->SetGeometryChunk( (D3D11GeometryChunk*)submesh->GetGeometryChunk() );
				m_pSubmeshRenderCommands.push_back(std::vector<D3D11RenderCommand_Draw*>());
				m_pSubmeshRenderCommands[i].push_back( RC );
			}

			// Shadowmap rendering
			D3D11RenderCommand_Draw* RC = new D3D11RenderCommand_Draw();
			RC->SetShaderset( (D3D11Shaderset*)m_pShadowmapShader );
			RC->SetGeometryChunk( (D3D11GeometryChunk*)submesh->GetGeometryChunk() );
			m_pShadowmapRenderCommands.push_back( RC );

			// Debug OBB drawing
			D3D11RenderCommand_Draw* rcDebug = new D3D11RenderCommand_Draw();
			rcDebug->SetGeometryChunk( (D3D11GeometryChunk*)m_pRenderSystem->CreateBoxWireframeMesh( XMFLOAT3(2.0f, 2.0f, 2.0f) )->GetSubmesh(0)->GetGeometryChunk() );
			rcDebug->SetShaderset( (D3D11Shaderset*)m_pRenderSystem->LoadShaderset( L"Shaders/OneColor.hlsl", "VS", "PS", SM_5_0 ) );
			m_pDebugRenderCommands.push_back( rcDebug );
		}
	
	}

	m_pMesh = pMesh;
}


void Renderer::AddLight( PointLight* pLight )
{
	for( int i=0; i<m_pMesh->GetNumberOfSubmeshes(); i++ )
	{
		if (m_bVisible[i])
		{
			m_PointLights[i].push_back( pLight );
		}
	}
}

void Renderer::AddLight( SpotLight* pLight )
{
	for( int i=0; i<m_pMesh->GetNumberOfSubmeshes(); i++ )
	{
		if (m_bVisible[i])
		{
			m_SpotLights[i].push_back( pLight );
		}
	}
}


void Renderer::ClearLights()
{
	for (std::vector<std::vector<PointLight*>>::iterator it = m_PointLights.begin(); it != m_PointLights.end(); ++it)
	{
		it->clear();
	}

	for (std::vector<std::vector<SpotLight*>>::iterator it = m_SpotLights.begin(); it != m_SpotLights.end(); ++it)
	{
		it->clear();
	}
}

// TODO : optimize implementation
void Renderer::Render( Transform* pTransform )
{
	if (m_pMesh)
	{
		for( int i=0; i<m_pMesh->GetNumberOfSubmeshes(); i++ )
		{
			if (m_bVisible[i])
			{
				Submesh* submesh = m_pMesh->GetSubmesh( i );

				Timer matTimer;
				// BOTTLENECK!!
				UINT passes = submesh->GetMaterial()->Bind( this, &m_pSubmeshRenderCommands[i], i,pTransform );
				m_pRenderSystem->t_material += matTimer.GetMiliseconds();

				for( int ii=0; ii<passes; ii++ )
				{
					m_pRenderSystem->SubmitThreaded( m_pSubmeshRenderCommands[i].at(ii) );
					m_pRenderSystem->drawcalls++;
				}
			}
		}
	}
}

void Renderer::RenderShadowmap( Transform* pTransform, Camera3D* pShadowCamera )
{
	if (m_pMesh)
	{
		for( int i=0; i<m_pMesh->GetNumberOfSubmeshes(); i++ )
		{
			if (m_bVisible[i])
			{
				D3D11RenderCommand_Draw* rc = m_pShadowmapRenderCommands[i];

				rc->shaderParams()->assign( "World", 0, &pTransform->GetMatrix().transpose() );
				rc->shaderParams()->assign( "SpotLightViewProjection", 0, &pShadowCamera->GetViewProjectionMatrix().transpose() );
			
				m_pRenderSystem->SubmitThreaded( rc ); 
				m_pRenderSystem->drawcalls++;
			}
		}
	}
}

void Renderer::RenderOBB( Transform* pTransform )
{
	if (m_pMesh)
	{
	for( int i=0; i<m_pMesh->GetNumberOfSubmeshes(); i++ )
	{
		if ( m_bVisible[i] )
		{
			D3D11RenderCommand_Draw* rc = m_pDebugRenderCommands[i];
			
			Submesh* submesh = m_pMesh->GetSubmesh( i );
			XMVECTOR extents = XMLoadFloat3( &submesh->GetGeometryChunk()->GetAABB()->Extents );
			XMVECTOR scale = XMLoadFloat3( &pTransform->GetScale().intoXMFLOAT3() );
			XMVECTOR offset = XMLoadFloat3( &submesh->GetGeometryChunk()->GetAABB()->Center )*scale;
			XMVECTOR position = XMVector3Rotate( offset, XMLoadFloat4(&pTransform->GetOrientation().intoXMFLOAT4()) );
			scale *= extents;
			position += XMLoadFloat3( &pTransform->GetPosition().intoXMFLOAT3() );

			XMMATRIX transform = XMMatrixAffineTransformation( scale, XMVectorZero(), XMLoadFloat4(&pTransform->GetOrientation().intoXMFLOAT4()), position );

			rc->shaderParams()->assign( "ViewProjection", 0, &m_pRenderSystem->GetCamera()->GetViewProjectionMatrix().transpose() );
			rc->shaderParams()->assign( "World", 0, &XMMatrixTranspose(transform) );
			rc->shaderParams()->assign( "vMeshColor", 0, &Vector4( 0.1f, 0.1f, 0.2f, 0.0f ) );

			m_pRenderSystem->SubmitThreaded( rc ); 
		}
	}
	}
}

void Renderer::Cull( XNA::Frustum* frustum, Transform* pTransform )
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
			m_bVisible[i] = true;
			else
			m_bVisible[i] = false;
		}
	}
}

void Renderer::Cull( XNA::Sphere* sphere, Transform* pTransform )
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

void Renderer::CullLight( PointLight* light, Transform* pTransform )
{
	if (m_pMesh)
	{
	XNA::OrientedBox objectBox;
	XNA::Sphere lightSphere;
	for( int i=0; i<m_pMesh->GetNumberOfSubmeshes(); i++ )
	{
		if (m_bVisible[i])
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
			m_PointLights[i].push_back( light );
		}
	}
	}
}

void Renderer::CullLight( SpotLight* light, Transform* pTransform )
{
	if (m_pMesh)
	{
		XNA::OrientedBox objectBox;
		XNA::Frustum lightFrustum = light->GetFrustum();
		for( int i=0; i<m_pMesh->GetNumberOfSubmeshes(); i++ )
		{
			if (m_bVisible[i])
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
				m_SpotLights[i].push_back( light );
			}
		}
	}
}

void Renderer::SetCulled( bool Culled )
{
	for(int i=0; i<m_pMesh->GetNumberOfSubmeshes(); i++)
	{
		m_bVisible[i] = !Culled;
	}
}