#pragma once

#include "Renderer.h"
#include "DX11\DX11RenderCommand.h"

void Renderer::SetRenderSystem( RenderSystem*	renderSystem )
{
	m_RenderSystem = renderSystem;
}

void Renderer::SetMesh( Mesh* mesh )
{
		m_Mesh = mesh;
}

void Renderer::AddLight( PointLight* pLight )
{
	if (m_Mesh)
	{
		for( int i=0; i<m_Mesh->GetNumberOfSubmeshes(); i++ )
		{
			if (m_SubmeshRenderData[i].bVisible)
			{
				m_SubmeshRenderData[i].AffectingPointLights.push_back( pLight );
			}
		}
	}
}

void Renderer::AddLight( SpotLight* pLight )
{
	if (m_Mesh)
	{
		for( int i=0; i<m_Mesh->GetNumberOfSubmeshes(); i++ )
		{
			if (m_SubmeshRenderData[i].bVisible)
			{
				m_SubmeshRenderData[i].AffectingSpotLights.push_back( pLight );
			}
		}
	}
}

void Renderer::ClearLights()
{
	if (m_Mesh)
	{
		for( int i=0; i<m_Mesh->GetNumberOfSubmeshes(); i++ )
		{
			m_SubmeshRenderData[i].AffectingPointLights.clear();
			m_SubmeshRenderData[i].AffectingSpotLights.clear();
		}
	}
}

void Renderer::Cull( XNA::Frustum* frustum, Transform* pTransform )
{
	if (m_Mesh)
	{
		XNA::OrientedBox objectBox;
		for( int i=0; i<m_Mesh->GetNumberOfSubmeshes(); i++ )
		{
			Submesh* submesh = m_Mesh->GetSubmesh( i );

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
	if (m_Mesh)
	{
		XNA::OrientedBox objectBox;
		XNA::Sphere lightSphere;
		for( int i=0; i<m_Mesh->GetNumberOfSubmeshes(); i++ )
		{
			if (m_SubmeshRenderData[i].bVisible)
			{
				Submesh* submesh = m_Mesh->GetSubmesh( i );

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

void Renderer::CullLight( SpotLight* light, Transform* pTransform )
{
	if (m_Mesh)
	{
		XNA::OrientedBox objectBox;
		XNA::Frustum lightFrustum = light->GetFrustum();
		for( int i=0; i<m_Mesh->GetNumberOfSubmeshes(); i++ )
		{
			if (m_SubmeshRenderData[i].bVisible)
			{
				Submesh* submesh = m_Mesh->GetSubmesh( i );

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

void Renderer::SetCulled( bool IsCulled )
{
	for(int i=0; i<m_Mesh->GetNumberOfSubmeshes(); i++)
	{
		m_SubmeshRenderData[i].bVisible = !IsCulled;
	}
}

RenderSystem* Renderer::GetRenderSystem()	
{ 
	return m_RenderSystem; 
}

void Renderer::SetTime( float t )	
{ 
	m_Time = t; 
}

float Renderer::GetTime()
{ 
	return m_Time;
}