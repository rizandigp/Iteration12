#pragma once

#include "Prerequisites.h"
#include "DX11Renderer.h"
#include "Light.h"
#include "DX11Material.h"

class LightVolume
{
public:
	LightVolume( RenderSystem* pRenderSystem, SpotLight* pSpotlight );
	LightVolume( RenderSystem* pRenderSystem, PointLight* pPointlight );

	void setGBuffer(Texture2D* buffers[3]);
	void render();
	void renderVolumetricLight();

protected:
	SpotLight* m_pSpotlight;
	PointLight* m_pPointlight;
	Mesh* m_pMesh;
	DX11Renderer m_Renderer;
	Texture2D* m_pGBuffers[3];
};