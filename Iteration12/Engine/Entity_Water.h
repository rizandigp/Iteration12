#pragma once

#include "Prerequisites.h"

#include "Entity_Prop.h"
#include "Scene.h"
#include "RenderSystem.h"
#include "FFTWater.h"

class Entity_Water : public Entity_StaticProp
{
public:
	Entity_Water(RenderSystem* ptr);

	void setGridProjectorCamera( Camera3D* ptr );
	Camera3D* getGridProjectorCamera();

	void update( float deltaTime );

protected:
	FFTWater*	m_pFFTWater;
	float m_Time;
	Camera3D* m_pProjectorCamera;
};