#pragma once

#include "Prerequisites.h"
#include "Shaderset.h"
#include "ConstantBufferData.h"
#include "ShaderParamBlock.h"

// Stores shader parameters for a Shaderset in ConstantBufferData's
class ShaderParams
{
public:
	void initialize( Shaderset* shader );

	// TODO :	-different matrix types
	//			-proper function names
	void setParam( const std::string& paramName, const Matrix4x4* matrix );
	void setParam( const std::string& paramName, const Vector4* vector );
	void setParam( const std::string& paramName, const XMMATRIX* pMatrixParam );		
	void setParam( const std::string& paramName, const XMFLOAT4X4* pMatrixParam );
	void setParam( const std::string& paramName, const XMFLOAT4* pVectorParam );
	void setParam( const std::string& paramName, float pScalarParam );
	void setParam( const std::string& paramName, int pScalarParam );
	void setParam( const std::string& paramName, UINT index, const Matrix4x4* matrix );
	void setParam( const std::string& paramName, UINT index, const XMMATRIX* pMatrixParam );
	void setParam( const std::string& paramName, UINT index, const XMFLOAT4X4* pMatrixParam );
	void setParam( const std::string& paramName, UINT index, const XMFLOAT4* pVectorParam );
	void setParam( const std::string& paramName, UINT index, const Vector4* vector );
	void setParam( const std::string& paramName, UINT index, float pScalarParam );
	void setParam( const std::string& paramName, UINT index, int pScalarParam );

	void assign( ShaderParamBlock* pParamBlock );
	void clear();

	UINT getNumConstantBuffersVS()							{ return m_VSConstantBuffers.size(); };
	UINT getNumConstantBuffersPS()							{ return m_PSConstantBuffers.size(); };
	ConstantBufferData* getVSConstantBuffer(UINT index)		{ return &m_VSConstantBuffers.at(index); };
	ConstantBufferData* getPSConstantBuffer(UINT index)		{ return &m_PSConstantBuffers.at(index); };

private:
	Shaderset* m_pShaderset;
	std::vector<ConstantBufferData,tbb::scalable_allocator<ConstantBufferData>> m_VSConstantBuffers;
	std::vector<ConstantBufferData,tbb::scalable_allocator<ConstantBufferData>> m_PSConstantBuffers;
};