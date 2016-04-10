#pragma once

#include "DX11Renderer.h"
#include "DX11Material.h"

void DX11Renderer::setRenderSystem( RenderSystem*	pRenderSystem )
{
	m_pRenderSystem = pRenderSystem;
	m_pShadowmapShader = pRenderSystem->loadShaderset( L"Shaders/Shadowmap.hlsl", "VS", "PS", SM_5_0 );
	m_pRSMShader = pRenderSystem->loadShaderset( L"Shaders/RSM.hlsl", "VS", "PS", SM_5_0 );
}

// TODO : optimize implementation
void DX11Renderer::setMesh( Mesh* pMesh )
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

		for( int i=0; i<m_pMesh->getNumberOfSubmeshes(); i++ )
		{
			SubmeshRenderData renderData;
			renderData.bVisible = true;
			m_SubmeshRenderData.push_back(renderData);

			Submesh* submesh = m_pMesh->getSubmesh( i );

			for (int multipass = 0; multipass < 4; multipass++)
			{
				// Main render commands
				DX11RenderCommand_Draw* command = new DX11RenderCommand_Draw();
				command->setGeometryChunk( (D3D11GeometryChunk*)submesh->getGeometryChunk() );
				m_pSubmeshRenderCommands.push_back(std::vector<DX11RenderCommand_Draw*>());
				m_pSubmeshRenderCommands[i].push_back( command );
			}

			// Render commands for shadowmap rendering
			DX11RenderCommand_Draw* command = new DX11RenderCommand_Draw();
			command->setShaderset( (D3D11Shaderset*)m_pShadowmapShader );
			command->setGeometryChunk( (D3D11GeometryChunk*)submesh->getGeometryChunk() );
			m_pShadowmapRenderCommands.push_back( command );

			// Render commands for debug OBB drawing
			DX11RenderCommand_Draw* rcDebug = new DX11RenderCommand_Draw();
			rcDebug->setGeometryChunk( (D3D11GeometryChunk*)m_pRenderSystem->createBoxWireframeMesh( XMFLOAT3(2.0f, 2.0f, 2.0f) )->getSubmesh(0)->getGeometryChunk() );
			rcDebug->setShaderset( (D3D11Shaderset*)m_pRenderSystem->loadShaderset( L"Shaders/OneColor.hlsl", "VS", "PS", SM_5_0 ) );
			m_pDebugRenderCommands.push_back( rcDebug );
		}
	}

	m_pMesh = pMesh;
}

// TODO : optimize implementation
void DX11Renderer::render( Transform* pTransform )
{
	if (m_pMesh)
	{
		for( int i=0; i<m_pMesh->getNumberOfSubmeshes(); i++ )
		{
			if (m_SubmeshRenderData[i].bVisible)
			{
				Submesh* submesh = m_pMesh->getSubmesh( i );
	
				Timer matTimer;
				UINT passes = 1;
				if (submesh->getMaterial())
						passes = submesh->getMaterial()->bind( this, (RenderCommand**)&m_pSubmeshRenderCommands[i][0], &m_SubmeshRenderData[i], pTransform );
				// else
				// "No material" shader
	
				m_pRenderSystem->t_material += matTimer.getMiliseconds();
					
				for( int ii=0; ii<passes; ii++ )
				{
					m_pRenderSystem->submit( m_pSubmeshRenderCommands[i].at(ii) );
					m_pRenderSystem->drawcalls++;
				}
			}
		}
	}
}

void DX11Renderer::renderShadowmap( Transform* pTransform, Camera3D* pShadowCamera )
{
	if (m_pMesh)
	{
		for( int i=0; i<m_pMesh->getNumberOfSubmeshes(); i++ )
		{
			if (m_SubmeshRenderData[i].bVisible)
			{
				DX11RenderCommand_Draw* command = m_pShadowmapRenderCommands[i];

				ShaderParams params;
				params.initialize( m_pShadowmapShader );

				params.setParam( "World", 0, &pTransform->getMatrix().transpose() );
				params.setParam( "SpotLightViewProjection", 0, &pShadowCamera->getViewProjectionMatrix().transpose() );
				params.setParam( "SpotLightView", 0, &pShadowCamera->getViewMatrix().transpose() );
				params.setParam( "fFarPlane", 0, pShadowCamera->getFarPlane() );
			
				command->setShaderParams( &params );
				command->setShaderset( (D3D11Shaderset*)m_pShadowmapShader );

				m_pRenderSystem->submit( command ); 
				m_pRenderSystem->drawcalls++;
			}
		}
	}
}

void DX11Renderer::renderRSM( Transform* pTransform, Camera3D* pShadowCamera, SpotLight* pLightSource )
{
	if (m_pMesh)
	{
		for( int i=0; i<m_pMesh->getNumberOfSubmeshes(); i++ )
		{
			if (m_SubmeshRenderData[i].bVisible)
			{
				DX11RenderCommand_Draw* command = m_pShadowmapRenderCommands[i];

				ShaderParams params;
				params.initialize( m_pRSMShader );

				params.setParam( "World", 0, &pTransform->getMatrix().transpose() );
				params.setParam( "SpotLightViewProjection", 0, &pShadowCamera->getViewProjectionMatrix().transpose() );
				params.setParam( "SpotLightView", 0, &pShadowCamera->getViewMatrix().transpose() );
				params.setParam( "NormalMatrix", 0, &pTransform->getMatrix().transpose() ) ;
				params.setParam( "vResolution", 0, &Vector4(pLightSource->getCookie()->getDimensions(),0.0f,0.0f) );
				params.setParam( "fFarPlane", 0, pShadowCamera->getFarPlane() );
			
				command->setShaderParams( &params );
				command->setShaderset( (D3D11Shaderset*)m_pRSMShader );
				command->setTexture( "txCookie", (DX11Texture2D*)pLightSource->getCookie() );
				if (dynamic_cast<DX11Material_DeferredIBL*>(m_pMesh->getSubmesh(i)->getMaterial()))
				{
					command->setTexture( "txDiffuse", (DX11Texture2D*)dynamic_cast<DX11Material_DeferredIBL*>(m_pMesh->getSubmesh(i)->getMaterial())->getDiffusemap() );
					command->setTexture( "txNormal", (DX11Texture2D*)dynamic_cast<DX11Material_DeferredIBL*>(m_pMesh->getSubmesh(i)->getMaterial())->getNormalmap() );
				}
				else if (dynamic_cast<DX11Material_Deferred*>(m_pMesh->getSubmesh(i)->getMaterial()))
				{
					command->setTexture( "txDiffuse", (DX11Texture2D*)dynamic_cast<DX11Material_Deferred*>(m_pMesh->getSubmesh(i)->getMaterial())->getDiffusemap() );
					command->setTexture( "txNormal", (DX11Texture2D*)dynamic_cast<DX11Material_Deferred*>(m_pMesh->getSubmesh(i)->getMaterial())->getNormalmap() );
				}
				m_pRenderSystem->submit( command ); 
				m_pRenderSystem->drawcalls++;
			}
		}
	}
}

void DX11Renderer::renderOBB( Transform* pTransform )
{
	if (m_pMesh)
	{
		for( int i=0; i<m_pMesh->getNumberOfSubmeshes(); i++ )
		{
			if (m_SubmeshRenderData[i].bVisible)
			{
				DX11RenderCommand_Draw* command = m_pDebugRenderCommands[i];
				
				Submesh* submesh = m_pMesh->getSubmesh( i );
				XMVECTOR extents = XMLoadFloat3( &submesh->getGeometryChunk()->getAABB()->Extents );
				XMVECTOR scale = XMLoadFloat3( &pTransform->getScale().intoXMFLOAT3() );
				XMVECTOR offset = XMLoadFloat3( &submesh->getGeometryChunk()->getAABB()->Center )*scale;
				XMVECTOR position = XMVector3Rotate( offset, XMLoadFloat4(&pTransform->getOrientation().intoXMFLOAT4()) );
				scale *= extents;
				position += XMLoadFloat3( &pTransform->getPosition().intoXMFLOAT3() );
	
				XMMATRIX transform = XMMatrixAffineTransformation( scale, XMVectorZero(), XMLoadFloat4(&pTransform->getOrientation().intoXMFLOAT4()), position );

				ShaderParams params;
				params.initialize( command->getShaderset() );

				params.setParam( "ViewProjection", 0, &m_pRenderSystem->getCamera()->getViewMatrix().transpose() );
				params.setParam( "World", 0, &XMMatrixTranspose(transform) );
				params.setParam( "vMeshColor", 0, &Vector4( 0.1f, 0.1f, 0.2f, 0.0f ) );

				command->setShaderParams( &params );

				m_pRenderSystem->submit( command ); 
			}
		}
	}
}


void DX11Renderer::addLight( PointLight* pLight )
{
	if (m_pMesh)
	{
		for( int i=0; i<m_pMesh->getNumberOfSubmeshes(); i++ )
		{
			if (m_SubmeshRenderData[i].bVisible)
			{
				m_SubmeshRenderData[i].AffectingPointLights.push_back( pLight );
			}
		}
	}
}

void DX11Renderer::addLight( SpotLight* pLight )
{
	if (m_pMesh)
	{
		for( int i=0; i<m_pMesh->getNumberOfSubmeshes(); i++ )
		{
			if (m_SubmeshRenderData[i].bVisible)
			{
				m_SubmeshRenderData[i].AffectingSpotLights.push_back( pLight );
			}
		}
	}
}


void DX11Renderer::clearLights()
{
	if (m_pMesh)
	{
		for( int i=0; i<m_pMesh->getNumberOfSubmeshes(); i++ )
		{
			m_SubmeshRenderData[i].AffectingPointLights.clear();
			m_SubmeshRenderData[i].AffectingSpotLights.clear();
		}
	}
}

void DX11Renderer::cull( XNA::Frustum* frustum, Transform* pTransform )
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
				m_SubmeshRenderData[i].bVisible = true;
			else
				m_SubmeshRenderData[i].bVisible = false;
		}
	}
}

void DX11Renderer::cull( XNA::Sphere* sphere, Transform* pTransform )
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

void DX11Renderer::cullLight( PointLight* light, Transform* pTransform )
{
	if (m_pMesh)
	{
		XNA::OrientedBox objectBox;
		XNA::Sphere lightSphere;
		for( int i=0; i<m_pMesh->getNumberOfSubmeshes(); i++ )
		{
			if (m_SubmeshRenderData[i].bVisible)
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
					m_SubmeshRenderData[i].AffectingPointLights.push_back( light );
			}
		}
	}
}

void DX11Renderer::cullLight( SpotLight* light, Transform* pTransform )
{
	if (m_pMesh)
	{
		XNA::OrientedBox objectBox;
		XNA::Frustum lightFrustum = light->getFrustum();
		for( int i=0; i<m_pMesh->getNumberOfSubmeshes(); i++ )
		{
			if (m_SubmeshRenderData[i].bVisible)
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
					m_SubmeshRenderData[i].AffectingSpotLights.push_back( light );
			}
		}
	}
}

void DX11Renderer::setCulled( bool culled )
{
	for(int i=0; i<m_pMesh->getNumberOfSubmeshes(); i++)
	{
		m_SubmeshRenderData[i].bVisible = !culled;
	}
}