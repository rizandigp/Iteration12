#pragma once

#include "..\Buffer.h"


class D3D10VertexBuffer : public VertexBuffer
{
public:
	D3D10VertexBuffer()	: m_pBuffer(NULL), m_BufferSize(0), m_Stride(0)	{};
	D3D10VertexBuffer( ID3D10Buffer* DX10Buffer, BufferLayout Layout )	{ m_pBuffer = DX10Buffer; m_Layout = Layout;	};

	void SetBuffer( ID3D10Buffer* pBuffer )			{ m_pBuffer = pBuffer; };
	void SetStride( UINT stride )					{ m_Stride = stride; };

	inline ID3D10Buffer* GetBuffer() const			{ return m_pBuffer; };
	inline UINT GetStride() const					{ return m_Stride; };

private:
	ID3D10Buffer*	m_pBuffer;
	UINT			m_BufferSize;
	UINT			m_Stride;
};


class D3D10IndexBuffer : public IndexBuffer
{
public:
	D3D10IndexBuffer()	: m_pBuffer(NULL), m_NumOfIndices(0)	{};
	D3D10IndexBuffer( ID3D10Buffer* DX10Buffer )	{ SetBuffer(DX10Buffer); D3D10_BUFFER_DESC desc; DX10Buffer->GetDesc( &desc ); m_NumOfIndices = desc.ByteWidth/sizeof(DWORD); }; 

	void SetBuffer( ID3D10Buffer* DX10Buffer )		{ m_pBuffer = DX10Buffer; };

	inline ID3D10Buffer* GetBuffer() const			{ return m_pBuffer; };
	inline UINT GetNumberOfIndices() const			{ return m_NumOfIndices; };

private:
	ID3D10Buffer*	m_pBuffer;
	UINT			m_NumOfIndices;
};