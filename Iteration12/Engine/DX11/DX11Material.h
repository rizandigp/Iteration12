#pragma once

#include "..\Material.h"
#include "DX11RenderCommand.h"
/*
class DX11Material : public Material
{
public:
	DX11Material( RenderSystem* pRenderSystem );

	// Bind material to the render commands, return number of passes
	virtual UINT Bind(Renderer* pRenderer, std::vector< D3D11RenderCommand_Draw* >* pRenderCommands, UINT submeshIndex, Transform* pTransform );
	virtual UINT Bind(Renderer* pRenderer, std::vector< RenderCommand* >* pRenderCommands, SubmeshRenderData* pRenderData, Transform* pTransform );
	UINT getNumberOfPasses();

protected:
	RenderSystem* m_pRenderSystem;
	UINT m_NumPasses;
	ID3D11BlendState*	m_pAdditiveBlendState;
	ID3D11BlendState*	m_pDefaultBlendState;
	ID3D11DepthStencilState* m_pAdditiveDepthStencilState;
	ID3D11DepthStencilState* m_pDefaultDepthStencilState;
	ID3D11RasterizerState*	m_pAdditiveRasterizerState;
	ID3D11RasterizerState*	m_pDefaultRasterizerState;
};*/



// Simple BRDF with detail normal mapping
class DX11Material_DiffuseDetailbump : public Material
{
public:
	DX11Material_DiffuseDetailbump( RenderSystem* pRenderSystem );

	void SetDiffusemap( Texture2D* pTexture )		{ m_pDiffuse = pTexture; };
	void SetNormalmap( Texture2D* pTexture )		{ m_pNormal = pTexture; };
	void SetSpecularIntensity( float val )			{ m_SpecIntensity = val; };
	void SetSpecularPower( float val )				{ m_SpecPower = val; };
	void SetDetailNormalmap( Texture2D* pTexture )	{ m_pDetailNormal = pTexture; };
	void SetDetailNormalStrength( float val )		{ m_DetailNormalStrength = val; };
	void SetDetailTiling( float val )				{ m_DetailTiling = val; };

	Texture2D* GetDetailNormalmap()					{ return m_pDetailNormal; };
	Texture2D* GetDiffuse()							{ return m_pDiffuse; };
	Texture2D* GetNormalmap()						{ return m_pNormal; };

	float GetDetailTiling()							{ return m_DetailTiling; };
		
	float GetSpecularIntensity()					{ return m_SpecIntensity; };
	float GetSpecularPower()						{ return m_SpecPower; };

	UINT Bind(Renderer* pRenderer, std::vector< D3D11RenderCommand_Draw* >* pRenderCommands, UINT submeshIndex, Transform* pTransform )	{return 0;};
	UINT Bind(Renderer* pRenderer, RenderCommand* pRenderCommands[], SubmeshRenderData* pRenderData, Transform* pTransform );

protected:
	Texture2D* m_pDiffuse;
	Texture2D* m_pNormal;
	Texture2D* m_pDetailNormal;

	float m_SpecIntensity;
	float m_SpecPower;
	float m_DetailNormalStrength;
	float m_DetailTiling;

	Shaderset* m_pShader[3];
	Shaderset* m_pShaderNoLight;
};

// Normalized Blinn-Phong
class DX11Material_BlinnPhong : public Material
{
public:
	DX11Material_BlinnPhong( RenderSystem* pRenderSystem );

	void SetDiffusemap( Texture2D* pTexture )	{ m_pDiffuse = pTexture; };
	void SetNormalmap( Texture2D* pTexture )	{ m_pNormal = pTexture; };
	void SetSpecularMap( Texture2D* pTexture )	{ m_pSpecular = pTexture; };
	void SetSpecularIntensity( float val )		{ m_SpecIntensity = val; };
	void SetSpecularPower( float val )			{ m_SpecPower = val; };

	Texture2D* GetDiffuse()			{ return m_pDiffuse; };
	Texture2D* GetNormalmap()		{ return m_pNormal; };
	Texture2D* GetSpecularMap()		{ return m_pSpecular; };
	float GetSpecularIntensity()	{ return m_SpecIntensity; };
	float GetSpecularPower()		{ return m_SpecPower; };

	UINT Bind(Renderer* pRenderer, std::vector< D3D11RenderCommand_Draw* >* pRenderCommands, UINT submeshIndex, Transform* pTransform ) {return 0;};
	UINT Bind(Renderer* pRenderer, RenderCommand* pRenderCommands[], SubmeshRenderData* pRenderData, Transform* pTransform );

protected:
	Texture2D* m_pDiffuse;
	Texture2D* m_pNormal;
	Texture2D* m_pSpecular;

	float m_SpecIntensity;
	float m_SpecPower;

	Shaderset* m_pShader[3];
	Shaderset* m_pShaderNoLight;
};


// Standard PBR material for deferred rendering.
// Fills the G-buffer
class DX11Material_Deferred : public Material
{
public:
	DX11Material_Deferred( RenderSystem* pRenderSystem );

	void SetDiffusemap( Texture2D* pTexture )	{ m_pDiffuse = pTexture; };
	void SetNormalmap( Texture2D* pTexture )	{ m_pNormal = pTexture; };
	void SetSpecularMap( Texture2D* pTexture )	{ m_pSpecular = pTexture; };
	void SetSpecularIntensity( float val )		{ m_SpecIntensity = val; };
	void SetGlossiness( float val )				{ m_Glossiness = val; };

	Texture2D* GetDiffusemap()			{ return m_pDiffuse; };
	Texture2D* GetNormalmap()			{ return m_pNormal; };
	Texture2D* GetSpecularMap()			{ return m_pSpecular; };
	float GetSpecularIntensity()		{ return m_SpecIntensity; };
	float GetSpecularPower()			{ return m_Glossiness; };

	UINT Bind(Renderer* pRenderer, std::vector< D3D11RenderCommand_Draw* >* pRenderCommands, UINT submeshIndex, Transform* pTransform ) {return 0;};
	UINT Bind(Renderer* pRenderer, RenderCommand* pRenderCommands[], SubmeshRenderData* pRenderData, Transform* pTransform );

protected:
	Texture2D* m_pDiffuse;
	Texture2D* m_pNormal;
	Texture2D* m_pSpecular;

	float m_SpecIntensity;
	float m_Glossiness;

	Shaderset* m_pShader[5];
};

// Same as above, but with Image-Based Lighting
class DX11Material_DeferredIBL : public DX11Material_Deferred
{
public:
	DX11Material_DeferredIBL( RenderSystem* pRenderSystem );

	void SetIBL( Texture* ptr )			{ m_pIBL = ptr; };
	void SetAOMap( Texture2D* ptr )		{ m_pAO = ptr; };
	Texture2D* GetAOMap()				{ return m_pAO; };
	Texture* GetIBL()					{ return m_pIBL; };

	UINT Bind(Renderer* pRenderer, std::vector< D3D11RenderCommand_Draw* >* pRenderCommands, UINT submeshIndex, Transform* pTransform ) {return 0;};
	UINT Bind(Renderer* pRenderer, RenderCommand* pRenderCommands[], SubmeshRenderData* pRenderData, Transform* pTransform );

protected:
	Texture2D* m_pAO;
	Texture* m_pIBL;
};

// Deferred spotlight material
class DX11Material_Spotlight : public Material
{
public:
	DX11Material_Spotlight( RenderSystem* pRenderSystem );

	void setGBuffer( Texture2D* ptr[3] )				{ m_pGbuffer[0] = ptr[0]; m_pGbuffer[1] = ptr[1]; m_pGbuffer[2] = ptr[2]; };
	void setShadowmap( Texture2D* ptr )					{ m_pShadowmap = ptr; };
	void setCookie( Texture2D* ptr )					{ m_pCookie = ptr; };
	void SetPosition( const Vector3 &spotlightPosition)	{ m_Position = spotlightPosition; };
	void setColor( const Vector3 &spotlightColor )		{ m_Color = spotlightColor; };
	void setIntensity( float spotlightIntensity )		{ m_Intensity = spotlightIntensity; };
	void setRadius( float spotlightRadius )				{ m_Radius = spotlightRadius; };
	void setInverseProjectionMatrix( Matrix4x4 &mat )	{ m_InvProjection = mat; };
	void setViewProjectionMatrix( Matrix4x4 &mat )		{ m_ViewProjection = mat; };
	void setViewMatrix( Matrix4x4 &mat )				{ m_View = mat; };
	
	Texture2D** getGBuffer()					{ return m_pGbuffer; };
	Texture2D* getShadowmap()					{ return m_pShadowmap; };
	
	UINT Bind(Renderer* pRenderer, std::vector< D3D11RenderCommand_Draw* >* pRenderCommands, UINT submeshIndex, Transform* pTransform ) {return 0;};
	UINT Bind(Renderer* pRenderer, RenderCommand* pRenderCommands[], SubmeshRenderData* pRenderData, Transform* pTransform );

protected:
	Texture2D* m_pGbuffer[3];
	Texture2D* m_pShadowmap;
	Texture2D* m_pCookie;
	Matrix4x4 m_InvProjection;
	Matrix4x4 m_ViewProjection;
	Matrix4x4 m_View;
	Shaderset* m_pShader;
	Vector3 m_Position;
	Vector3 m_Color;
	float m_Intensity;
	float m_Radius;

	ID3D11DepthStencilState* m_pBackfaceDepthStencilState;
	ID3D11RasterizerState* m_pBackfaceRasterizerState;
};

// Deferred pointlight material
class DX11Material_Pointlight : public Material
{
public:
	DX11Material_Pointlight( RenderSystem* pRenderSystem );

	void setGBuffer( Texture2D* ptr[3] )				{ m_pGbuffer[0] = ptr[0]; m_pGbuffer[1] = ptr[1]; m_pGbuffer[2] = ptr[2]; };
	void setShadowmap( Texture2D* ptr )					{ m_pShadowmap = ptr; };
	void setCookie( Texture2D* ptr )					{ m_pCookie = ptr; };
	void SetPosition( const Vector3 &spotlightPosition)	{ m_Position = spotlightPosition; };
	void setColor( const Vector3 &spotlightColor )		{ m_Color = spotlightColor; };
	void setIntensity( float spotlightIntensity )		{ m_Intensity = spotlightIntensity; };
	void setRadius( float spotlightRadius )				{ m_Radius = spotlightRadius; };
	
	Texture2D** getGBuffer()					{ return m_pGbuffer; };
	Texture2D* getShadowmap()					{ return m_pShadowmap; };
	
	UINT Bind(Renderer* pRenderer, std::vector< D3D11RenderCommand_Draw* >* pRenderCommands, UINT submeshIndex, Transform* pTransform ) {return 0;};
	UINT Bind(Renderer* pRenderer, RenderCommand* pRenderCommands[], SubmeshRenderData* pRenderData, Transform* pTransform );

protected:
	Texture2D* m_pGbuffer[3];
	Texture2D* m_pShadowmap;
	Texture2D* m_pCookie;
	Shaderset* m_pShader;
	Vector3 m_Position;
	Vector3 m_Color;
	float m_Intensity;
	float m_Radius;

	ID3D11DepthStencilState* m_pBackfaceDepthStencilState;
	ID3D11RasterizerState* m_pBackfaceRasterizerState;
};

// Duh owshun material
class DX11Material_Water : public Material
{
public:
	DX11Material_Water( RenderSystem* pRenderSystem );

	void SetDiffusemap( Texture2D* pTexture )	{ m_pDiffuse = pTexture; };
	void SetNormalmap( Texture2D* pTexture )	{ m_pNormal = pTexture; };
	void SetFoamTexture( Texture2D* pTexture )	{ m_pFoam = pTexture; };
	void SetSpecularMap( Texture2D* pTexture )	{ m_pSpecular = pTexture; };
	void SetSpecularIntensity( float val )		{ m_SpecIntensity = val; };
	void SetSpecularPower( float val )			{ m_SpecPower = val; };
	void SetIBL( Texture2D* ptr )				{ m_pIBL = ptr; };

	Texture2D* GetDiffuse()			{ return m_pDiffuse; };
	Texture2D* GetNormalmap()		{ return m_pNormal; };
	Texture2D* GetSpecularMap()		{ return m_pSpecular; };
	float GetSpecularIntensity()	{ return m_SpecIntensity; };
	float GetSpecularPower()		{ return m_SpecPower; };
	Texture2D* GetIBL()				{ return m_pIBL; };

	UINT Bind(Renderer* pRenderer, std::vector< D3D11RenderCommand_Draw* >* pRenderCommands, UINT submeshIndex, Transform* pTransform ) {return 0;};
	UINT Bind(Renderer* pRenderer, RenderCommand* pRenderCommands[], SubmeshRenderData* pRenderData, Transform* pTransform );

protected:
	Texture2D* m_pDiffuse;
	Texture2D* m_pNormal;
	Texture2D* m_pSpecular;
	Texture2D* m_pFoam;
	Texture2D* m_pIBL;

	float m_SpecIntensity;
	float m_SpecPower;

	Shaderset* m_pShader;
};

// Texture is assumed linear (i.e. HDR skyboxes), so no gamma correction is done
// in the shader.
class DX11Material_Skybox : public Material
{
public:
	DX11Material_Skybox( RenderSystem* pRenderSystem );

	void SetCubemap( TextureCube* pTexture )	{ m_pCubemap = pTexture; };
	TextureCube* GetCubemap()					{ return m_pCubemap; };
	
	UINT Bind(Renderer* pRenderer, std::vector< D3D11RenderCommand_Draw* >* pRenderCommands, UINT submeshIndex, Transform* pTransform ) {return 0;};
	UINT Bind(Renderer* pRenderer, RenderCommand* pRenderCommands[], SubmeshRenderData* pRenderData, Transform* pTransform );

protected:
	TextureCube* m_pCubemap;
	Shaderset* m_pShader;
};