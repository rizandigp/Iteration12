#pragma once

#include "..\RenderCommand.h"
#include "..\ShaderParams.h"
#include "..\RenderState.h"
#include "..\BlendState.h"
#include "..\Texture.h"
#include "..\Vector4.h"
#include <d3d11.h>
#include <d3dx11.h>


// Generic drawing
// New DX11 class
class DX11RenderCommand_Draw : public RenderCommand
{
public:
	DX11RenderCommand_Draw()	: m_RenderState(), m_BlendState(), m_StencilRef(0)		{};

	// MUTATORS
	inline void SetGeometryChunk( DX11GeometryChunk *pGeometryChunk )					{ m_pGeometryChunk = pGeometryChunk; };
	inline void SetShaderset( DX11Shaderset *pShaderset )								{ m_pShaderset = pShaderset;};
	inline void ClearTextures()															{ m_pTextures.clear(); };
	inline void SetShaderParams( ShaderParams* pParams )								{ m_ShaderParams = *pParams; };
	void SetTexture( const std::string& name, Texture* pTexture );//					{ m_pTextures.push_back( std::pair< std::string, DX11Texture2D* > (name,pTexture) );};

	//inline void SetBlendState( ID3D11BlendState* pBlendState )							{ m_pBlendState = pBlendState; };
	//inline void SetDepthStencilState( ID3D11DepthStencilState* pDepthStencilState )		{ m_pDepthStencilState = pDepthStencilState; };
	//inline void SetRasterizerState( ID3D11RasterizerState* pRasterizerState )			{ m_pRasterizerState = pRasterizerState; };

	void SetBlendState( const BlendState& blendState )		{ m_BlendState = blendState; };
	void SetRenderState( const RenderState& renderState )	{ m_RenderState = renderState; };
	void SetStencilRef( UINT StencilRef )					{ m_StencilRef = StencilRef; };

	// ACCESSORS
	inline DX11GeometryChunk *GetGeometryChunk()									{ return m_pGeometryChunk; };
	inline DX11Shaderset* GetShaderset()											{ return m_pShaderset; };
	inline std::vector< std::pair< std::string, Texture* >, tbb::scalable_allocator<std::pair< std::string, Texture* >> > *GetTextures()	{ return &m_pTextures; };

	void Execute( RenderDispatcher* pDispatcher );

	void Clone( RenderCommand* ptr );

protected:
	DX11Shaderset		*m_pShaderset;
	DX11GeometryChunk	*m_pGeometryChunk;
	std::vector< std::pair< std::string, Texture* >, tbb::scalable_allocator<std::pair< std::string, Texture* >> > m_pTextures;
	//ShaderParamBlock	m_ShaderParamBlock;
	ShaderParams		m_ShaderParams;
	// DX11 state objects
	//ID3D11BlendState*	m_pBlendState;
	//ID3D11DepthStencilState*	m_pDepthStencilState;
	//ID3D11RasterizerState*	m_pRasterizerState;
	RenderState		m_RenderState;
	BlendState		m_BlendState;
	UINT			m_StencilRef;
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

// Generic drawing
// Slow, use the new class instead
class D3D11RenderCommand_Draw : public RenderCommand
{
public:
	D3D11RenderCommand_Draw()	: m_pRasterizerState( NULL ), m_pDepthStencilState(NULL), m_pBlendState( NULL )		{};

	// MUTATORS
	inline void SetGeometryChunk( DX11GeometryChunk *pGeometryChunk )			{ m_pGeometryChunk = pGeometryChunk; };
	inline void SetShaderset( DX11Shaderset *pShaderset )						{ m_pShaderset = pShaderset;};
	inline void ClearTextures()													{ m_pTextures.clear(); };
	void SetTexture( std::string name, DX11Texture2D *pTexture );				//{ m_pTextures.push_back( std::pair< std::string, DX11Texture2D* > (name,pTexture) );};

	inline void SetBlendState( ID3D11BlendState* pBlendState )							{ m_pBlendState = pBlendState; };
	inline void SetDepthStencilState( ID3D11DepthStencilState* pDepthStencilState )		{ m_pDepthStencilState = pDepthStencilState; };
	inline void SetRasterizerState( ID3D11RasterizerState* pRasterizerState )			{ m_pRasterizerState = pRasterizerState; };


	// ACCESSORS
	inline DX11GeometryChunk *GetGeometryChunk()									{ return m_pGeometryChunk; };
	inline DX11Shaderset *GetShaderset()											{ return m_pShaderset; };
	inline ShaderParamBlock* shaderParams()											{ return &m_ShaderParamBlock; };
	inline std::vector< std::pair< std::string, DX11Texture2D* >, tbb::scalable_allocator<std::pair< std::string, DX11Texture2D* >> > *GetTextures()	{ return &m_pTextures; };

	void Execute( RenderDispatcher* pDispatcher );

	void Clone( RenderCommand* ptr );

protected:
	DX11Shaderset *m_pShaderset;
	DX11GeometryChunk *m_pGeometryChunk;
	std::vector< std::pair< std::string, DX11Texture2D* >, tbb::scalable_allocator<std::pair< std::string, DX11Texture2D* >> > m_pTextures;
	ShaderParamBlock m_ShaderParamBlock;
	// DX11 state objects
	ID3D11BlendState*	m_pBlendState;
	ID3D11DepthStencilState* m_pDepthStencilState;
	ID3D11RasterizerState*	m_pRasterizerState;
};

// For changing render targets
class D3D11RenderCommand_RenderTarget : public RenderCommand
{
public:
	D3D11RenderCommand_RenderTarget()	: m_pRenderTargetTexture(NULL)	{};

	void SetRenderTargetTexture( DX11Texture2D *pTexture )						{ m_pRenderTargetTexture = pTexture; };
	void SetRenderViewport( RenderViewport vp )									{ m_RenderViewport = vp; };

	DX11Texture2D *GetRenderTargetTexture()									{ return m_pRenderTargetTexture; };
	RenderViewport *GetRenderViewport()											{ return &m_RenderViewport; };

	void Execute( RenderDispatcher* pDispatcher );

	void Clone( RenderCommand* ptr );

protected:
	DX11Texture2D *m_pRenderTargetTexture;
	RenderViewport m_RenderViewport;
};

// Multiple Render Targets
class D3D11RenderCommand_MRT : public RenderCommand
{
public:
	D3D11RenderCommand_MRT()	: m_pRenderTargetTextureArray(NULL)	{};

	void SetRenderTargetTextures( UINT numRenderTargets, DX11Texture2D** pTextureArray );
	void SetRenderViewport( RenderViewport vp )									{ m_RenderViewport = vp; };

	DX11Texture2D **GetRenderTargetTextureArray()								{ return &m_pRenderTargetTextureArray[0]; };
	RenderViewport *GetRenderViewport()											{ return &m_RenderViewport; };

	void Execute( RenderDispatcher* pDispatcher );

	void Clone( RenderCommand* ptr );

protected:
	UINT m_numRenderTargets;
	std::vector<DX11Texture2D*> m_pRenderTargetTextureArray;
	RenderViewport m_RenderViewport;
};

// Resolve an msaa texture
class D3D11RenderCommand_ResolveMSAA : public RenderCommand
{
public:
	D3D11RenderCommand_ResolveMSAA()	{};

	void SetDestination( Texture2D* ptr )	{ m_pDestination = ptr; };
	void SetSource( Texture2D* ptr )		{ m_pSource = ptr; };

	Texture2D* GetDestination()		{ return m_pDestination; };
	Texture2D* GetSource()			{ return m_pSource; };

	void Execute( RenderDispatcher* pDispatcher );

	void Clone( RenderCommand* ptr );

protected:
	Texture2D* m_pDestination;
	Texture2D* m_pSource;
};

class D3D11RenderCommand_ClearTexture : public RenderCommand
{
public:
	D3D11RenderCommand_ClearTexture()	: m_pTexture(NULL)	{};

	void SetTexture( Texture *pTexture )				{ m_pTexture = pTexture; };
	void SetClearColor( Vector4 clearColorRGBA )		{ m_ClearColor = clearColorRGBA; };
	void SetClearFlags( UINT clearFlags )				{ m_ClearFlags = clearFlags; };

	Texture* GetTexture()							{ return m_pTexture; };

	void Execute( RenderDispatcher* pDispatcher );

	void Clone( RenderCommand* ptr );

protected:
	Texture *m_pTexture;
	Vector4 m_ClearColor;
	UINT m_ClearFlags;
};

class D3D11RenderCommand_EndFrame : public RenderCommand
{
public:
	void Execute( RenderDispatcher* pDispatcher );

	void Clone( RenderCommand* ptr );
};

class D3D11RenderCommand_BeginFrame : public RenderCommand
{
public:
	void SetClearColor( float* clearColorRGBA )					{ m_ClearColor[0] = clearColorRGBA[0]; m_ClearColor[1] = clearColorRGBA[1]; m_ClearColor[2] = clearColorRGBA[2]; m_ClearColor[3] = clearColorRGBA[3]; };
	void SetDepthStencilClear( float depth, UINT8 stencil )		{ m_Depth = depth; m_Stencil = stencil; };

	void Execute( RenderDispatcher* pDispatcher );

	void Clone( RenderCommand* ptr );

protected:
	float m_ClearColor[4];
	float m_Depth;
	UINT8 m_Stencil;
};

// For setting shared constant buffers
class D3D11RenderCommand_SharedCB : public RenderCommand
{
public:
	inline ShaderParamBlock* shaderParams()											{ return &m_ShaderParamBlock; };

	//void execute( RenderDispatcher* pDispatcher );

protected:
	ShaderParamBlock m_ShaderParamBlock;
};