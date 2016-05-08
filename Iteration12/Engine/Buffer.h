#pragma once

#include "Prerequisites.h"
#include "Vector2.h"
#include "Vector3.h"
#include "Texture.h"


typedef TEXTURE_FORMAT VERTEX_ELEMENT_FORMAT;
//enum VERTEX_ELEMENT_FORMAT;

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
    VERTEX_ELEMENT_FORMAT Format;
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
					VERTEX_ELEMENT_FORMAT Format )
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