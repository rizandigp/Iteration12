#pragma once

#include "Prerequisites.h"

#include "Texture2D.h"
#include "Renderer.h"
#include "Mesh.h"
#include "ShaderParams.h"
#include "RenderSystem.h"


// Base class
class Material
{
public:
	Material( RenderSystem* pRenderSystem );

	// Bind material to the render commands, return number of passes
	virtual UINT Bind(Renderer* pRenderer, std::vector< D3D11RenderCommand_Draw* >* pRenderCommands, UINT submeshIndex, Transform* pTransform )=0;
	virtual UINT Bind(Renderer* pRenderer, RenderCommand* pRenderCommands[], SubmeshRenderData* pRenderData, Transform* pTransform )	{return 0;};
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
	ID3D11RasterizerState*	m_pWireframeRasterizerState;
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

	UINT Bind(Renderer* pRenderer, std::vector< D3D11RenderCommand_Draw* >* pRenderCommands, UINT submeshIndex, Transform* pTransform ){};
	//UINT Bind(Renderer* pRenderer, RenderCommand* pRenderCommands[], SubmeshRenderData* pRenderData, Transform* pTransform );

protected:
	Texture2D* m_pDiffuse;
	Texture2D* m_pNormal;

	float m_SpecIntensity;
	float m_SpecPower;

	Shaderset* m_pShader[3];
	Shaderset* m_pShaderNoLight;
};

class Material_DiffuseDetailbump : public Material_DiffuseBump
{
public:
	//Material_DiffuseDetailbump( RenderSystem* pRenderSystem ){};

	void SetDetailNormalmap( Texture2D* pTexture )	{ m_pDetailNormal = pTexture; };
	void SetDetailNormalStrength( float val )		{ m_DetailNormalStrength = val; };
	void SetDetailTiling( float val )				{ m_DetailTiling = val; };

	Texture2D* GetDetailNormalmap()					{ return m_pDetailNormal; };
	float GetDetailTiling()							{ return m_DetailTiling; };

	UINT Bind(Renderer* pRenderer, std::vector< D3D11RenderCommand_Draw* >* pRenderCommands, UINT submeshIndex, Transform* pTransform ){};
	UINT Bind(Renderer* pRenderer, RenderCommand* pRenderCommands[], SubmeshRenderData* pRenderData, Transform* pTransform );

protected:
	float m_DetailNormalStrength;
	float m_DetailTiling;
	Texture2D* m_pDetailNormal;
};

class Material_DiffuseBumpSpecular : public Material_DiffuseBump
{
public:
	//Material_DiffuseBumpSpecular( RenderSystem* pRenderSystem ){};

	void SetSpecularMap( Texture2D* pTexture )	{ m_pSpecular = pTexture; };
	
	Texture2D* GetSpecularMap()					{ return m_pSpecular; };

	UINT Bind(Renderer* pRenderer, std::vector< D3D11RenderCommand_Draw* >* pRenderCommands, UINT submeshIndex, Transform* pTransform ){};
	//UINT Bind(Renderer* pRenderer, RenderCommand* pRenderCommands[], SubmeshRenderData* pRenderData, Transform* pTransform );

protected:
	Texture2D* m_pSpecular;
};

class Material_Diffuse : public Material
{
public:
	//Material_Diffuse( RenderSystem* pRenderSystem );

	void SetDiffusemap( Texture2D* pTexture )		{ m_pDiffuse = pTexture; };
	void SetSpecularIntensity( float val )		{ m_SpecIntensity = val; };
	void SetSpecularPower( float val )			{ m_SpecPower = val; };

	Texture2D* GetDiffuse()			{ return m_pDiffuse; };
	float GetSpecularIntensity()	{ return m_SpecIntensity; };
	float GetSpecularPower()		{ return m_SpecPower; };

	UINT Bind(Renderer* pRenderer, std::vector< D3D11RenderCommand_Draw* >* pRenderCommands, UINT submeshIndex, Transform* pTransform ){};
	//UINT Bind(Renderer* pRenderer, RenderCommand* pRenderCommands[], SubmeshRenderData* pRenderData, Transform* pTransform );

protected:
	Texture2D* m_pDiffuse;

	float m_SpecIntensity;
	float m_SpecPower;

	Shaderset* m_pShader[2];
	ID3D11BlendState*	m_pBlendState;
	ID3D11DepthStencilState* m_pDepthStencilState;
	ID3D11RasterizerState*	m_pRasterizerState;
};


class Material_BlinnPhong : public Material
{
public:
	//Material_BlinnPhong( RenderSystem* pRenderSystem ){};

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

	UINT Bind(Renderer* pRenderer, std::vector< D3D11RenderCommand_Draw* >* pRenderCommands, UINT submeshIndex, Transform* pTransform ){};
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