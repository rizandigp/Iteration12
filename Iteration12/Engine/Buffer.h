#pragma once

#include "Prerequisites.h"

#include <d3d11.h>
#include <d3dx11.h>
#include <d3dcompiler.h>
#include <xnamath.h>
#include <vector>
#include "Vector3.h"
#include "Vector2.h"


struct Vertex
{
    Vector3 Pos;
    Vector2 Tex;
	Vector3	Normal;
	Vector3 Tangent;
	Vector3 Bitangent;
};


struct VertexElement
{
	LPCSTR SemanticName;
    UINT SemanticIndex;
    DXGI_FORMAT Format;
};


class BufferLayout
{
public:
	BufferLayout()	: m_NumOfElements(0), m_ByteWidth(0) {}; 

	void AddElement( VertexElement vertexElement )
	{ 
		m_VertexElements.push_back( vertexElement ); 
		m_NumOfElements += 1; 
	};

	void AddElement( LPCSTR SemanticName,
					UINT SemanticIndex,
					DXGI_FORMAT Format )
	{
		VertexElement vertexElement;
		vertexElement.SemanticName = SemanticName;
		vertexElement.SemanticIndex = SemanticIndex;
		vertexElement.Format = Format;
		m_VertexElements.push_back( vertexElement ); 
		m_NumOfElements += 1; 
		if ( (Format >= 1)&&(Format <= 4) )
			m_ByteWidth += 16;
		else if ( (Format >= 5)&&(Format <= 8) )
			m_ByteWidth += 12;
		else if ( (Format >= 9)&&(Format <= 22) )
			m_ByteWidth += 8;
		else if ( (Format >= 23)&&(Format <= 47) )
			m_ByteWidth += 4;
		else 
			m_ByteWidth += 2;
	};

	void Clear()						{ m_VertexElements.clear(); m_NumOfElements = 0; m_ByteWidth = 0; };

	inline VertexElement* GetElements()	{ return &m_VertexElements[0]; };
	inline UINT GetNumberOfElements() 	{ return m_NumOfElements; };
	inline UINT GetByteSize() 			{ return m_ByteWidth; };

protected:
	std::vector<VertexElement>	m_VertexElements;
	UINT	m_NumOfElements;
	UINT    m_ByteWidth;
};


// Base
class VertexBuffer
{
public:
	void SetBufferLayout( const BufferLayout &layout )		{ m_Layout = layout; };
	inline BufferLayout* GetBufferLayout()					{ return &m_Layout; };

protected:
	BufferLayout	m_Layout;
};

class IndexBuffer
{
public:
	//virtual void bind()=0;
	//virtual void unbind()=0;
};


//
// DX10
//
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



//
// DX11
//

class D3D11VertexBuffer : public VertexBuffer
{
public:
	D3D11VertexBuffer()  : m_pd3d11Buffer(NULL), m_BufferSize(0), m_Stride(0) {};
	D3D11VertexBuffer( ID3D11Buffer* DX11Buffer, BufferLayout Layout )	{ m_pd3d11Buffer = DX11Buffer; SetBufferLayout(Layout); };

	void SetBuffer( ID3D11Buffer* DX11Buffer )			{ m_pd3d11Buffer = DX11Buffer; };
	void SetStride( UINT stride )						{ m_Stride = stride; };

	inline ID3D11Buffer* GetBuffer()					{ return m_pd3d11Buffer; };
	inline UINT GetStride()								{ return m_Stride; };

private:
	ID3D11Buffer*	m_pd3d11Buffer;
	UINT			m_BufferSize;
	UINT			m_Stride;
};

class D3D11IndexBuffer : public IndexBuffer
{
public:
	D3D11IndexBuffer()	: m_pd3d11Buffer(NULL), m_NumIndices(0)	{};
	D3D11IndexBuffer( ID3D11Buffer* DX11Buffer )	{ SetBuffer(DX11Buffer); }; 

	void SetBuffer( ID3D11Buffer* DX11Buffer )		{ m_pd3d11Buffer = DX11Buffer; D3D11_BUFFER_DESC desc; DX11Buffer->GetDesc( &desc ); m_NumIndices = desc.ByteWidth/sizeof(WORD); };
	void SetNumberOfIndices( UINT n )				{ m_NumIndices = n; };

	inline ID3D11Buffer* GetBuffer()				{ return m_pd3d11Buffer; };
	inline UINT GetNumberOfIndices()				{ return m_NumIndices; };
private:
	ID3D11Buffer*	m_pd3d11Buffer;
	UINT			m_NumIndices;
};

