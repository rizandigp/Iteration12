#pragma once

#include "LightVolume.h"

LightVolume::LightVolume( RenderSystem* pRenderSystem, SpotLight* pSpotlight )
{
	m_pPointlight = NULL;
	m_pSpotlight = pSpotlight;
	m_pMesh = pRenderSystem->loadMesh( "Media/Meshes/unit_cube_2.obj" );
	//m_pMesh = pRenderSystem->createBoxWireframeMesh( XMFLOAT3(2.0f, 2.0f, 2.0f) );
	m_pMesh->setMaterial( new DX11Material_Spotlight( pRenderSystem ) );
	m_Renderer.setRenderSystem( pRenderSystem );
	m_Renderer.setMesh( m_pMesh );
}

LightVolume::LightVolume( RenderSystem* pRenderSystem, PointLight* pPointlight )
{
	m_pSpotlight = NULL;
	m_pPointlight = pPointlight;
	m_pMesh = pRenderSystem->loadMesh( "Media/Meshes/geosphere.obj" );
	//m_pMesh = pRenderSystem->createBoxWireframeMesh( XMFLOAT3(2.0f, 2.0f, 2.0f) );
	m_pMesh->setMaterial( new DX11Material_Pointlight( pRenderSystem ) );
	m_Renderer.setRenderSystem( pRenderSystem );
	m_Renderer.setMesh( m_pMesh );
}

void LightVolume::setGBuffer(Texture2D* buffers[3])
{
	m_pGBuffers[0] = buffers[0];
	m_pGBuffers[1] = buffers[1];
	m_pGBuffers[2] = buffers[2];
}

void LightVolume::render()
{
	if (m_pSpotlight)
	{
		DX11Material_Spotlight* mat = ((DX11Material_Spotlight*)m_pMesh->getSubmesh(0)->getMaterial());
		mat->setInverseProjectionMatrix( m_pSpotlight->getProjectionCamera()->getProjectionMatrix().inverse() );
		mat->setViewProjectionMatrix( m_pSpotlight->getProjectionCamera()->getViewProjectionMatrix() );
		mat->setViewMatrix( m_pSpotlight->getProjectionCamera()->getViewMatrix() );
		mat->setGBuffer( m_pGBuffers );
		mat->setShadowmap( m_pSpotlight->getShadowmap() );
		mat->setPosition( m_pSpotlight->transform()->getPosition() );
		mat->setColor( m_pSpotlight->getColor() );
		mat->setIntensity( m_pSpotlight->getIntensity() );
		mat->setRadius( m_pSpotlight->getRadius() );
		if (m_pSpotlight->isRSMEnabled()&&m_pSpotlight->isCastingShadow())
			mat->setCookie( m_pSpotlight->getRSMNormal() );
		else
			mat->setCookie( m_pSpotlight->getCookie() );

		m_Renderer.render( m_pSpotlight->transform() );
	}
	else if (m_pPointlight)
	{
		DX11Material_Pointlight* mat = ((DX11Material_Pointlight*)m_pMesh->getSubmesh(0)->getMaterial());
		mat->setGBuffer( m_pGBuffers );
		mat->setPosition( m_pPointlight->transform()->getPosition() );
		mat->setColor( m_pPointlight->getColor() );
		mat->setIntensity( m_pPointlight->getIntensity() );
		mat->setRadius( m_pPointlight->getRadius() );

		m_Renderer.render( m_pPointlight->transform() );
	}
}