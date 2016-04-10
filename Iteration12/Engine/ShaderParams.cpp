#pragma once

#include "ShaderParams.h"
#include "Vector4.h"

void ShaderParams::initialize( Shaderset* shader )
{
	m_pShaderset = shader;

	// Vertex Shader
	// Initialize empty constant buffers
	UINT numConstantBuffers = ((D3D11Shaderset*)m_pShaderset)->numVSConstantBuffers;
	m_VSConstantBuffers.reserve(numConstantBuffers);
	for (int i=0; i<numConstantBuffers; i++)
	{
		ConstantBufferData empty;
		m_VSConstantBuffers.push_back(empty);
		UINT size = ((D3D11Shaderset*)m_pShaderset)->getVSConstantBufferSize(i);
		m_VSConstantBuffers.back().init( size );
	}
	
	// Pixel Shader
	// Initialize empty constant buffers
	numConstantBuffers = ((D3D11Shaderset*)m_pShaderset)->numPSConstantBuffers;
	m_PSConstantBuffers.reserve(numConstantBuffers);
	for (int i=0; i<numConstantBuffers; i++)
	{
		ConstantBufferData empty;
		m_PSConstantBuffers.push_back(empty);
		UINT size = ((D3D11Shaderset*)m_pShaderset)->getPSConstantBufferSize(i);
		m_PSConstantBuffers.back().init( size );
	}
}


void ShaderParams::setParam( const std::string& paramName, UINT index, const XMFLOAT4X4* pMatrixParam )
{
	// Vertex Shader
	const ShaderVariable* pvar = ((D3D11Shaderset*)m_pShaderset)->getVSVariable( paramName );
	if(pvar)
	{
		if ( index < pvar->Elements )
		{
			//memcpy( (m_pVSCBDataBuffer[pvar->ConstantBuffer])+pvar->Offset+(it->first.second*sizeof(XMFLOAT4X4)), &it->second, min( pvar->Size - (it->first.second*sizeof(XMFLOAT4X4)), sizeof(XMFLOAT4X4)) );
			UINT size = min( pvar->Size - (index*sizeof(XMFLOAT4X4)), sizeof(XMFLOAT4X4));
			m_VSConstantBuffers.at(pvar->ConstantBuffer).setData( pvar->Offset + index*sizeof(XMFLOAT4X4), (void*)pMatrixParam, size );
		}
		else
		{
			DEBUG_OUTPUT( __FUNCTION__ );
			DEBUG_OUTPUT( " : '" );
			DEBUG_OUTPUT( paramName.c_str() );
			DEBUG_OUTPUT( "' element '" );
			char str[10];
			_itoa_s(index, str, 10);
			DEBUG_OUTPUT( str );
			DEBUG_OUTPUT( "' : index out of bounds.\n" );
		}
	}
	
	// Pixel Shader
	pvar = ((D3D11Shaderset*)m_pShaderset)->getPSVariable( paramName );
	if(pvar)
	{
		if ( index < pvar->Elements )
		{
			//memcpy( (m_pVSCBDataBuffer[pvar->ConstantBuffer])+pvar->Offset+(it->first.second*sizeof(XMFLOAT4X4)), &it->second, min( pvar->Size - (it->first.second*sizeof(XMFLOAT4X4)), sizeof(XMFLOAT4X4)) );
			UINT size = min( pvar->Size - (index*sizeof(XMFLOAT4X4)), sizeof(XMFLOAT4X4));
			m_PSConstantBuffers.at(pvar->ConstantBuffer).setData( pvar->Offset + index*sizeof(XMFLOAT4X4), (void*)pMatrixParam, size );
		}
		else
		{
			DEBUG_OUTPUT( __FUNCTION__ );
			DEBUG_OUTPUT( " : '" );
			DEBUG_OUTPUT( paramName.c_str() );
			DEBUG_OUTPUT( "' element '" );
			char str[10];
			_itoa_s(index, str, 10);
			DEBUG_OUTPUT( str );
			DEBUG_OUTPUT( "' : index out of bounds.\n" );
		}
	}
}

void ShaderParams::setParam( const std::string& paramName, UINT index, const XMFLOAT4* pVectorParam )
{
	// Vertex Shader
	const ShaderVariable* pvar = ((D3D11Shaderset*)m_pShaderset)->getVSVariable( paramName );
	if(pvar)
	{
		if ( index < pvar->Elements )
		{
			//memcpy( (void*)(m_pVSCBDataBuffer[pvar->ConstantBuffer]+pvar->Offset+(it->first.second*sizeof(XMFLOAT4))), &it->second, min( pvar->Size - (it->first.second*sizeof(XMFLOAT4)), sizeof(XMFLOAT4)) );
			UINT size = min( pvar->Size - (index*sizeof(XMFLOAT4)), sizeof(XMFLOAT4));
			m_VSConstantBuffers.at(pvar->ConstantBuffer).setData( pvar->Offset + index*sizeof(XMFLOAT4), (void*)pVectorParam, size );
		}
		else
		{
			DEBUG_OUTPUT( __FUNCTION__ );
			DEBUG_OUTPUT( " : '" );
			DEBUG_OUTPUT( paramName.c_str() );
			DEBUG_OUTPUT( "' element '" );
			char str[10];
			_itoa_s(index, str, 10);
			DEBUG_OUTPUT( str );
			DEBUG_OUTPUT( "' : index out of bounds.\n" );
		}
	}
	
	// Pixel Shader
	pvar = ((D3D11Shaderset*)m_pShaderset)->getPSVariable( paramName );
	if(pvar)
	{
		if ( index < pvar->Elements )
		{
			//memcpy( (void*)(m_pVSCBDataBuffer[pvar->ConstantBuffer]+pvar->Offset+(it->first.second*sizeof(XMFLOAT4))), &it->second, min( pvar->Size - (it->first.second*sizeof(XMFLOAT4)), sizeof(XMFLOAT4)) );
			UINT size = min( pvar->Size - (index*sizeof(XMFLOAT4)), sizeof(XMFLOAT4));
			m_PSConstantBuffers.at(pvar->ConstantBuffer).setData( pvar->Offset + index*sizeof(XMFLOAT4), (void*)pVectorParam, size );
		}
		else
		{
			DEBUG_OUTPUT( __FUNCTION__ );
			DEBUG_OUTPUT( " : '" );
			DEBUG_OUTPUT( paramName.c_str() );
			DEBUG_OUTPUT( "' element '" );
			char str[10];
			_itoa_s(index, str, 10);
			DEBUG_OUTPUT( str );
			DEBUG_OUTPUT( "' : index out of bounds.\n" );
		}
	}
}

void ShaderParams::setParam( const std::string& paramName, UINT index, float pScalarParam )
{
	// Vertex Shader
	const ShaderVariable* pvar = ((D3D11Shaderset*)m_pShaderset)->getVSVariable( paramName );
	if(pvar)
	{
		if ( index < pvar->Elements )
		{
			//memcpy( (void*)(m_pVSCBDataBuffer[pvar->ConstantBuffer]+pvar->Offset+(it->first.second*sizeof(XMFLOAT4))), &it->second, min( pvar->Size - (it->first.second*sizeof(XMFLOAT4)), sizeof(XMFLOAT4)) );
			//UINT size = min( pvar->Size - (index*sizeof(XMFLOAT4)), sizeof(XMFLOAT4));
			UINT size = sizeof(float);
			m_VSConstantBuffers.at(pvar->ConstantBuffer).setData( pvar->Offset + index*sizeof(XMFLOAT4), &pScalarParam, size );
		}
		else
		{
			DEBUG_OUTPUT( __FUNCTION__ );
			DEBUG_OUTPUT( " : '" );
			DEBUG_OUTPUT( paramName.c_str() );
			DEBUG_OUTPUT( "' element '" );
			char str[10];
			_itoa_s(index, str, 10);
			DEBUG_OUTPUT( str );
			DEBUG_OUTPUT( "' : index out of bounds.\n" );
		}
	}
	
	// Pixel Shader
	pvar = ((D3D11Shaderset*)m_pShaderset)->getPSVariable( paramName );
	if(pvar)
	{
		if ( index < pvar->Elements )
		{
			//memcpy( (void*)(m_pVSCBDataBuffer[pvar->ConstantBuffer]+pvar->Offset+(it->first.second*sizeof(XMFLOAT4))), &it->second, min( pvar->Size - (it->first.second*sizeof(XMFLOAT4)), sizeof(XMFLOAT4)) );
			//UINT size = min( pvar->Size - (index*sizeof(XMFLOAT4)), sizeof(XMFLOAT4));
			UINT size = sizeof(float);
			m_PSConstantBuffers.at(pvar->ConstantBuffer).setData( pvar->Offset + index*sizeof(XMFLOAT4), &pScalarParam, size );
		}
		else
		{
			DEBUG_OUTPUT( __FUNCTION__ );
			DEBUG_OUTPUT( " : '" );
			DEBUG_OUTPUT( paramName.c_str() );
			DEBUG_OUTPUT( "' element '" );
			char str[10];
			_itoa_s(index, str, 10);
			DEBUG_OUTPUT( str );
			DEBUG_OUTPUT( "' : index out of bounds.\n" );
		}
	}
}

void ShaderParams::setParam( const std::string& paramName, UINT index, const Matrix4x4* matrix )
{
	// Vertex Shader
	const ShaderVariable* pvar = ((D3D11Shaderset*)m_pShaderset)->getVSVariable( paramName );
	if(pvar)
	{
		if ( index < pvar->Elements )
		{
			//memcpy( (m_pVSCBDataBuffer[pvar->ConstantBuffer])+pvar->Offset+(it->first.second*sizeof(XMFLOAT4X4)), &it->second, min( pvar->Size - (it->first.second*sizeof(XMFLOAT4X4)), sizeof(XMFLOAT4X4)) );
			UINT size = min( pvar->Size - (index*sizeof(XMFLOAT4X4)), sizeof(XMFLOAT4X4));
			m_VSConstantBuffers.at(pvar->ConstantBuffer).setData( pvar->Offset + index*sizeof(XMFLOAT4X4), (void*)matrix, size );
		}
		else
		{
			DEBUG_OUTPUT( __FUNCTION__ );
			DEBUG_OUTPUT( " : '" );
			DEBUG_OUTPUT( paramName.c_str() );
			DEBUG_OUTPUT( "' element '" );
			char str[10];
			_itoa_s(index, str, 10);
			DEBUG_OUTPUT( str );
			DEBUG_OUTPUT( "' : index out of bounds.\n" );
		}
	}
	
	// Pixel Shader
	pvar = ((D3D11Shaderset*)m_pShaderset)->getPSVariable( paramName );
	if(pvar)
	{
		if ( index < pvar->Elements )
		{
			//memcpy( (m_pVSCBDataBuffer[pvar->ConstantBuffer])+pvar->Offset+(it->first.second*sizeof(XMFLOAT4X4)), &it->second, min( pvar->Size - (it->first.second*sizeof(XMFLOAT4X4)), sizeof(XMFLOAT4X4)) );
			UINT size = min( pvar->Size - (index*sizeof(XMFLOAT4X4)), sizeof(XMFLOAT4X4));
			m_PSConstantBuffers.at(pvar->ConstantBuffer).setData( pvar->Offset + index*sizeof(XMFLOAT4X4), (void*)matrix, size );
		}
		else
		{
			DEBUG_OUTPUT( __FUNCTION__ );
			DEBUG_OUTPUT( " : '" );
			DEBUG_OUTPUT( paramName.c_str() );
			DEBUG_OUTPUT( "' element '" );
			char str[10];
			_itoa_s(index, str, 10);
			DEBUG_OUTPUT( str );
			DEBUG_OUTPUT( "' : index out of bounds.\n" );
		}
	}
}

/*
void ShaderParams::setParam( const std::string& paramName, UINT index, const Matrix4x4* matrix )
{
	setParam( paramName, index, (XMFLOAT4X4*)matrix );
}*/

void ShaderParams::setParam( const std::string& paramName, UINT index, const Vector4* pVectorParam )
{
	setParam( paramName, index, &XMFLOAT4(pVectorParam->x,pVectorParam->y,pVectorParam->z,pVectorParam->w) );
}

void ShaderParams::setParam( const std::string& paramName, UINT index, const XMMATRIX* pMatrixParam )
{
	XMFLOAT4X4 matrix;
	XMStoreFloat4x4( &matrix, *pMatrixParam );
	setParam( paramName, index, &matrix );
}

void ShaderParams::setParam( const std::string& paramName, UINT index, int pScalarParam )
{
	setParam( paramName, index, reinterpret_cast<float&>(pScalarParam) );
}

void ShaderParams::setParam( const std::string& paramName, const Matrix4x4* matrix )
{
	setParam( paramName, 0, matrix );
}

void ShaderParams::setParam( const std::string& paramName, const XMMATRIX* pMatrixParam )
{
	setParam( paramName, 0, pMatrixParam );
}

void ShaderParams::setParam( const std::string& paramName, const XMFLOAT4X4* pMatrixParam )
{
	setParam( paramName, 0, pMatrixParam );
}

void ShaderParams::setParam( const std::string& paramName, const XMFLOAT4* pVectorParam )
{
	setParam( paramName, 0, pVectorParam );
}

void ShaderParams::setParam( const std::string& paramName, const Vector4* pVectorParam )
{
	setParam( paramName, 0, &XMFLOAT4(pVectorParam->x,pVectorParam->y,pVectorParam->z,pVectorParam->w) );
}

void ShaderParams::setParam( const std::string& paramName, float pScalarParam )
{
	setParam( paramName, 0, pScalarParam );
}

void ShaderParams::setParam( const std::string& paramName, int pScalarParam )
{
	setParam( paramName, 0, pScalarParam );
}

void ShaderParams::assign( ShaderParamBlock* pParamBlock )
{
	std::map< std::pair< std::string, UINT > , XMFLOAT4X4, std::less<std::pair< std::string, UINT >> , tbb::scalable_allocator<std::pair<std::pair< std::string, UINT >,XMFLOAT4X4>> > *pMatrixParams = pParamBlock->getMatrixParams();
	std::map< std::pair< std::string, UINT > , XMFLOAT4 , std::less<std::pair< std::string, UINT >>, tbb::scalable_allocator<std::pair<std::pair< std::string, UINT >,XMFLOAT4>> > *pVectorParams = pParamBlock->getVectorParams();
	std::map< std::pair< std::string, UINT > , float , std::less<std::pair< std::string, UINT >>, tbb::scalable_allocator<std::pair<std::pair< std::string, UINT >,float>> > *pScalarParams = pParamBlock->getScalarParams();

	// Matrix type parameters
	if ( !pMatrixParams->empty() )
	{
		// ((paramName,index),value)
		for( std::map< std::pair< std::string, UINT > , XMFLOAT4X4, std::less<std::pair< std::string, UINT >> , tbb::scalable_allocator<std::pair<std::pair< std::string, UINT >,XMFLOAT4X4>> >::const_iterator it=pMatrixParams->begin(); it!=pMatrixParams->end(); ++it )
		{
			setParam(it->first.first, it->first.second, &it->second);
		}
	}

	// Vector type parameters
	if ( !pVectorParams->empty() )
	{
		// ((paramName,index),value)
		for( std::map< std::pair< std::string, UINT > , XMFLOAT4 , std::less<std::pair< std::string, UINT >>, tbb::scalable_allocator<std::pair<std::pair< std::string, UINT >,XMFLOAT4>> >::const_iterator it=pVectorParams->begin(); it!=pVectorParams->end(); ++it )
		{
			setParam(it->first.first, it->first.second, &it->second);
		}
	}

	// Scalar type parameters
	if ( !pScalarParams->empty() )
	{
		for( std::map< std::pair< std::string, UINT > , float , std::less<std::pair< std::string, UINT >>, tbb::scalable_allocator<std::pair<std::pair< std::string, UINT >,float>> >::const_iterator it=pScalarParams->begin(); it!=pScalarParams->end(); ++it )
		{
			setParam(it->first.first, it->first.second, it->second);
		}
	}
}