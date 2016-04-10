#pragma once

#include "Renderer.h"

void Renderer::setRenderSystem( RenderSystem*	pRenderSystem )
{
	m_pRenderSystem = pRenderSystem;
	m_pShadowmapShader = pRenderSystem->loadShaderset( L"Shaders/Shadowmap.hlsl", "VS", "PS", SM_5_0 );
}

// TODO : optimize implementation
void Renderer::setMesh( Mesh* pMesh )
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

		for( int i=0; i<m_pMesh->getNumberOfSubmeshes(); i++ )
		{
			m_bVisible.push_back( true );
			Submesh* submesh = m_pMesh->getSubmesh( i );

			m_PointLights.push_back( std::vector<PointLight*>() );
			m_SpotLights.push_back( std::vector<SpotLight*>() );

			// Main rendering
			for (int multipass = 0; multipass < 4; multipass++)
			{
				D3D11RenderCommand_Draw* RC = new D3D11RenderCommand_Draw();
				//submesh->getMaterial()->bind( this, RC );
				RC->setGeometryChunk( (D3D11GeometryChunk*)submesh->getGeometryChunk() );
				m_pSubmeshRenderCommands.push_back(std::vector<D3D11RenderCommand_Draw*>());
				m_pSubmeshRenderCommands[i].push_back( RC );
			}
			/*
			for (int ii=0; ii<10; ii++)
			{
				D3D11RenderCommand_Draw* RC = new D3D11RenderCommand_Draw();
				//submesh->getMaterial()->bind( this, RC );
				RC->setGeometryChunk( (D3D11GeometryChunk*)submesh->getGeometryChunk() );
				m_pSubmeshRenderCommands[i].push_back( RC );
			}*/

			// Shadowmap rendering
			D3D11RenderCommand_Draw* RC = new D3D11RenderCommand_Draw();
			RC->setShaderset( (D3D11Shaderset*)m_pShadowmapShader );
			RC->setGeometryChunk( (D3D11GeometryChunk*)submesh->getGeometryChunk() );
			m_pShadowmapRenderCommands.push_back( RC );

			// Debug OBB drawing
			D3D11RenderCommand_Draw* rcDebug = new D3D11RenderCommand_Draw();
			rcDebug->setGeometryChunk( (D3D11GeometryChunk*)m_pRenderSystem->createBoxWireframeMesh( XMFLOAT3(2.0f, 2.0f, 2.0f) )->getSubmesh(0)->getGeometryChunk() );
			rcDebug->setShaderset( (D3D11Shaderset*)m_pRenderSystem->loadShaderset( L"Shaders/OneColor.hlsl", "VS", "PS", SM_5_0 ) );
			m_pDebugRenderCommands.push_back( rcDebug );
		}
	
	}

	m_pMesh = pMesh;
}


void Renderer::addLight( PointLight* pLight )
{
	for( int i=0; i<m_pMesh->getNumberOfSubmeshes(); i++ )
	{
		if (m_bVisible[i])
		{
			m_PointLights[i].push_back( pLight );
		}
	}
}

void Renderer::addLight( SpotLight* pLight )
{
	for( int i=0; i<m_pMesh->getNumberOfSubmeshes(); i++ )
	{
		if (m_bVisible[i])
		{
			m_SpotLights[i].push_back( pLight );
		}
	}
}


void Renderer::clearLights()
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
void Renderer::render( Transform* pTransform )
{
	if (m_pMesh)
	{
		for( int i=0; i<m_pMesh->getNumberOfSubmeshes(); i++ )
		{
			if (m_bVisible[i])
			{
				Submesh* submesh = m_pMesh->getSubmesh( i );

				Timer matTimer;
				// BOTTLENECK!!
				UINT passes = submesh->getMaterial()->bind( this, &m_pSubmeshRenderCommands[i], i,pTransform );
				m_pRenderSystem->t_material += matTimer.getMiliseconds();

				for( int ii=0; ii<passes; ii++ )
				{
					m_pRenderSystem->submitThreaded( m_pSubmeshRenderCommands[i].at(ii) );
					m_pRenderSystem->drawcalls++;
				}
			}
		}
	}
}

void Renderer::renderShadowmap( Transform* pTransform, Camera3D* pShadowCamera )
{
	if (m_pMesh)
	{
		for( int i=0; i<m_pMesh->getNumberOfSubmeshes(); i++ )
		{
			if (m_bVisible[i])
			{
				D3D11RenderCommand_Draw* rc = m_pShadowmapRenderCommands[i];

				rc->shaderParams()->assign( "World", 0, &XMMatrixTranspose(pTransform->getXMMatrix()) );
				rc->shaderParams()->assign( "SpotLightViewProjection", 0, &pShadowCamera->getViewProjectionMatrix().transpose().intoXMFLOAT4X4() );
			
				m_pRenderSystem->submitThreaded( rc ); 
				m_pRenderSystem->drawcalls++;
			}
		}
	}
}

void Renderer::renderOBB( Transform* pTransform )
{
	if (m_pMesh)
	{
	for( int i=0; i<m_pMesh->getNumberOfSubmeshes(); i++ )
	{
		if ( m_bVisible[i] )
		{
			D3D11RenderCommand_Draw* rc = m_pDebugRenderCommands[i];
			
			Submesh* submesh = m_pMesh->getSubmesh( i );
			XMVECTOR extents = XMLoadFloat3( &submesh->getGeometryChunk()->getAABB()->Extents );
			XMVECTOR scale = XMLoadFloat3( &pTransform->getScale().intoXMFLOAT3() );
			XMVECTOR offset = XMLoadFloat3( &submesh->getGeometryChunk()->getAABB()->Center )*scale;
			XMVECTOR position = XMVector3Rotate( offset, XMLoadFloat4(&pTransform->getOrientation().intoXMFLOAT4()) );
			scale *= extents;
			position += XMLoadFloat3( &pTransform->getPosition().intoXMFLOAT3() );

			XMMATRIX transform = XMMatrixAffineTransformation( scale, XMVectorZero(), XMLoadFloat4(&pTransform->getOrientation().intoXMFLOAT4()), position );

			rc->shaderParams()->assign( "ViewProjection", 0, &m_pRenderSystem->getCamera()->getViewProjectionMatrix().transpose().intoXMFLOAT4X4() );
			rc->shaderParams()->assign( "World", 0, &XMMatrixTranspose(transform) );
			rc->shaderParams()->assign( "vMeshColor", 0, &XMFLOAT4( 0.1f, 0.1f, 0.2f, 0.0f ) );

			//D3D11RenderCommand_Draw* rcsubmitted = new D3D11RenderCommand_Draw( **it );*/
			m_pRenderSystem->submitThreaded( rc ); 
		}
	}
	}
}

void Renderer::cull( XNA::Frustum* frustum, Transform* pTransform )
{
	if (m_pMesh)
	{
		XNA::OrientedBox objectBox;
		for( int i=0; i<m_pMesh->getNumberOfSubmeshes(); i++ )
		{
			Submesh* submesh = m_pMesh->getSubmesh( i );

			XMVECTOR extents = XMLoadFloat3( &submesh->getGeometryChunk()->getAABB()->Extents );
			XMVECTOR scale = XMLoadFloat3( &pTransform->getScale().intoXMFLOAT3() );
			XMVECTOR offset = XMLoadFloat3( &submesh->getGeometryChunk()->getAABB()->Center )*scale;
			XMVECTOR position = XMVector3Rotate( offset, XMLoadFloat4(&pTransform->getOrientation().intoXMFLOAT4()) );
			position += XMLoadFloat3( &pTransform->getPosition().intoXMFLOAT3() );
			XMStoreFloat3( &objectBox.Center, position );
			XMStoreFloat3( &objectBox.Extents, extents*scale );
			objectBox.Orientation = pTransform->getOrientation().intoXMFLOAT4();

			if( XNA::IntersectOrientedBoxFrustum( &objectBox, frustum ) > 0 )
			m_bVisible[i] = true;
			else
			m_bVisible[i] = false;
		}
	}
}

void Renderer::cull( XNA::Sphere* sphere, Transform* pTransform )
{/*
	XNA::OrientedBox objectBox;
	for (std::vector< std::pair<D3D11RenderCommand_Draw*,bool > >::iterator it = m_pRenderCommands.begin(); it != m_pRenderCommands.end(); ++it)
	{
		objectBox.Center = pTransform->getPosition();
		objectBox.Orientation = pTransform->getRotation();

		objectBox.Extents = XMFLOAT3(	it->first->getGeometryChunk()->getAABB()->Extents.x*pTransform->getScale().x,
										it->first->getGeometryChunk()->getAABB()->Extents.y*pTransform->getScale().y,
										it->first->getGeometryChunk()->getAABB()->Extents.z*pTransform->getScale().z	);

		if( XNA::IntersectSphereOrientedBox( sphere, &objectBox ) > 0 )
		it->second = true;
		else
		it->second = false;
	}*/
}

void Renderer::cullLight( PointLight* light, Transform* pTransform )
{
	if (m_pMesh)
	{
	XNA::OrientedBox objectBox;
	XNA::Sphere lightSphere;
	for( int i=0; i<m_pMesh->getNumberOfSubmeshes(); i++ )
	{
		if (m_bVisible[i])
		{
			Submesh* submesh = m_pMesh->getSubmesh( i );

			XMVECTOR extents = XMLoadFloat3( &submesh->getGeometryChunk()->getAABB()->Extents );
			XMVECTOR scale = XMLoadFloat3( &pTransform->getScale().intoXMFLOAT3() );
			XMVECTOR offset = XMLoadFloat3( &submesh->getGeometryChunk()->getAABB()->Center )*scale;
			XMVECTOR position = XMVector3Rotate( offset, XMLoadFloat4(&pTransform->getOrientation().intoXMFLOAT4()) );
			position += XMLoadFloat3( &pTransform->getPosition().intoXMFLOAT3() );
			XMStoreFloat3( &objectBox.Center, position );
			XMStoreFloat3( &objectBox.Extents, extents*scale );
			objectBox.Orientation = pTransform->getOrientation().intoXMFLOAT4();

			lightSphere.Center = light->transform()->getPosition().intoXMFLOAT3();
			lightSphere.Radius = light->getRadius();

			if( XNA::IntersectSphereOrientedBox( &lightSphere, &objectBox ) )
			m_PointLights[i].push_back( light );
		}
	}
	}
}

void Renderer::cullLight( SpotLight* light, Transform* pTransform )
{
	if (m_pMesh)
	{
		XNA::OrientedBox objectBox;
		XNA::Frustum lightFrustum = light->getFrustum();
		for( int i=0; i<m_pMesh->getNumberOfSubmeshes(); i++ )
		{
			if (m_bVisible[i])
			{
				Submesh* submesh = m_pMesh->getSubmesh( i );

				XMVECTOR extents = XMLoadFloat3( &submesh->getGeometryChunk()->getAABB()->Extents );
				XMVECTOR scale = XMLoadFloat3( &pTransform->getScale().intoXMFLOAT3() );
				XMVECTOR offset = XMLoadFloat3( &submesh->getGeometryChunk()->getAABB()->Center )*scale;
				XMVECTOR position = XMVector3Rotate( offset, XMLoadFloat4(&pTransform->getOrientation().intoXMFLOAT4()) );
				position += XMLoadFloat3( &pTransform->getPosition().intoXMFLOAT3() );
				XMStoreFloat3( &objectBox.Center, position );
				XMStoreFloat3( &objectBox.Extents, extents*scale );
				objectBox.Orientation = pTransform->getOrientation().intoXMFLOAT4();

				if( XNA::IntersectOrientedBoxFrustum( &objectBox, &lightFrustum ) > 0 )
				m_SpotLights[i].push_back( light );
			}
		}
	}
}

void Renderer::setCulled( bool culled )
{
	for(int i=0; i<m_pMesh->getNumberOfSubmeshes(); i++)
	{
		m_bVisible[i] = !culled;
	}
}