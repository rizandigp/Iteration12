#pragma once

#include "..\Material.h"
#include "DX11RenderCommand.h"


// Simple BRDF with detail normal mapping
class DX11Material_DiffuseDetailbump : public Material_DiffuseDetailbump
{
public:
	DX11Material_DiffuseDetailbump( RenderSystem* pRenderSystem );

	UINT Bind(Renderer* pRenderer, RenderCommand* pRenderCommands[], SubmeshRenderData* pRenderData, Transform* pTransform );
};

// Normalized Blinn-Phong
class DX11Material_BlinnPhong : public Material_BlinnPhong
{
public:
	DX11Material_BlinnPhong( RenderSystem* pRenderSystem );

	UINT Bind(Renderer* pRenderer, RenderCommand* pRenderCommands[], SubmeshRenderData* pRenderData, Transform* pTransform );
};


// Standard PBR material for deferred rendering.
// Fills the G-buffer
class DX11Material_Deferred : public Material_Deferred
{
public:
	DX11Material_Deferred( RenderSystem* pRenderSystem );

	UINT Bind(Renderer* pRenderer, RenderCommand* pRenderCommands[], SubmeshRenderData* pRenderData, Transform* pTransform );
};

// Same as above, but with Image-Based Lighting
class DX11Material_DeferredIBL : public Material_DeferredIBL
{
public:
	DX11Material_DeferredIBL( RenderSystem* pRenderSystem );

	UINT Bind(Renderer* pRenderer, RenderCommand* pRenderCommands[], SubmeshRenderData* pRenderData, Transform* pTransform );
};

// Deferred spotlight material
class DX11Material_Spotlight : public Material_Spotlight
{
public:
	DX11Material_Spotlight( RenderSystem* pRenderSystem );
	
	UINT Bind(Renderer* pRenderer, RenderCommand* pRenderCommands[], SubmeshRenderData* pRenderData, Transform* pTransform );
};

// Deferred pointlight material
class DX11Material_Pointlight : public Material_Pointlight
{
public:
	DX11Material_Pointlight( RenderSystem* pRenderSystem );

	UINT Bind(Renderer* pRenderer, RenderCommand* pRenderCommands[], SubmeshRenderData* pRenderData, Transform* pTransform );
};

// Duh owshun material
class DX11Material_Water : public Material_Water
{
public:
	DX11Material_Water( RenderSystem* pRenderSystem );

	UINT Bind(Renderer* pRenderer, RenderCommand* pRenderCommands[], SubmeshRenderData* pRenderData, Transform* pTransform );
};

// Texture is assumed linear (i.e. HDR skyboxes), so no gamma correction is done
// in the shader.
class DX11Material_Skybox : public Material_Skybox
{
public:
	DX11Material_Skybox( RenderSystem* pRenderSystem );

	UINT Bind(Renderer* pRenderer, RenderCommand* pRenderCommands[], SubmeshRenderData* pRenderData, Transform* pTransform );
};