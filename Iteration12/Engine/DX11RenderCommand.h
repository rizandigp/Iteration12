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
	inline void SetGeometryChunk( D3D11GeometryChunk *pGeometryChunk )					{ m_pGeometryChunk = pGeometryChunk; };
	inline void SetShaderset( D3D11Shaderset *pShaderset )								{ m_pShaderset = pShaderset;};
	inline void ClearTextures()															{ m_pTextures.clear(); };
	inline void SetShaderParams( ShaderParams* pParams )								{ m_ShaderParams = *pParams; };
	void SetTexture( std::string const &name, DX11Texture2D *pTexture );//					{ m_pTextures.push_back( std::pair< std::string, DX11Texture2D* > (name,pTexture) );};

	inline void SetBlendState( ID3D11BlendState* pBlendState )							{ m_pBlendState = pBlendState; };
	inline void SetDepthStencilState( ID3D11DepthStencilState* pDepthStencilState )		{ m_pDepthStencilState = pDepthStencilState; };
	inline void SetRasterizerState( ID3D11RasterizerState* pRasterizerState )			{ m_pRasterizerState = pRasterizerState; };


	// ACCESSORS
	inline D3D11GeometryChunk *GetGeometryChunk()									{ return m_pGeometryChunk; };
	inline D3D11Shaderset* GetShaderset()											{ return m_pShaderset; };
	inline std::vector< std::pair< std::string, DX11Texture2D* >, tbb::scalable_allocator<std::pair< std::string, DX11Texture2D* >> > *GetTextures()	{ return &m_pTextures; };

	void Execute( RenderDispatcher* pDispatcher );

	void Clone( RenderCommand* ptr );

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

	inline void SetResource( ID3D11Resource* ptr, void* pData, UINT dataSize )	{ m_pResource = ptr; m_pData = pData; m_DataSize = dataSize; };

	void Execute( RenderDispatcher* pDispatcher );

	void Clone( RenderCommand* ptr );
protected:
	ID3D11Resource*		m_pResource;
	void*	m_pData;
	UINT	m_DataSize;
};

class DX11RenderCommand_CopySubresourceRegion : public RenderCommand
{
public:
	DX11RenderCommand_CopySubresourceRegion() : m_pSource(NULL), m_pDestination(NULL), m_SourceSubresourceIndex(0), m_DestinationSubresourceIndex(0) {};

	void SetSource( ID3D11Resource*	source, UINT subresourceIndex )				{ m_pSource = source; m_SourceSubresourceIndex = subresourceIndex; };
	void SetDestination( ID3D11Resource* destination, UINT subresourceIndex )	{ m_pDestination = destination; m_DestinationSubresourceIndex = subresourceIndex; };

	void Execute( RenderDispatcher* pDispatcher );

	void Clone( RenderCommand* ptr );

protected:
	ID3D11Resource*		m_pSource;
	UINT				m_SourceSubresourceIndex;
	ID3D11Resource*		m_pDestination;
	UINT				m_DestinationSubresourceIndex;
};