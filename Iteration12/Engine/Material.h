#pragma once

#include "Prerequisites.h"

#include "Texture2D.h"
#include "Renderer.h"
#include "Mesh.h"
#include "ShaderParams.h"
#include "RenderSystem.h"
#include "RenderState.h"
#include "BlendState.h"


// Base class
class Material
{
public:
	Material( RenderSystem* renderSystem );

	// Bind material to the render commands, return number of passes
	virtual UINT Bind(Renderer* pRenderer, RenderCommand* pRenderCommands[], SubmeshRenderData* pRenderData, Transform* pTransform )=0;

	UINT GetNumberOfPasses();

protected:
	RenderSystem* m_pRenderSystem;
	UINT m_NumPasses;
	RenderState		m_RenderState;
	BlendState		m_BlendState;
};

class Material_DiffuseBump : public Material
{
public:
	//Material_DiffuseBump( RenderSystem* pRenderSystem ){};

	void SetDiffusemap( Texture2D* pTexture )	{ m_pDiffuse = pTexture; };
	void SetNormalmap( Texture2D* pTexture )	{ m_pNormal = pTexture; };
	void SetSpecularIntensity( float val )		{ m_SpecIntensity = val; };
	void SetSpecularPower( float val )			{ m_SpecPower = val; };

	Texture2D* GetDiffuse()			{ return m_pDiffuse; };
	Texture2D* GetNormalmap()		{ return m_pNormal; };
	float GetSpecularIntensity()	{ return m_SpecIntensity; };
	float GetSpecularPower()		{ return m_SpecPower; };

	UINT Bind(Renderer* pRenderer, RenderCommand* pRenderCommands[], SubmeshRenderData* pRenderData, Transform* pTransform );

protected:
	Texture2D* m_pDiffuse;
	Texture2D* m_pNormal;

	float m_SpecIntensity;
	float m_SpecPower;

	Shaderset* m_pShader[3];
	Shaderset* m_pShaderNoLight;
};


//-------------------------------------------------------------------------------------
// These are implemented
//-------------------------------------------------------------------------------------
// Simple BRDF with detail normal mapping
class Material_DiffuseDetailbump : public Material
{
public:
	static Material_DiffuseDetailbump* Create( RenderSystem* renderSystem );

	Material_DiffuseDetailbump( RenderSystem* renderSystem ) :	m_DetailNormalStrength(1.0f),
																m_DetailTiling(10.0f),
																Material (renderSystem)
	{
	};

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

	//UINT Bind(Renderer* pRenderer, RenderCommand* pRenderCommands[], SubmeshRenderData* pRenderData, Transform* pTransform );

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
class Material_BlinnPhong : public Material
{
public:
	static Material_BlinnPhong* Create( RenderSystem* renderSystem );

	Material_BlinnPhong( RenderSystem* renderSystem ) :	m_pDiffuse(NULL),
														m_pNormal(NULL),
														m_pSpecular(NULL),
														Material(renderSystem)
	{
	};

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

	//UINT Bind(Renderer* pRenderer, RenderCommand* pRenderCommands[], SubmeshRenderData* pRenderData, Transform* pTransform );

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
class Material_Deferred : public Material
{
public:
	static Material_Deferred* Create( RenderSystem* renderSystem );

	Material_Deferred( RenderSystem* renderSystem ) :	m_pDiffuse(NULL),
														m_pNormal(NULL),
														m_pSpecular(NULL),
														m_SpecIntensity(1.0f),
														m_Glossiness(2048.0f),
														Material(renderSystem)
	{
	};

	void SetDiffusemap( Texture2D* pTexture )	{ m_pDiffuse = pTexture; };
	void SetNormalmap( Texture2D* pTexture )	{ m_pNormal = pTexture; };
	void SetSpecularMap( Texture2D* pTexture )	{ m_pSpecular = pTexture; };
	void SetSpecularIntensity( float val )		{ m_SpecIntensity = val; };
	void SetGlossiness( float val )				{ m_Glossiness = val; };
	void SetIBL( TextureCube* ptr )				{ m_pIBL = ptr; };
	void SetAOMap( Texture2D* ptr )				{ m_pAO = ptr; };

	Texture2D* GetDiffusemap()			{ return m_pDiffuse; };
	Texture2D* GetNormalmap()			{ return m_pNormal; };
	Texture2D* GetSpecularMap()			{ return m_pSpecular; };
	Texture2D* GetAOMap()				{ return m_pAO; };
	TextureCube* GetIBL()				{ return m_pIBL; };
	float GetSpecularIntensity()		{ return m_SpecIntensity; };
	float GetSpecularPower()			{ return m_Glossiness; };

	//UINT Bind(Renderer* pRenderer, RenderCommand* pRenderCommands[], SubmeshRenderData* pRenderData, Transform* pTransform );

protected:
	Texture2D* m_pDiffuse;
	Texture2D* m_pNormal;
	Texture2D* m_pSpecular;
	Texture2D* m_pAO;
	TextureCube* m_pIBL;

	float m_SpecIntensity;
	float m_Glossiness;

	Shaderset* m_pShader[5];
};

// Same as above, but with Image-Based Lighting
class Material_DeferredIBL : public Material_Deferred
{
public:
	static Material_DeferredIBL* Create( RenderSystem* renderSystem );

	Material_DeferredIBL( RenderSystem* renderSystem ) :	Material_Deferred(renderSystem)
	{
	};

	//UINT Bind(Renderer* pRenderer, RenderCommand* pRenderCommands[], SubmeshRenderData* pRenderData, Transform* pTransform );
};

// Deferred spotlight material
class Material_Spotlight : public Material
{
public:
	static Material_Spotlight* Create( RenderSystem* pRenderSystem );

	Material_Spotlight( RenderSystem* renderSystem ) :	m_Shadowmap(NULL), m_Cookie(NULL), m_Noise(NULL), m_ShadowBlurRadius(4.0f), Material(renderSystem)
	{
		m_Gbuffer[0] = NULL;
		m_Gbuffer[1] = NULL;
		m_Gbuffer[2] = NULL;
	};

	void setGBuffer( Texture2D* ptr[3] )				{ m_Gbuffer[0] = ptr[0]; m_Gbuffer[1] = ptr[1]; m_Gbuffer[2] = ptr[2]; };
	void setShadowmap( Texture2D* texture )				{ m_Shadowmap = texture; };
	void setCookie( Texture2D* texture )				{ m_Cookie = texture; };
	void SetPosition( const Vector3 &spotlightPosition)	{ m_Position = spotlightPosition; };
	void setColor( const Vector3 &spotlightColor )		{ m_Color = spotlightColor; };
	void setIntensity( float spotlightIntensity )		{ m_Intensity = spotlightIntensity; };
	void setRadius( float spotlightRadius )				{ m_Radius = spotlightRadius; };
	void setInverseProjectionMatrix( Matrix4x4 &mat )	{ m_InvProjection = mat; };
	void setViewProjectionMatrix( Matrix4x4 &mat )		{ m_ViewProjection = mat; };
	void setViewMatrix( Matrix4x4 &mat )				{ m_View = mat; };
	void setNoiseTexture( Texture2D* texture )			{ m_Noise = texture; }
	void setShadowBlurRadius( float texels )			{ m_ShadowBlurRadius = texels; };
	
	Texture2D** getGBuffer()					{ return m_Gbuffer; };
	Texture2D* getShadowmap()					{ return m_Shadowmap; };
	
	//UINT Bind(Renderer* pRenderer, RenderCommand* pRenderCommands[], SubmeshRenderData* pRenderData, Transform* pTransform );

protected:
	Texture2D* m_Gbuffer[3];
	Texture2D* m_Shadowmap;
	Texture2D* m_Cookie;
	Texture2D* m_Noise;
	Matrix4x4 m_InvProjection;
	Matrix4x4 m_ViewProjection;
	Matrix4x4 m_View;
	Shaderset* m_pShader;
	Vector3 m_Position;
	Vector3 m_Color;
	float m_Intensity;
	float m_Radius;
	float m_ShadowBlurRadius;
};

// Deferred pointlight material
class Material_Pointlight : public Material
{
public:
	static Material_Pointlight* Create( RenderSystem* renderSystem );

	Material_Pointlight( RenderSystem* renderSystem ) :	m_pShadowmap(NULL), Material(renderSystem)
	{
		m_pGbuffer[0] = NULL;
		m_pGbuffer[1] = NULL;
		m_pGbuffer[2] = NULL;
	};

	void setGBuffer( Texture2D* ptr[3] )				{ m_pGbuffer[0] = ptr[0]; m_pGbuffer[1] = ptr[1]; m_pGbuffer[2] = ptr[2]; };
	void setShadowmap( Texture2D* ptr )					{ m_pShadowmap = ptr; };
	void setCookie( Texture2D* ptr )					{ m_pCookie = ptr; };
	void SetPosition( const Vector3 &spotlightPosition)	{ m_Position = spotlightPosition; };
	void setColor( const Vector3 &spotlightColor )		{ m_Color = spotlightColor; };
	void setIntensity( float spotlightIntensity )		{ m_Intensity = spotlightIntensity; };
	void setRadius( float spotlightRadius )				{ m_Radius = spotlightRadius; };
	
	Texture2D** getGBuffer()					{ return m_pGbuffer; };
	Texture2D* getShadowmap()					{ return m_pShadowmap; };
	
	//UINT Bind(Renderer* pRenderer, RenderCommand* pRenderCommands[], SubmeshRenderData* pRenderData, Transform* pTransform );

protected:
	Texture2D* m_pGbuffer[3];
	Texture2D* m_pShadowmap;
	Texture2D* m_pCookie;
	Shaderset* m_pShader;
	Vector3 m_Position;
	Vector3 m_Color;
	float m_Intensity;
	float m_Radius;
};

// Duh owshun material
class Material_Water : public Material
{
public:
	static Material_Water* Create( RenderSystem* renderSystem );

	Material_Water( RenderSystem* renderSystem ) :	Material(renderSystem)
	{
	};

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

	//UINT Bind(Renderer* pRenderer, RenderCommand* pRenderCommands[], SubmeshRenderData* pRenderData, Transform* pTransform );

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
class Material_Skybox : public Material
{
public:
	static Material_Skybox* Create( RenderSystem* renderSystem );

	Material_Skybox( RenderSystem* renderSystem ) :	Material(renderSystem)
	{
	};

	void SetCubemap( TextureCube* pTexture )	{ m_pCubemap = pTexture; };
	TextureCube* GetCubemap()					{ return m_pCubemap; };
	
	//UINT Bind(Renderer* pRenderer, RenderCommand* pRenderCommands[], SubmeshRenderData* pRenderData, Transform* pTransform );

protected:
	TextureCube* m_pCubemap;
	Shaderset* m_pShader;
};

// Used for stencil masking: light volumes, shadow volumes, light clips, etc.
class Material_StencilMask : public Material
{
public:
	static Material_StencilMask* Create( RenderSystem* renderSystem );

	Material_StencilMask( RenderSystem* renderSystem ) :	Material(renderSystem)
	{
	};

public:
	RenderState _RenderState;
	BlendState _BlendState;
	UINT _StencilRef;

protected:
	Shaderset* m_Shader;
};