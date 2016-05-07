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
	uint32_t m[40];
};

/*
	Base class
*/
class RenderCommand
{
public:
	virtual ~RenderCommand();

	// Executes rendering
	virtual void Execute( RenderDispatcher* pDispatcher ) = 0;

	// Clone into pre-allocated ptr
	virtual void Clone( RenderCommand* ptr ) = 0;
	/*
	void* operator new(size_t);
	void* operator new(size_t, void*);
	void operator delete(void*);*/

	// Instance should not be modified or submitted if m_InUse is true  
	bool m_InUse;
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