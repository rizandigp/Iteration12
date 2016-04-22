#pragma once

#include "Prerequisites.h"

#include <d3d11.h>
#include <d3dx11.h>
#include <xnamath.h>
#include <map>
#include <unordered_map>
#include <string>


class ShaderParamBlock
{
public:
	void assign( std::string paramName, const XMMATRIX* pMatrixParam );	
	void assign( std::string paramName, const XMFLOAT4X4* pMatrixParam );
	void assign( std::string paramName, const XMFLOAT4* pVectorParam );
	void assign( std::string paramName, const Vector4* pVectorParam );
	void assign( std::string paramName, float pScalarParam );
	void assign( std::string paramName, int pScalarParam );
	void assign( std::string paramName, UINT index, const Matrix4x4* matrix );
	void assign( std::string paramName, UINT index, const XMFLOAT4X4* pMatrixParam );
	void assign( std::string paramName, UINT index, const XMMATRIX* pMatrixParam );
	void assign( std::string paramName, UINT index, const XMFLOAT4* pVectorParam );
	void assign( std::string paramName, UINT index, const Vector4* pVectorParam );
	void assign( std::string paramName, UINT index, float pScalarParam );
	void assign( std::string paramName, UINT index, int pScalarParam );

	void assign( ShaderParamBlock paramBlock );
	
	inline std::map< std::pair< std::string, UINT > , XMFLOAT4X4 , std::less<std::pair< std::string, UINT >> , tbb::scalable_allocator<std::pair<std::pair< std::string, UINT >,XMFLOAT4X4>> >*	GetMatrixParams()	{ return &m_MatrixParams; };
	inline std::map< std::pair< std::string, UINT > , XMFLOAT4 , std::less<std::pair< std::string, UINT >> , tbb::scalable_allocator<std::pair<std::pair< std::string, UINT >,XMFLOAT4>> >*	getVectorParams()	{ return &m_VectorParams; };
	inline std::map< std::pair< std::string, UINT > , float , std::less<std::pair< std::string, UINT >>, tbb::scalable_allocator<std::pair<std::pair< std::string, UINT >,float>> >*		getScalarParams()	{ return &m_ScalarParams; };

	void clear();
	void clearMatrices();
	void clearVectors();
	void clearScalars();

private:
	// ((paramName,index),value)
	std::map< std::pair< std::string, UINT > , XMFLOAT4X4, std::less<std::pair< std::string, UINT >> , tbb::scalable_allocator<std::pair<std::pair< std::string, UINT >,XMFLOAT4X4>> > m_MatrixParams;
	std::map< std::pair< std::string, UINT > , XMFLOAT4 , std::less<std::pair< std::string, UINT >>, tbb::scalable_allocator<std::pair<std::pair< std::string, UINT >,XMFLOAT4>> > m_VectorParams;
	std::map< std::pair< std::string, UINT > , float , std::less<std::pair< std::string, UINT >>, tbb::scalable_allocator<std::pair<std::pair< std::string, UINT >,float>> > m_ScalarParams;
};