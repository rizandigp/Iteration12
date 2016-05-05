#pragma once

#include "Prerequisites.h"
#include "Renderer.h"
#include "Light.h"


class LightVolume
{
public:
	LightVolume( RenderSystem* pRenderSystem, SpotLight* pSpotlight );
	LightVolume( RenderSystem* pRenderSystem, PointLight* pPointlight );

	void SetGBuffer(Texture2D* buffers[3]);
	void Render();
	void RenderVolumetricLight();

protected:
	SpotLight* m_pSpotlight;
	PointLight* m_pPointlight;
	Mesh* m_pMesh;
	Renderer* m_Renderer;
	Texture2D* m_pGBuffers[3];
};