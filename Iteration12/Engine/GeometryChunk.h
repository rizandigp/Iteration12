#pragma once

#include "Prerequisites.h"

#include <map>
#include "Buffer.h"
#include "xnaCollision.h"
#include "GPUResource.h"

class GeometryChunk : public GPUResource
{
public:
	virtual void Bind( RenderDispatcher* pDispatcher )=0;
	virtual void Unbind( RenderDispatcher* pDispatcher )=0;

	void SetRenderSystem( RenderSystem* ptr )		{ m_pRenderSystem = ptr; };		
	void SetAABB( XNA::AxisAlignedBox* pAABB )		{ m_AABB = *pAABB; };
	inline XNA::AxisAlignedBox* GetAABB()			{ return &m_AABB; };
	inline void SetDynamic( bool dyn )				{ m_bDynamic = dyn; };
	inline bool IsDynamic()							{ return m_bDynamic; };
protected:
	RenderSystem* m_pRenderSystem;
	XNA::AxisAlignedBox m_AABB;
	bool m_bDynamic;
};

class D3D11GeometryChunk : public GeometryChunk
{
public:
	D3D11GeometryChunk() : m_pIndexBuffer(NULL)	{ m_bDynamic = false; };
	void UpdateVertexBuffer( float* vertices, UINT vertexCount, UINT byteSize, UINT stride );
	void AddVertexBuffer( D3D11VertexBuffer* pVertexBuffer )		{ m_pVertexBuffers.push_back( pVertexBuffer ); };
	void SetIndexBuffer( D3D11IndexBuffer* pIndexBuffer )			{ m_pIndexBuffer = pIndexBuffer; };
	void SetPrimitiveTopology( D3D_PRIMITIVE_TOPOLOGY topology )	{ m_PrimitiveTopology = topology; };
	void SetCompatibleInputLayout( D3D11Shaderset* pShaderset, ID3D11InputLayout* pInputLayout )	{ m_pInputLayouts[pShaderset] = pInputLayout; };

	inline D3D11VertexBuffer* GetVertexBuffer( UINT index )			{ return m_pVertexBuffers[index]; };
	inline D3D11IndexBuffer* GetIndexBuffer()						{ return m_pIndexBuffer; };
	inline int GetNumberOfVertexBuffers() const						{ return m_pVertexBuffers.size();};
	inline int GetNumberOfVerts() const								{ return m_pIndexBuffer->GetNumberOfIndices(); };
	inline D3D_PRIMITIVE_TOPOLOGY GetPrimitiveTopology()			{ return m_PrimitiveTopology; };
	inline ID3D11InputLayout*	GetCompatibleInputLayout( D3D11Shaderset* pShaderset );

	void Bind( RenderDispatcher* pDispatcher );
	void Unbind( RenderDispatcher* pDispatcher ){};

private:
	ID3D11InputLayout* CreateInputLayoutFromBufferLayouts( ID3D11Device* pDevice, BufferLayout* BufferLayouts[], int numberOfBuffers, const void *pShaderBytecodeWithInputSignature, SIZE_T BytecodeLength );
	HRESULT CreateInputLayoutDescFromVertexShaderSignature( ID3DBlob* pShaderBlob, ID3D11Device* pD3DDevice, ID3D11InputLayout** pInputLayout );

protected:
	std::vector<D3D11VertexBuffer*>	m_pVertexBuffers;
	D3D11IndexBuffer*	m_pIndexBuffer;
	std::map< D3D11Shaderset*, ID3D11InputLayout* >	m_pInputLayouts;
	D3D_PRIMITIVE_TOPOLOGY m_PrimitiveTopology;
};

inline ID3D11InputLayout*	D3D11GeometryChunk::GetCompatibleInputLayout( D3D11Shaderset* pShaderset )	
{	
	std::map< D3D11Shaderset*, ID3D11InputLayout* >::iterator it = m_pInputLayouts.find( pShaderset );
	if (it!=m_pInputLayouts.end())
		return (*it).second;
	else return NULL;
}