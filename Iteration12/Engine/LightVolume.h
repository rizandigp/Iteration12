#pragma once

#include "Prerequisites.h"
#include "Renderer.h"
#include "Light.h"


class LightVolume
{
public:
	LightVolume( RenderSystem* pRenderSystem, SpotLight* spotlight );
	LightVolume( RenderSystem* pRenderSystem, PointLight* sointlight );
	~LightVolume();

	void SetGBuffer(Texture2D* buffers[3]);
	void Render();
	void RenderVolumetricLight();

protected:
	SpotLight* m_Spotlight;
	PointLight* m_Pointlight;
	Mesh* m_Mesh;
	Material* m_Material;
	Renderer* m_Renderer;
	Texture2D* m_GBuffers[3];
};