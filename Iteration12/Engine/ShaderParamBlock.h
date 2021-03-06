#pragma once

#include "Prerequisites.h"
#include <xnamath.h>

// Container for shader parameters
class ShaderParamBlock
{
public:
	// ((paramName,index),value)
	typedef std::map< std::pair< std::string, UINT > , XMFLOAT4X4, std::less<std::pair< std::string, UINT >> , tbb::scalable_allocator<std::pair<std::pair< std::string, UINT >,XMFLOAT4X4>> > MatrixShaderParamList;
	typedef std::map< std::pair< std::string, UINT > , XMFLOAT4 , std::less<std::pair< std::string, UINT >>, tbb::scalable_allocator<std::pair<std::pair< std::string, UINT >,XMFLOAT4>> > VectorShaderParamList;
	typedef std::map< std::pair< std::string, UINT > , float , std::less<std::pair< std::string, UINT >>, tbb::scalable_allocator<std::pair<std::pair< std::string, UINT >,float>> > ScalarShaderParamList;

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
	
	inline MatrixShaderParamList*	GetMatrixParams()	{ return &m_MatrixParams; };
	inline VectorShaderParamList*	getVectorParams()	{ return &m_VectorParams; };
	inline ScalarShaderParamList*	getScalarParams()	{ return &m_ScalarParams; };

	void clear();
	void clearMatrices();
	void clearVectors();
	void clearScalars();

private:	
	MatrixShaderParamList m_MatrixParams;
	VectorShaderParamList m_VectorParams;
	ScalarShaderParamList m_ScalarParams;
};