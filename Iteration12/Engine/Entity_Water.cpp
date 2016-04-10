#pragma once

#include "Entity_Water.h"

Entity_Water::Entity_Water(RenderSystem* ptr) : Entity_StaticProp(ptr, NULL), m_Time(0.0f)
{
	// Mesh is initialized here
	m_pFFTWater = new FFTWater(ptr, 128, 0.0005f, Vector2(0.0f,16.0f), 64, false, false);
	// Get the mesh
	setMesh(m_pFFTWater->getMesh());
}

void Entity_Water::setGridProjectorCamera( Camera3D* ptr )
{
	m_pFFTWater->setGridProjectorCamera( ptr );
}

Camera3D* Entity_Water::getGridProjectorCamera()
{
	return m_pFFTWater->getGridProjectorCamera();
}

void Entity_Water::update( float deltaTime )
{
	m_Time += deltaTime;
	m_Renderer.setTime( m_Time );
	m_pFFTWater->evaluateWavesFFT( m_Time );
	m_pFFTWater->generateWaterSurfaceLod(4);
	Entity_StaticProp::update( deltaTime );
}