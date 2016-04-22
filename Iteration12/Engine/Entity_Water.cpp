#pragma once

#include "Entity_Water.h"

Entity_Water::Entity_Water(RenderSystem* ptr) : Entity_StaticProp(ptr, NULL), m_Time(0.0f)
{
	// Mesh is initialized here
	m_pFFTWater = new FFTWater(ptr, 128, 0.0005f, Vector2(0.0f,16.0f), 64, false, false);
	// Get the mesh
	SetMesh(m_pFFTWater->GetMesh());
}

void Entity_Water::SetGridProjectorCamera( Camera3D* ptr )
{
	m_pFFTWater->SetGridProjectorCamera( ptr );
}

Camera3D* Entity_Water::GetGridProjectorCamera()
{
	return m_pFFTWater->GetGridProjectorCamera();
}

void Entity_Water::Update( float deltaTime )
{
	m_Time += deltaTime;
	m_Renderer.SetTime( m_Time );
	m_pFFTWater->EvaluateWavesFFT( m_Time );
	m_pFFTWater->GenerateWaterSurfaceLod(4);
	Entity_StaticProp::Update( deltaTime );
}