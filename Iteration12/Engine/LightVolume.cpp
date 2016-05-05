#pragma once

#include "LightVolume.h"
#include "Material.h"

LightVolume::LightVolume( RenderSystem* pRenderSystem, SpotLight* pSpotlight )
{
	m_pPointlight = NULL;
	m_pSpotlight = pSpotlight;
	m_pMesh = pRenderSystem->LoadMesh( "Media/Meshes/unit_cube_2.obj" );
	//m_pMesh = pRenderSystem->CreateBoxWireframeMesh( XMFLOAT3(2.0f, 2.0f, 2.0f) );
	m_pMesh->SetMaterial( Material_Spotlight::Create( pRenderSystem ) );

	m_Renderer = pRenderSystem->CreateRenderer();
	m_Renderer->SetRenderSystem( pRenderSystem );
	m_Renderer->SetMesh( m_pMesh );
}

LightVolume::LightVolume( RenderSystem* pRenderSystem, PointLight* pPointlight )
{
	m_pSpotlight = NULL;
	m_pPointlight = pPointlight;
	m_pMesh = pRenderSystem->LoadMesh( "Media/Meshes/geosphere.obj" );
	//m_pMesh = pRenderSystem->CreateBoxWireframeMesh( XMFLOAT3(2.0f, 2.0f, 2.0f) );
	m_pMesh->SetMaterial( Material_Pointlight::Create( pRenderSystem ) );

	m_Renderer = pRenderSystem->CreateRenderer();
	m_Renderer->SetRenderSystem( pRenderSystem );
	m_Renderer->SetMesh( m_pMesh );
}

void LightVolume::SetGBuffer(Texture2D* buffers[3])
{
	m_pGBuffers[0] = buffers[0];
	m_pGBuffers[1] = buffers[1];
	m_pGBuffers[2] = buffers[2];
}

void LightVolume::Render()
{
	if (m_pSpotlight)
	{
		Material_Spotlight* mat = (Material_Spotlight*)m_pMesh->GetSubmesh(0)->GetMaterial();
		mat->setInverseProjectionMatrix( m_pSpotlight->GetProjectionCamera()->GetProjectionMatrix().inverse() );
		mat->setViewProjectionMatrix( m_pSpotlight->GetProjectionCamera()->GetViewProjectionMatrix() );
		mat->setViewMatrix( m_pSpotlight->GetProjectionCamera()->GetViewMatrix() );
		mat->setGBuffer( m_pGBuffers );
		mat->setShadowmap( m_pSpotlight->GetShadowmap() );
		mat->SetPosition( m_pSpotlight->Transformation()->GetPosition() );
		mat->setColor( m_pSpotlight->GetColor() );
		mat->setIntensity( m_pSpotlight->GetIntensity() );
		mat->setRadius( m_pSpotlight->GetRadius() );
		if (m_pSpotlight->IsRSMEnabled()&&m_pSpotlight->IsCastingShadow())
			mat->setCookie( m_pSpotlight->GetRSMNormal() );
		else
			mat->setCookie( m_pSpotlight->GetCookie() );

		m_Renderer->Render( m_pSpotlight->Transformation() );
	}
	else if (m_pPointlight)
	{
		Material_Pointlight* mat = (Material_Pointlight*)m_pMesh->GetSubmesh(0)->GetMaterial();
		mat->setGBuffer( m_pGBuffers );
		mat->SetPosition( m_pPointlight->Transformation()->GetPosition() );
		mat->setColor( m_pPointlight->getColor() );
		mat->setIntensity( m_pPointlight->GetIntensity() );
		mat->setRadius( m_pPointlight->GetRadius() );

		m_Renderer->Render( m_pPointlight->Transformation() );
	}
}