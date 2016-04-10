//--------------------------------------------------------------------------------------
// File: Iteration6b.fx
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------
#include "EngineGlobal.hlsl"

cbuffer cbPerObject
{
    matrix World;
	matrix SpotLightViewProjection;
	matrix SpotLightView;
	float fFarPlane;
};


//--------------------------------------------------------------------------------------
struct VS_INPUT
{
    float4 Pos : POSITION;
};

struct PS_INPUT
{
    float4 Pos : SV_POSITION;
	float Depth : WHATEVER;
};


//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
PS_INPUT VS( VS_INPUT input )
{
    PS_INPUT output = (PS_INPUT)0;
    float4 WSPos = mul( input.Pos, World );
    output.Pos = mul( WSPos, SpotLightViewProjection );
	output.Depth = mul( WSPos, SpotLightView ).z/fFarPlane; // View-space linear depth

    return output;
}


//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS( PS_INPUT input) : SV_Target
{
	//return 1;
	//return input.Normal.xyzz;
	//return input.Pos.z/input.Pos.w;
	return input.Depth;
}
