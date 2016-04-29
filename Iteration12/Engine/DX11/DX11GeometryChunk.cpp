#pragma once

#include "DX11GeometryChunk.h"
#include "DX11RenderCommand.h"
#include "DX11RenderDispatcher.h"
#include "DX11Shaderset.h"
#include "..\RenderSystem.h"

void DX11GeometryChunk::UpdateVertexBuffer( float* vertices, UINT vertexCount, UINT byteSize, UINT stride )
{
	if (!m_bDynamic)
	{
		NGWARNING( " : Failed. The geometry is not set as dynamic. Vertex buffer content unchanged.\n" );
		return;
	}

	// Map new data to GPU
	DX11RenderCommand_Map mapcommand;
	mapcommand.SetResource( this->GetVertexBuffer(0)->GetBuffer(), vertices, byteSize );
	m_pRenderSystem->Submit( &mapcommand );

	// Update bounding box
	XNA::AxisAlignedBox AABB;
	XNA::ComputeBoundingAxisAlignedBoxFromPoints( &AABB, vertexCount, (XMFLOAT3*)vertices, stride );
	this->SetAABB(&AABB);
}

void DX11GeometryChunk::Bind( RenderDispatcher* pDispatcher )
{
	// Retrieve pointers
	DX11RenderDispatcher *pd3d11Dispatcher = dynamic_cast<DX11RenderDispatcher*>(pDispatcher);
	ID3D11Device* pDevice = pd3d11Dispatcher->GetDevice();
	ID3D11DeviceContext *pContext = pd3d11Dispatcher->GetImmediateContext();

	// TODO : multiple VBs
	ID3D11Buffer* vb = GetVertexBuffer(0)->GetBuffer();
	ID3D11Buffer* ib = GetIndexBuffer()->GetBuffer();

	BufferLayout *buffLayout = GetVertexBuffer(0)->GetBufferLayout();

	// Set input layout
	// TODO : optimize! too many input layouts get created!
	ID3D11InputLayout* pInputLayout = NULL;
	pInputLayout = GetCompatibleInputLayout( pd3d11Dispatcher->GetActiveShaderset() );

	if ( pInputLayout )
	{
		pContext->IASetInputLayout( pInputLayout );
	}
	else
	{
		pInputLayout = CreateInputLayoutFromBufferLayouts( pDevice, &buffLayout, 1, pd3d11Dispatcher->GetActiveShaderset()->GetInputSignature()->GetBufferPointer(), pd3d11Dispatcher->GetActiveShaderset()->GetInputSignature()->GetBufferSize() );
		SetCompatibleInputLayout( pd3d11Dispatcher->GetActiveShaderset(), pInputLayout );
		pContext->IASetInputLayout( pInputLayout );
	}
	
	
	// Set vertex & index buffer
    UINT stride = GetVertexBuffer(0)->GetStride();
    UINT offset = 0;
    pContext->IASetVertexBuffers( 0, 1, &vb , &stride, &offset );
    pContext->IASetIndexBuffer( ib, DXGI_FORMAT_R32_UINT, 0 );

	pContext->IASetPrimitiveTopology( ToD3DPrimitiveTopology(m_PrimitiveTopology) );
}


//	Helper functions
ID3D11InputLayout* DX11GeometryChunk::CreateInputLayoutFromBufferLayouts( ID3D11Device* pDevice, BufferLayout* BufferLayouts[], int numberOfBuffers, const void *pShaderBytecodeWithInputSignature, SIZE_T BytecodeLength )
{
	std::vector<D3D11_INPUT_ELEMENT_DESC> inputElementDescs;
	UINT byteOffset = 0;
	int i, ii;
	for ( i = 0; i < numberOfBuffers; i++ )
	{
		VertexElement* pElements = (*BufferLayouts[i]).GetElements();
		int numElements = (*BufferLayouts[i]).GetNumberOfElements();
		for (ii=0; ii<numElements; ii++)
		{
			D3D11_INPUT_ELEMENT_DESC elementDesc;
			elementDesc.SemanticName = pElements[ii].SemanticName;		
			elementDesc.SemanticIndex = pElements[ii].SemanticIndex;
			elementDesc.InputSlot = i;
			elementDesc.Format = pElements[ii].Format;
			elementDesc.AlignedByteOffset = byteOffset;
			elementDesc.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
			elementDesc.InstanceDataStepRate = 0;

			inputElementDescs.push_back( elementDesc );

			if ( (pElements[ii].Format == DXGI_FORMAT_R32_FLOAT) || (pElements[ii].Format == DXGI_FORMAT_R32_UINT) || (pElements[ii].Format == DXGI_FORMAT_R32_SINT) )
				byteOffset += 4;
			else if ( (pElements[ii].Format == DXGI_FORMAT_R32G32_FLOAT) || (pElements[ii].Format == DXGI_FORMAT_R32G32_UINT) || (pElements[ii].Format == DXGI_FORMAT_R32G32_SINT) )
				byteOffset += 8;
			else if ( (pElements[ii].Format == DXGI_FORMAT_R32G32B32_FLOAT) || (pElements[ii].Format == DXGI_FORMAT_R32G32B32_UINT) || (pElements[ii].Format == DXGI_FORMAT_R32G32B32_SINT) )
				byteOffset += 12;
			else if ( (pElements[ii].Format == DXGI_FORMAT_R32G32B32A32_FLOAT) || (pElements[ii].Format == DXGI_FORMAT_R32G32B32A32_UINT) || (pElements[ii].Format == DXGI_FORMAT_R32G32B32A32_SINT) )
				byteOffset += 16;
		}
	}

	ID3D11InputLayout* pInputLayout;
	HRESULT hr = pDevice->CreateInputLayout( &inputElementDescs[0], inputElementDescs.size(), pShaderBytecodeWithInputSignature, BytecodeLength, &pInputLayout );
	if( FAILED( hr ) )
	{
		DEBUG_OUTPUT( __FUNCTION__ );
		DEBUG_OUTPUT( " : Failed creating a compatible D3D11 Input Layout.\n" );
		return NULL;
	}

	return pInputLayout;
};

HRESULT DX11GeometryChunk::CreateInputLayoutDescFromVertexShaderSignature( ID3DBlob* pShaderBlob, ID3D11Device* pD3DDevice, ID3D11InputLayout** pInputLayout )
{
	// Reflect shader info
	ID3D11ShaderReflection* pVertexShaderReflection = NULL;	
	if ( FAILED( D3DReflect( pShaderBlob->GetBufferPointer(), pShaderBlob->GetBufferSize(), IID_ID3D10ShaderReflection, (void**) &pVertexShaderReflection ) ) ) 
	{
		DEBUG_OUTPUT( __FUNCTION__ );
		DEBUG_OUTPUT( " : Shader reflection failed. Returning S_FALSE\n" );
		return S_FALSE;
	}
	
	// Get shader info
	D3D11_SHADER_DESC shaderDesc;
	pVertexShaderReflection->GetDesc( &shaderDesc );
	
	// Read input layout description from shader info
	UINT32 byteOffset = 0;
	std::vector<D3D11_INPUT_ELEMENT_DESC> inputLayoutDesc;
	for ( UINT32 i=0; i< shaderDesc.InputParameters; i++ )
	{
		D3D11_SIGNATURE_PARAMETER_DESC paramDesc;		
		pVertexShaderReflection->GetInputParameterDesc(i, &paramDesc );

		// fill out input element desc
		D3D11_INPUT_ELEMENT_DESC elementDesc;	
		elementDesc.SemanticName = paramDesc.SemanticName;		
		elementDesc.SemanticIndex = paramDesc.SemanticIndex;
		elementDesc.InputSlot = 0;
		elementDesc.AlignedByteOffset = byteOffset;
		elementDesc.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		elementDesc.InstanceDataStepRate = 0;	

		// determine DXGI format
		if ( paramDesc.Mask == 1 )
		{
			if ( paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32 ) elementDesc.Format = DXGI_FORMAT_R32_UINT;
			else if ( paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32 ) elementDesc.Format = DXGI_FORMAT_R32_SINT;
			else if ( paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32 ) elementDesc.Format = DXGI_FORMAT_R32_FLOAT;
			byteOffset += 4;
		}
		else if ( paramDesc.Mask <= 3 )
		{
			if ( paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32 ) elementDesc.Format = DXGI_FORMAT_R32G32_UINT;
			else if ( paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32 ) elementDesc.Format = DXGI_FORMAT_R32G32_SINT;
			else if ( paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32 ) elementDesc.Format = DXGI_FORMAT_R32G32_FLOAT;
			byteOffset += 8;
		}
		else if ( paramDesc.Mask <= 7 )
		{
			if ( paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32 ) elementDesc.Format = DXGI_FORMAT_R32G32B32_UINT;
			else if ( paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32 ) elementDesc.Format = DXGI_FORMAT_R32G32B32_SINT;
			else if ( paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32 ) elementDesc.Format = DXGI_FORMAT_R32G32B32_FLOAT;
			byteOffset += 12;
		}
		else if ( paramDesc.Mask <= 15 )
		{
			if ( paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32 ) elementDesc.Format = DXGI_FORMAT_R32G32B32A32_UINT;
			else if ( paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32 ) elementDesc.Format = DXGI_FORMAT_R32G32B32A32_SINT;
			else if ( paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32 ) elementDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
			byteOffset += 16;
		}
			
		//save element desc
		inputLayoutDesc.push_back(elementDesc);
	}		

	// Try to create Input Layout
	HRESULT hr = pD3DDevice->CreateInputLayout( &inputLayoutDesc[0], inputLayoutDesc.size(), pShaderBlob->GetBufferPointer(), pShaderBlob->GetBufferSize(), pInputLayout );

	//Free allocation shader reflection memory
	pVertexShaderReflection->Release();
	return hr;
}
