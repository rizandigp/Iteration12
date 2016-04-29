#pragma once

#include "Prerequisites.h"
#include "DX11/DX11Renderer.h"
#include "Light.h"
#include "DX11/DX11Material.h"

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
	DX11Renderer m_Renderer;
	Texture2D* m_pGBuffers[3];
};