#pragma once

#include "LightVolume.h"
#include "Material.h"

LightVolume::LightVolume( RenderSystem* renderSystem, SpotLight* spotlight )
{
	m_Pointlight = NULL;
	m_Spotlight = spotlight;
	m_Mesh = renderSystem->LoadMesh( "Media/Meshes/unit_cube_2.obj" );
	//m_pMesh = pRenderSystem->CreateBoxWireframeMesh( XMFLOAT3(2.0f, 2.0f, 2.0f) );
	m_Material = Material_Spotlight::Create( renderSystem );
	m_Mesh->SetMaterial( m_Material );

	m_Renderer = renderSystem->CreateRenderer();
	m_Renderer->SetRenderSystem( renderSystem );
	m_Renderer->SetMesh( m_Mesh );
}

LightVolume::LightVolume( RenderSystem* renderSystem, PointLight* pointlight )
{
	m_Spotlight = NULL;
	m_Pointlight = pointlight;
	m_Mesh = renderSystem->LoadMesh( "Media/Meshes/geosphere.obj" );
	//m_pMesh = pRenderSystem->CreateBoxWireframeMesh( XMFLOAT3(2.0f, 2.0f, 2.0f) );
	m_Material = Material_Pointlight::Create( renderSystem );
	m_Mesh->SetMaterial( m_Material );

	m_Renderer = renderSystem->CreateRenderer();
	m_Renderer->SetRenderSystem( renderSystem );
	m_Renderer->SetMesh( m_Mesh );
}

void LightVolume::SetGBuffer(Texture2D* buffers[3])
{
	m_GBuffers[0] = buffers[0];
	m_GBuffers[1] = buffers[1];
	m_GBuffers[2] = buffers[2];
}

void LightVolume::Render()
{
	if (m_Spotlight)
	{
		Material_Spotlight* mat = (Material_Spotlight*)m_Mesh->GetSubmesh(0)->GetMaterial();
		mat->setInverseProjectionMatrix( m_Spotlight->GetProjectionCamera()->GetProjectionMatrix().inverse() );
		mat->setViewProjectionMatrix( m_Spotlight->GetProjectionCamera()->GetViewProjectionMatrix() );
		mat->setViewMatrix( m_Spotlight->GetProjectionCamera()->GetViewMatrix() );
		mat->setGBuffer( m_GBuffers );
		mat->setShadowmap( m_Spotlight->GetShadowmap() );
		mat->SetPosition( m_Spotlight->Transformation()->GetPosition() );
		mat->setColor( m_Spotlight->GetColor() );
		mat->setIntensity( m_Spotlight->GetIntensity() );
		mat->setRadius( m_Spotlight->GetRadius() );
		mat->setNoiseTexture( m_Renderer->GetRenderSystem()->CreateNoiseTexture( 4, 4 ) );
		if (m_Spotlight->IsRSMEnabled()&&m_Spotlight->IsCastingShadow())
			mat->setCookie( m_Spotlight->GetRSMNormal() );
		else
			mat->setCookie( m_Spotlight->GetCookie() );

		m_Renderer->Render( m_Spotlight->Transformation() );
	}
	else if (m_Pointlight)
	{
		Material_Pointlight* mat = (Material_Pointlight*)m_Mesh->GetSubmesh(0)->GetMaterial();
		mat->setGBuffer( m_GBuffers );
		mat->SetPosition( m_Pointlight->Transformation()->GetPosition() );
		mat->setColor( m_Pointlight->getColor() );
		mat->setIntensity( m_Pointlight->GetIntensity() );
		mat->setRadius( m_Pointlight->GetRadius() );

		m_Renderer->Render( m_Pointlight->Transformation() );
	}
}

LightVolume::~LightVolume()
{
	delete m_Renderer;
	delete m_Material;
}