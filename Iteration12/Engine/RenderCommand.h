#pragma once

#include "Prerequisites.h"

#include <vector>
#include <string>
#include <tbb\scalable_allocator.h>
#include "ShaderParamBlock.h"

typedef unsigned long resourceHandle;

struct RenderViewport
{
	int TopLeftX;
    int TopLeftY;
    int Width;
    int Height;
    float MinDepth;
    float MaxDepth;
};

struct RenderCommandBlock
{
protected:
	uint32_t m[32];
};

/*
	Base class
*/
class RenderCommand
{
public:
	// Executes rendering
	virtual void Execute( RenderDispatcher* pDispatcher ) = 0;

	// Clone into pre-allocated ptr
	virtual void Clone( RenderCommand* ptr ) = 0;

	virtual ~RenderCommand()	{};
	/*
	void* operator new(size_t);
	void* operator new(size_t, void*);
	void operator delete(void*);*/

	// Instance should not be modified or submitted if m_InUse is true  
	bool m_InUse;
};

// Generic drawing
// Slow, use the new class instead
class D3D11RenderCommand_Draw : public RenderCommand
{
public:
	D3D11RenderCommand_Draw()	: m_pRasterizerState( NULL ), m_pDepthStencilState(NULL), m_pBlendState( NULL )		{};

	// MUTATORS
	inline void SetGeometryChunk( D3D11GeometryChunk *pGeometryChunk )			{ m_pGeometryChunk = pGeometryChunk; };
	inline void SetShaderset( D3D11Shaderset *pShaderset )						{ m_pShaderset = pShaderset;};
	inline void ClearTextures()													{ m_pTextures.clear(); };
	void SetTexture( std::string name, DX11Texture2D *pTexture );				//{ m_pTextures.push_back( std::pair< std::string, DX11Texture2D* > (name,pTexture) );};

	inline void SetBlendState( ID3D11BlendState* pBlendState )							{ m_pBlendState = pBlendState; };
	inline void SetDepthStencilState( ID3D11DepthStencilState* pDepthStencilState )		{ m_pDepthStencilState = pDepthStencilState; };
	inline void SetRasterizerState( ID3D11RasterizerState* pRasterizerState )			{ m_pRasterizerState = pRasterizerState; };


	// ACCESSORS
	inline D3D11GeometryChunk *GetGeometryChunk()									{ return m_pGeometryChunk; };
	inline D3D11Shaderset *GetShaderset()											{ return m_pShaderset; };
	inline ShaderParamBlock* shaderParams()											{ return &m_ShaderParamBlock; };
	inline std::vector< std::pair< std::string, DX11Texture2D* >, tbb::scalable_allocator<std::pair< std::string, DX11Texture2D* >> > *GetTextures()	{ return &m_pTextures; };

	void Execute( RenderDispatcher* pDispatcher );

	void Clone( RenderCommand* ptr );

protected:
	D3D11Shaderset *m_pShaderset;
	D3D11GeometryChunk *m_pGeometryChunk;
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

	void SetTexture( DX11Texture2D *pTexture )				{ m_pTexture = pTexture; };
	void SetClearColor( float* clearColorRGBA )				{ m_ClearColor[0] = clearColorRGBA[0]; m_ClearColor[1] = clearColorRGBA[1]; m_ClearColor[2] = clearColorRGBA[2]; m_ClearColor[3] = clearColorRGBA[3]; };

	DX11Texture2D *GetTexture()							{ return m_pTexture; };

	void Execute( RenderDispatcher* pDispatcher );

	void Clone( RenderCommand* ptr );

protected:
	DX11Texture2D *m_pTexture;
	float m_ClearColor[4];
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


// OBSOLETE
// The old class ( fat/generic, api-agnostic )
class GenericRenderCommand : public RenderCommand
{
public:
	GenericRenderCommand()	: 	m_pShaderset(NULL), m_pGeometryChunk(NULL), m_pRenderTargetTexture(NULL) {};
	// MUTATORS
	void SetGeometryChunk( GeometryChunk *pGeometryChunk )					{ m_pGeometryChunk = pGeometryChunk; };
	void SetShaderset( Shaderset *pShaderset )								{ m_pShaderset = pShaderset;};
	void SetRenderTargetTexture( Texture2D *pTexture )						{ m_pRenderTargetTexture = pTexture; };
	void AddTexture( std::string name, Texture2D *pTexture )				{ m_pTextures.push_back( std::pair< std::string, Texture2D* > (name,pTexture) );};
	void ClearTextures()													{ m_pTextures.clear(); };
	void SetRenderViewport( RenderViewport vp )								{ m_RenderViewport = vp; };

	// ACCESSORS
	inline GeometryChunk *GetGeometryChunk()									{ return m_pGeometryChunk; };
	inline Shaderset *GetShaderset()											{ return m_pShaderset; };
	inline Texture2D *GetRenderTargetTexture()									{ return m_pRenderTargetTexture; };
	inline ShaderParamBlock* shaderParams()										{ return &m_ShaderParamBlock; };
	inline std::vector< std::pair< std::string, Texture2D* > > *GetTextures()	{ return &m_pTextures; };
	inline RenderViewport *GetRenderViewport()									{ return &m_RenderViewport; };

	void Execute( RenderDispatcher* pDispatcher );

protected:
	Shaderset *m_pShaderset;
	ShaderParamBlock m_ShaderParamBlock;
	GeometryChunk *m_pGeometryChunk;
	Texture2D *m_pRenderTargetTexture;
	std::vector< std::pair< std::string, Texture2D* > > m_pTextures;
	RenderViewport m_RenderViewport;
};