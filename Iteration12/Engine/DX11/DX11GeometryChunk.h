#pragma once

#include "..\GeometryChunk.h"
#include "DX11Buffer.h"
#include <d3d11.h>
#include <d3dx11.h>
#include <d3dcompiler.h>


// DX11 implementation of GeometryChunk
class DX11GeometryChunk : public GeometryChunk
{
public:
	DX11GeometryChunk() : m_pIndexBuffer(NULL)	{ m_bDynamic = false; };
	void UpdateVertexBuffer( float* vertices, UINT vertexCount, UINT byteSize, UINT stride );
	void AddVertexBuffer( DX11VertexBuffer* pVertexBuffer )			{ m_pVertexBuffers.push_back( pVertexBuffer ); };
	void SetIndexBuffer( DX11IndexBuffer* pIndexBuffer )			{ m_pIndexBuffer = pIndexBuffer; };
	void SetCompatibleInputLayout( DX11Shaderset* pShaderset, ID3D11InputLayout* pInputLayout )	{ m_pInputLayouts[pShaderset] = pInputLayout; };

	inline DX11VertexBuffer* GetVertexBuffer( UINT index )			{ return m_pVertexBuffers[index]; };
	inline DX11IndexBuffer* GetIndexBuffer()						{ return m_pIndexBuffer; };
	inline int GetNumberOfVertexBuffers() const						{ return m_pVertexBuffers.size();};
	inline int GetNumberOfVertices() const							{ return m_pIndexBuffer->GetNumberOfIndices(); };
	inline ID3D11InputLayout*	GetCompatibleInputLayout( DX11Shaderset* pShaderset );

	void Bind( RenderDispatcher* pDispatcher );
	void Unbind( RenderDispatcher* pDispatcher ){};

private:
	ID3D11InputLayout* CreateInputLayoutFromBufferLayouts( ID3D11Device* pDevice, BufferLayout* BufferLayouts[], int numberOfBuffers, const void *pShaderBytecodeWithInputSignature, SIZE_T BytecodeLength );
	HRESULT CreateInputLayoutDescFromVertexShaderSignature( ID3DBlob* pShaderBlob, ID3D11Device* pD3DDevice, ID3D11InputLayout** pInputLayout );

protected:
	std::vector<DX11VertexBuffer*>	m_pVertexBuffers;
	DX11IndexBuffer*	m_pIndexBuffer;
	std::map< DX11Shaderset*, ID3D11InputLayout* >	m_pInputLayouts;
};

inline ID3D11InputLayout*	DX11GeometryChunk::GetCompatibleInputLayout( DX11Shaderset* pShaderset )	
{	
	std::map< DX11Shaderset*, ID3D11InputLayout* >::iterator it = m_pInputLayouts.find( pShaderset );
	if (it!=m_pInputLayouts.end())
		return (*it).second;
	else return NULL;
}