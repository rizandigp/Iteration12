#pragma once

#include "Prerequisites.h"

#include <map>
#include "Buffer.h"
#include "xnaCollision.h"
#include "GPUResource.h"

class GeometryChunk : public GPUResource
{
public:
	virtual void bind( RenderDispatcher* pDispatcher )=0;
	virtual void unbind( RenderDispatcher* pDispatcher )=0;

	void setRenderSystem( RenderSystem* ptr )		{ m_pRenderSystem = ptr; };		
	void setAABB( XNA::AxisAlignedBox* pAABB )		{ m_AABB = *pAABB; };
	inline XNA::AxisAlignedBox* getAABB()			{ return &m_AABB; };
	inline void setDynamic( bool dyn )				{ m_bDynamic = dyn; };
	inline bool isDynamic()							{ return m_bDynamic; };
protected:
	RenderSystem* m_pRenderSystem;
	XNA::AxisAlignedBox m_AABB;
	bool m_bDynamic;
};

class D3D11GeometryChunk : public GeometryChunk
{
public:
	D3D11GeometryChunk() : m_pIndexBuffer(NULL)	{ m_bDynamic = false; };
	void updateVertexBuffer( float* vertices, UINT vertexCount, UINT byteSize, UINT stride );
	void addVertexBuffer( D3D11VertexBuffer* pVertexBuffer )		{ m_pVertexBuffers.push_back( pVertexBuffer ); };
	void setIndexBuffer( D3D11IndexBuffer* pIndexBuffer )			{ m_pIndexBuffer = pIndexBuffer; };
	void setPrimitiveTopology( D3D_PRIMITIVE_TOPOLOGY topology )	{ m_PrimitiveTopology = topology; };
	void setCompatibleInputLayout( D3D11Shaderset* pShaderset, ID3D11InputLayout* pInputLayout )	{ m_pInputLayouts[pShaderset] = pInputLayout; };

	inline D3D11VertexBuffer* getVertexBuffer( UINT index )			{ return m_pVertexBuffers[index]; };
	inline D3D11IndexBuffer* getIndexBuffer()						{ return m_pIndexBuffer; };
	inline int getNumberOfVertexBuffers() const						{ return m_pVertexBuffers.size();};
	inline int getNumberOfVerts() const								{ return m_pIndexBuffer->getNumberOfIndices(); };
	inline D3D_PRIMITIVE_TOPOLOGY getPrimitiveTopology()			{ return m_PrimitiveTopology; };
	inline ID3D11InputLayout*	getCompatibleInputLayout( D3D11Shaderset* pShaderset );

	void bind( RenderDispatcher* pDispatcher );
	void unbind( RenderDispatcher* pDispatcher ){};

private:
	ID3D11InputLayout* CreateInputLayoutFromBufferLayouts( ID3D11Device* pDevice, BufferLayout* BufferLayouts[], int numberOfBuffers, const void *pShaderBytecodeWithInputSignature, SIZE_T BytecodeLength );
	HRESULT CreateInputLayoutDescFromVertexShaderSignature( ID3DBlob* pShaderBlob, ID3D11Device* pD3DDevice, ID3D11InputLayout** pInputLayout );

protected:
	std::vector<D3D11VertexBuffer*>	m_pVertexBuffers;
	D3D11IndexBuffer*	m_pIndexBuffer;
	std::map< D3D11Shaderset*, ID3D11InputLayout* >	m_pInputLayouts;
	D3D_PRIMITIVE_TOPOLOGY m_PrimitiveTopology;
};

inline ID3D11InputLayout*	D3D11GeometryChunk::getCompatibleInputLayout( D3D11Shaderset* pShaderset )	
{	
	std::map< D3D11Shaderset*, ID3D11InputLayout* >::iterator it = m_pInputLayouts.find( pShaderset );
	if (it!=m_pInputLayouts.end())
		return (*it).second;
	else return NULL;
}