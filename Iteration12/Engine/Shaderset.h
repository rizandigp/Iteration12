#pragma once

#include "Prerequisites.h"
#include "GPUResource.h"

#include <map>
#include <unordered_map>
#include <string>
#include <d3d11.h>
#include <d3dx11.h>
#include <d3dcompiler.h>


enum SHADERMODEL
{
	SM_2_0,
	SM_3_0,
	SM_4_0,
	SM_5_0,
	SM_AUTO,
};

enum SHADER_VARIABLE_TYPE
{
	SVT_INT,
	SVT_UINT,
	SVT_FLOAT,
	SVT_DOUBLE,
	SVT_VECTOR,
	SVT_MATRIX,
};

struct ShaderVariable
{
	UINT ConstantBuffer;	// Index of CB it belongs in
	SHADER_VARIABLE_TYPE Type;	// Currently unused
	std::string Name;		
	UINT Size;				// Size of variable in bytes
	UINT Offset;			// Offset in constant buffer's backing store
	UINT Elements;			// Number of elements (1 if not an array)
};

struct ShaderMacro
{
	std::string Name;
	std::string Definition;

	ShaderMacro(std::string name, std::string definition)
	{
		Name = name;
		Definition = definition;
	}

	inline bool ShaderMacro::operator== (const ShaderMacro &other) const
	{
		if (Name==other.Name && Definition==other.Definition)
			return true;
		else
			return false;
	}
};

struct ShadersetDescription
{
	std::wstring filename;
	std::string vertexShader;
	std::string pixelShader;
	SHADERMODEL sm;
	std::vector<ShaderMacro> macros;	// TODO : Hash?
	bool debug;

	inline bool ShadersetDescription::operator== (const ShadersetDescription& other) const
	{
		if (filename==other.filename)
			if (vertexShader==other.vertexShader)
				if (pixelShader==other.pixelShader)
					if (sm==other.sm)
						if (macros==other.macros)
							if (debug==other.debug)
								return true;
		return false;
	}
};

class Shaderset : public GPUResource
{
public:

	virtual void Bind( RenderDispatcher* pDispatcher )=0;
	virtual void Unbind( RenderDispatcher* pDispatcher )=0;		// Not needed?

	virtual void ReleaseResources()=0;
};


class D3D11Shaderset : public Shaderset
{
public:
	D3D11Shaderset();
	~D3D11Shaderset();

	// Mutators
	void SetVertexShader( ID3D11VertexShader* pShader )				{ m_pVertexShader = pShader; };
	void SetGeometryShader( ID3D11GeometryShader* pShader )			{ m_pGeometryShader = pShader; };
	void SetPixelShader( ID3D11PixelShader* pShader )				{ m_pPixelShader = pShader; };
	void SetHullShader( ID3D11HullShader* pShader )					{ m_pHullShader = pShader; };
	void SetDomainShader( ID3D11DomainShader* pShader )				{ m_pDomainShader = pShader; };
	void SetComputeShader( ID3D11ComputeShader* pShader )			{ m_pComputeShader = pShader; };
	void SetInputSignature( ID3DBlob* pInputSignature )				{ m_pInputSignature = pInputSignature; };

	void SetVSShaderResourceIndex( std::string name, UINT index )	{ m_pVSShaderResourceIndices[name] = index; };
	void SetGSShaderResourceIndex( std::string name, UINT index )	{ m_pGSShaderResourceIndices[name] = index; };
	void SetPSShaderResourceIndex( std::string name, UINT index )	{ m_pPSShaderResourceIndices[name] = index; };
	void SetCSShaderResourceIndex( std::string name, UINT index )	{ m_pCSShaderResourceIndices[name] = index; };

	void SetVSVariable( std::string name, ShaderVariable var )		{ m_pVSVariables[name] = var; };
	void SetGSVariable( std::string name, ShaderVariable var )		{ m_pGSVariables[name] = var; };
	void SetPSVariable( std::string name, ShaderVariable var )		{ m_pPSVariables[name] = var; };
	void SetCSVariable( std::string name, ShaderVariable var )		{ m_pCSVariables[name] = var; };

	void SetVSConstantBufferSize( UINT index, UINT byteWidth )		{ m_pVSConstantBufferByteSizes[index] = byteWidth; };
	void SetGSConstantBufferSize( UINT index, UINT byteWidth )		{ m_pGSConstantBufferByteSizes[index] = byteWidth; };
	void SetPSConstantBufferSize( UINT index, UINT byteWidth )		{ m_pPSConstantBufferByteSizes[index] = byteWidth; };
	void SetCSConstantBufferSize( UINT index, UINT byteWidth )		{ m_pCSConstantBufferByteSizes[index] = byteWidth; };

	void SetVSSampler( UINT index, std::string name, ID3D11SamplerState* pSampler );
	void SetGSSampler( UINT index, std::string name, ID3D11SamplerState* pSampler );
	void SetPSSampler( UINT index, std::string name, ID3D11SamplerState* pSampler );
	void SetCSSampler( UINT index, std::string name, ID3D11SamplerState* pSampler );

	inline ID3D11VertexShader*	GetVertexShader()							{ return m_pVertexShader; };
	inline ID3D11GeometryShader* GetGeometryShader()						{ return m_pGeometryShader; };
	inline ID3D11PixelShader*	GetPixelShader()							{ return m_pPixelShader; };
	inline ID3D11HullShader*	GetHullShader()								{ return m_pHullShader; };
	inline ID3D11DomainShader*	GetDomainShader()							{ return m_pDomainShader; };
	inline ID3D11ComputeShader*	GetComputeShader()							{ return m_pComputeShader; };
	inline ID3DBlob*			GetInputSignature()							{ return m_pInputSignature; };

	// Accessors
	inline int GetVSShaderResourceIndex( std::string const &name );
	inline int GetGSShaderResourceIndex( std::string const &name );
	inline int GetPSShaderResourceIndex( std::string const &name );
	inline int GetCSShaderResourceIndex( std::string const &name );
	
	inline const ShaderVariable* GetVSVariable( std::string const &name ) const;
	inline const ShaderVariable* GetGSVariable( std::string const &name ) const;
	inline const ShaderVariable* GetPSVariable( std::string const &name ) const;
	inline const ShaderVariable* GetCSVariable( std::string const &name ) const;
	
	inline int GetVSConstantBufferSize( UINT index )					{ return m_pVSConstantBufferByteSizes.find(index)->second; };
	inline int GetGSConstantBufferSize( UINT index )					{ return m_pGSConstantBufferByteSizes.find(index)->second; };
	inline int GetPSConstantBufferSize( UINT index )					{ return m_pPSConstantBufferByteSizes.find(index)->second; };
	inline int GetCSConstantBufferSize( UINT index )					{ return m_pCSConstantBufferByteSizes.find(index)->second; };
	

	UINT numVSConstantBuffers;
	UINT numGSConstantBuffers;
	UINT numPSConstantBuffers;
	UINT numCSConstantBuffers;

	void Bind( RenderDispatcher* pDispatcher );
	void Unbind( RenderDispatcher* pDispatcher )	{};		// Not needed

	// TODO : implement
	void ReleaseResources()	{};

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

inline INT D3D11Shaderset::GetVSShaderResourceIndex( std::string const &name )	
{
	std::map<std::string, UINT>::const_iterator it = m_pVSShaderResourceIndices.find(name);
	if(it!=m_pVSShaderResourceIndices.end())
		return m_pVSShaderResourceIndices.find(name)->second; 
	else
		return -1; 
}

inline INT D3D11Shaderset::GetGSShaderResourceIndex( std::string const &name )					
{
	std::map<std::string, UINT>::const_iterator it = m_pGSShaderResourceIndices.find(name);
	if(it!=m_pGSShaderResourceIndices.end())
		return m_pGSShaderResourceIndices.find(name)->second;
	else
		return -1;
}

inline INT D3D11Shaderset::GetPSShaderResourceIndex( std::string const &name )
{
	std::map<std::string, UINT>::const_iterator it = m_pPSShaderResourceIndices.find(name);
	if(it!=m_pPSShaderResourceIndices.end())
		return m_pPSShaderResourceIndices.find(name)->second;
	else
		return -1;
}

inline INT D3D11Shaderset::GetCSShaderResourceIndex( std::string const &name )
{
	std::map<std::string, UINT>::const_iterator it = m_pCSShaderResourceIndices.find(name);
	if(it!=m_pCSShaderResourceIndices.end())
		return m_pCSShaderResourceIndices.find(name)->second;
	else
		return -1;
}
	

inline const ShaderVariable* D3D11Shaderset::GetVSVariable( std::string const &name ) const
{
	std::unordered_map<std::string, ShaderVariable>::const_iterator it = m_pVSVariables.find(name);
	if(it!=m_pVSVariables.end())
		return &it->second;
	return NULL;
}

inline const ShaderVariable* D3D11Shaderset::GetGSVariable( std::string const &name ) const
{
	std::unordered_map<std::string, ShaderVariable>::const_iterator it = m_pGSVariables.find(name);
	if(it!=m_pGSVariables.end())
		return &it->second;
	return NULL;
}

inline const ShaderVariable* D3D11Shaderset::GetPSVariable( std::string const &name ) const
{
	std::unordered_map<std::string, ShaderVariable>::const_iterator it = m_pPSVariables.find(name);
	if(it!=m_pPSVariables.end())
		return &it->second;
	return NULL;
}

inline const ShaderVariable* D3D11Shaderset::GetCSVariable( std::string const &name ) const
{
	std::unordered_map<std::string, ShaderVariable>::const_iterator it = m_pCSVariables.find(name);
	if(it!=m_pCSVariables.end())
		return &it->second;
	return NULL;
}
