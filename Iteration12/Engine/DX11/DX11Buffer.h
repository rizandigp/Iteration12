#pragma once

#include "..\Buffer.h"
#include <d3d11.h>
#include <d3dx11.h>
#include <d3dcompiler.h>

class DX11VertexBuffer : public VertexBuffer
{
public:
	DX11VertexBuffer()  : m_pd3d11Buffer(NULL), m_BufferSize(0), m_Stride(0) {};
	DX11VertexBuffer( ID3D11Buffer* DX11Buffer, BufferLayout Layout )	{ m_pd3d11Buffer = DX11Buffer; SetBufferLayout(Layout); };

	void SetBuffer( ID3D11Buffer* DX11Buffer )			{ m_pd3d11Buffer = DX11Buffer; };
	void SetStride( UINT stride )						{ m_Stride = stride; };

	inline ID3D11Buffer* GetBuffer()					{ return m_pd3d11Buffer; };
	inline UINT GetStride()								{ return m_Stride; };

private:
	ID3D11Buffer*	m_pd3d11Buffer;
	UINT			m_BufferSize;
	UINT			m_Stride;
};

class DX11IndexBuffer : public IndexBuffer
{
public:
	DX11IndexBuffer()	: m_pd3d11Buffer(NULL), m_NumIndices(0)	{};
	DX11IndexBuffer( ID3D11Buffer* DX11Buffer )	{ SetBuffer(DX11Buffer); }; 

	void SetBuffer( ID3D11Buffer* DX11Buffer )		{ m_pd3d11Buffer = DX11Buffer; D3D11_BUFFER_DESC desc; DX11Buffer->GetDesc( &desc ); m_NumIndices = desc.ByteWidth/sizeof(WORD); };
	void SetNumberOfIndices( UINT n )				{ m_NumIndices = n; };

	inline ID3D11Buffer* GetBuffer()				{ return m_pd3d11Buffer; };
	inline UINT GetNumberOfIndices()				{ return m_NumIndices; };
private:
	ID3D11Buffer*	m_pd3d11Buffer;
	UINT			m_NumIndices;
};

