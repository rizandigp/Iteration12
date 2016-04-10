#pragma once

#include "RenderCommand.h"
#include "ShaderParams.h"


// Generic drawing
// New DX11 class
class DX11RenderCommand_Draw : public RenderCommand
{
public:
	DX11RenderCommand_Draw()	: m_pRasterizerState( NULL ), m_pDepthStencilState(NULL), m_pBlendState( NULL )		{};

	// MUTATORS
	inline void setGeometryChunk( D3D11GeometryChunk *pGeometryChunk )					{ m_pGeometryChunk = pGeometryChunk; };
	inline void setShaderset( D3D11Shaderset *pShaderset )								{ m_pShaderset = pShaderset;};
	inline void clearTextures()															{ m_pTextures.clear(); };
	inline void setShaderParams( ShaderParams* pParams )								{ m_ShaderParams = *pParams; };
	void setTexture( std::string const &name, DX11Texture2D *pTexture );//					{ m_pTextures.push_back( std::pair< std::string, DX11Texture2D* > (name,pTexture) );};

	inline void setBlendState( ID3D11BlendState* pBlendState )							{ m_pBlendState = pBlendState; };
	inline void setDepthStencilState( ID3D11DepthStencilState* pDepthStencilState )		{ m_pDepthStencilState = pDepthStencilState; };
	inline void setRasterizerState( ID3D11RasterizerState* pRasterizerState )			{ m_pRasterizerState = pRasterizerState; };


	// ACCESSORS
	inline D3D11GeometryChunk *getGeometryChunk()									{ return m_pGeometryChunk; };
	inline D3D11Shaderset *getShaderset()											{ return m_pShaderset; };
	//inline ShaderParamBlock* shaderParams()											{ return &m_ShaderParamBlock; };
	inline std::vector< std::pair< std::string, DX11Texture2D* >, tbb::scalable_allocator<std::pair< std::string, DX11Texture2D* >> > *getTextures()	{ return &m_pTextures; };

	void execute( RenderDispatcher* pDispatcher );

	void clone( RenderCommand* ptr );

protected:
	D3D11Shaderset		*m_pShaderset;
	D3D11GeometryChunk	*m_pGeometryChunk;
	std::vector< std::pair< std::string, DX11Texture2D* >, tbb::scalable_allocator<std::pair< std::string, DX11Texture2D* >> > m_pTextures;
	//ShaderParamBlock	m_ShaderParamBlock;
	ShaderParams		m_ShaderParams;
	// DX11 state objects
	ID3D11BlendState*	m_pBlendState;
	ID3D11DepthStencilState*	m_pDepthStencilState;
	ID3D11RasterizerState*	m_pRasterizerState;
};

// Render command for mapping data to GPU
class DX11RenderCommand_Map : public RenderCommand
{
public:
	DX11RenderCommand_Map()	: m_pResource(NULL)	{};

	inline void setResource( ID3D11Resource* ptr, void* pData, UINT dataSize )	{ m_pResource = ptr; m_pData = pData; m_DataSize = dataSize; };

	void execute( RenderDispatcher* pDispatcher );

	void clone( RenderCommand* ptr );
protected:
	ID3D11Resource*		m_pResource;
	void*	m_pData;
	UINT	m_DataSize;
};