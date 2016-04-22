#pragma once
#include "Prerequisites.h"

#include "ShaderParamBlock.h"
#include "Vector4.h"


void ShaderParamBlock::assign( std::string paramName, const XMMATRIX* pMatrixParam )
{
	XMFLOAT4X4 matrix;
	XMStoreFloat4x4( &matrix, *pMatrixParam );
	m_MatrixParams[std::pair< std::string, UINT > (paramName,0)] = matrix;
}

void ShaderParamBlock::assign( std::string paramName, UINT index, const XMMATRIX* pMatrixParam )
{
	XMFLOAT4X4 matrix;
	XMStoreFloat4x4( &matrix, *pMatrixParam );
	m_MatrixParams[std::pair< std::string, UINT > (paramName,index)] = matrix;
}

void ShaderParamBlock::assign( std::string paramName, const XMFLOAT4X4* pMatrixParam )
{
	m_MatrixParams[std::pair< std::string, UINT > (paramName,0)] = *pMatrixParam;
}

void ShaderParamBlock::assign( std::string paramName, UINT index, const Matrix4x4* matrix )
{
	m_MatrixParams[std::pair< std::string, UINT > (paramName,index)] = *(XMFLOAT4X4*)matrix;
}

void ShaderParamBlock::assign( std::string paramName, UINT index, const XMFLOAT4X4* pMatrixParam )
{
	m_MatrixParams[std::pair< std::string, UINT > (paramName,index)] = *pMatrixParam;
}

void ShaderParamBlock::assign( std::string paramName, const XMFLOAT4* pVectorParam )
{
	m_VectorParams[std::pair< std::string, UINT > (paramName,0)] = *pVectorParam;
}

void ShaderParamBlock::assign( std::string paramName, const Vector4* pVectorParam )
{
	m_VectorParams[std::pair< std::string, UINT > (paramName,0)] = pVectorParam->intoXMFLOAT4();
}

void ShaderParamBlock::assign( std::string paramName, UINT index, const XMFLOAT4* pVectorParam )
{
	m_VectorParams[std::pair< std::string, UINT > (paramName,index)] = *pVectorParam;
}

void ShaderParamBlock::assign( std::string paramName, UINT index, const Vector4* pVectorParam )
{
	m_VectorParams[std::pair< std::string, UINT > (paramName,index)] = pVectorParam->intoXMFLOAT4();
}

void ShaderParamBlock::assign( std::string paramName, float pScalarParam )
{
	m_ScalarParams[std::pair< std::string, UINT > (paramName,0)] = pScalarParam;
}

void ShaderParamBlock::assign( std::string paramName, UINT index, float pScalarParam )
{
	m_ScalarParams[std::pair< std::string, UINT > (paramName,index)] = pScalarParam;
}

void ShaderParamBlock::assign( std::string paramName, int pScalarParam )
{
	m_ScalarParams[std::pair< std::string, UINT > (paramName,0)] = reinterpret_cast<float&>(pScalarParam);
}

void ShaderParamBlock::assign( std::string paramName, UINT index, int pScalarParam )
{
	m_ScalarParams[std::pair< std::string, UINT > (paramName,index)] = reinterpret_cast<float&>(pScalarParam);
}



void ShaderParamBlock::assign( ShaderParamBlock paramBlock )
{
	m_MatrixParams = *paramBlock.GetMatrixParams();
	m_VectorParams = *paramBlock.getVectorParams();
	m_ScalarParams = *paramBlock.getScalarParams();
}

void ShaderParamBlock::clear()
{
	m_MatrixParams.clear();
	m_VectorParams.clear();
	m_ScalarParams.clear();
}