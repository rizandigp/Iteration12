#pragma once

#include "..\Shaderset.h"
#include <d3d11.h>
#include <d3dx11.h>
#include <d3dcompiler.h>


// The DX11 implementation of the Shaderset class
class DX11Shaderset : public Shaderset
{
public:
	DX11Shaderset();
	~DX11Shaderset();

	// Mutators
	void SetVertexShader( ID3D11VertexShader* pShader )				{ m_pVertexShader = pShader; };
	void SetGeometryShader( ID3D11GeometryShader* pShader )			{ m_pGeometryShader = pShader; };
	void SetPixelShader( ID3D11PixelShader* pShader )				{ m_pPixelShader = pShader; };
	void SetHullShader( ID3D11HullShader* pShader )					{ m_pHullShader = pShader; };
	void SetDomainShader( ID3D11DomainShader* pShader )				{ m_pDomainShader = pShader; };
	void SetComputeShader( ID3D11ComputeShader* pShader )			{ m_pComputeShader = pShader; };
	void SetInputSignature( ID3DBlob* pInputSignature )				{ m_pInputSignature = pInputSignature; };

	void SetVSShaderResourceIndex( const std::string& name, UINT index )	{ m_pVSShaderResourceIndices[name] = index; };
	void SetGSShaderResourceIndex( const std::string& name, UINT index )	{ m_pGSShaderResourceIndices[name] = index; };
	void SetPSShaderResourceIndex( const std::string& name, UINT index )	{ m_pPSShaderResourceIndices[name] = index; };
	void SetCSShaderResourceIndex( const std::string& name, UINT index )	{ m_pCSShaderResourceIndices[name] = index; };

	void SetVSVariable( const std::string& name, ShaderVariable var )		{ m_pVSVariables[name] = var; };
	void SetGSVariable( const std::string& name, ShaderVariable var )		{ m_pGSVariables[name] = var; };
	void SetPSVariable( const std::string& name, ShaderVariable var )		{ m_pPSVariables[name] = var; };
	void SetCSVariable( const std::string& name, ShaderVariable var )		{ m_pCSVariables[name] = var; };

	void SetVSConstantBufferSize( UINT index, UINT byteWidth )		{ m_pVSConstantBufferByteSizes[index] = byteWidth; };
	void SetGSConstantBufferSize( UINT index, UINT byteWidth )		{ m_pGSConstantBufferByteSizes[index] = byteWidth; };
	void SetPSConstantBufferSize( UINT index, UINT byteWidth )		{ m_pPSConstantBufferByteSizes[index] = byteWidth; };
	void SetCSConstantBufferSize( UINT index, UINT byteWidth )		{ m_pCSConstantBufferByteSizes[index] = byteWidth; };

	// TODO : Remember what these are for
	void SetVSSampler( UINT index, const std::string& name, ID3D11SamplerState* pSampler );
	void SetGSSampler( UINT index, const std::string& name, ID3D11SamplerState* pSampler );
	void SetPSSampler( UINT index, const std::string& name, ID3D11SamplerState* pSampler );
	void SetCSSampler( UINT index, const std::string& name, ID3D11SamplerState* pSampler );

	inline ID3D11VertexShader*	GetVertexShader()							{ return m_pVertexShader; };
	inline ID3D11GeometryShader* GetGeometryShader()						{ return m_pGeometryShader; };
	inline ID3D11PixelShader*	GetPixelShader()							{ return m_pPixelShader; };
	inline ID3D11HullShader*	GetHullShader()								{ return m_pHullShader; };
	inline ID3D11DomainShader*	GetDomainShader()							{ return m_pDomainShader; };
	inline ID3D11ComputeShader*	GetComputeShader()							{ return m_pComputeShader; };
	inline ID3DBlob*			GetInputSignature()							{ return m_pInputSignature; };

	// Accessors
	inline int GetVSShaderResourceIndex( const std::string& name );
	inline int GetGSShaderResourceIndex( const std::string& name );
	inline int GetPSShaderResourceIndex( const std::string& name );
	inline int GetCSShaderResourceIndex( const std::string& name );
	
	inline const ShaderVariable* GetVSVariable( const std::string& name ) const;
	inline const ShaderVariable* GetGSVariable( const std::string& name ) const;
	inline const ShaderVariable* GetPSVariable( const std::string& name ) const;
	inline const ShaderVariable* GetCSVariable( const std::string& name ) const;
	
	inline int GetVSConstantBufferSize( UINT index )					{ return m_pVSConstantBufferByteSizes.find(index)->second; };
	inline int GetGSConstantBufferSize( UINT index )					{ return m_pGSConstantBufferByteSizes.find(index)->second; };
	inline int GetPSConstantBufferSize( UINT index )					{ return m_pPSConstantBufferByteSizes.find(index)->second; };
	inline int GetCSConstantBufferSize( UINT index )					{ return m_pCSConstantBufferByteSizes.find(index)->second; };
	
	UINT numVSConstantBuffers;
	UINT numGSConstantBuffers;
	UINT numPSConstantBuffers;
	UINT numCSConstantBuffers;

	void Bind( RenderDispatcher* pDispatcher );
	void Unbind( RenderDispatcher* pDispatcher );	// Not needed?

	// TODO : implement
	void ReleaseResources();

private:
	// Shaders
	ID3D11VertexShader* m_pVertexShader;
	ID3D11GeometryShader* m_pGeometryShader;
	ID3D11PixelShader* m_pPixelShader;
	ID3D11HullShader* m_pHullShader;
	ID3D11DomainShader* m_pDomainShader;
	ID3D11ComputeShader* m_pComputeShader;

	ID3DBlob*	m_pInputSignature;
	
	// Shader resource binding indices
	std::map<std::string, UINT> m_pVSShaderResourceIndices;
	std::map<std::string, UINT> m_pGSShaderResourceIndices;
	std::map<std::string, UINT> m_pPSShaderResourceIndices;
	std::map<std::string, UINT> m_pCSShaderResourceIndices;

	// Shader variables
	std::unordered_map<std::string, ShaderVariable>	m_pVSVariables;
	std::unordered_map<std::string, ShaderVariable>	m_pGSVariables;
	std::unordered_map<std::string, ShaderVariable>	m_pPSVariables;
	std::unordered_map<std::string, ShaderVariable>	m_pCSVariables;

	// Constant buffer sizes <index, size>
	std::unordered_map<UINT, UINT> m_pVSConstantBufferByteSizes;
	std::unordered_map<UINT, UINT> m_pGSConstantBufferByteSizes;
	std::unordered_map<UINT, UINT> m_pPSConstantBufferByteSizes;
	std::unordered_map<UINT, UINT> m_pCSConstantBufferByteSizes;
};

//////////////////////////////////
//
//	optimization lessons learned:
//	-unordered_map has significantly faster find() than map in this case
//	-find() is not so cheap, keep the pointers.
//
//
//
/////////////////////////////////

inline INT DX11Shaderset::GetVSShaderResourceIndex( const std::string& name )	
{
	std::map<std::string, UINT>::const_iterator it = m_pVSShaderResourceIndices.find(name);
	if(it!=m_pVSShaderResourceIndices.end())
		return m_pVSShaderResourceIndices.find(name)->second; 
	else
		return -1; 
}

inline INT DX11Shaderset::GetGSShaderResourceIndex( const std::string& name )					
{
	std::map<std::string, UINT>::const_iterator it = m_pGSShaderResourceIndices.find(name);
	if(it!=m_pGSShaderResourceIndices.end())
		return m_pGSShaderResourceIndices.find(name)->second;
	else
		return -1;
}

inline INT DX11Shaderset::GetPSShaderResourceIndex( const std::string& name )
{
	std::map<std::string, UINT>::const_iterator it = m_pPSShaderResourceIndices.find(name);
	if(it!=m_pPSShaderResourceIndices.end())
		return m_pPSShaderResourceIndices.find(name)->second;
	else
		return -1;
}

inline INT DX11Shaderset::GetCSShaderResourceIndex( const std::string& name )
{
	std::map<std::string, UINT>::const_iterator it = m_pCSShaderResourceIndices.find(name);
	if(it!=m_pCSShaderResourceIndices.end())
		return m_pCSShaderResourceIndices.find(name)->second;
	else
		return -1;
}
	

inline const ShaderVariable* DX11Shaderset::GetVSVariable( const std::string& name ) const
{
	std::unordered_map<std::string, ShaderVariable>::const_iterator it = m_pVSVariables.find(name);
	if(it!=m_pVSVariables.end())
		return &it->second;
	return NULL;
}

inline const ShaderVariable* DX11Shaderset::GetGSVariable( const std::string& name ) const
{
	std::unordered_map<std::string, ShaderVariable>::const_iterator it = m_pGSVariables.find(name);
	if(it!=m_pGSVariables.end())
		return &it->second;
	return NULL;
}

inline const ShaderVariable* DX11Shaderset::GetPSVariable( const std::string& name ) const
{
	std::unordered_map<std::string, ShaderVariable>::const_iterator it = m_pPSVariables.find(name);
	if(it!=m_pPSVariables.end())
		return &it->second;
	return NULL;
}

inline const ShaderVariable* DX11Shaderset::GetCSVariable( const std::string& name ) const
{
	std::unordered_map<std::string, ShaderVariable>::const_iterator it = m_pCSVariables.find(name);
	if(it!=m_pCSVariables.end())
		return &it->second;
	return NULL;
}
