#pragma once

#include "Prerequisites.h"
#include "GPUResource.h"


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