//--------------------------------------------------------------------------------------
// File: Iteration6b.fx
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------
#include "EngineGlobal.hlsl"

Texture2D txDiffuse;
Texture2D txNormal;
Texture2D txCookie;

cbuffer cbPerObject
{
    matrix World;
	matrix SpotLightViewProjection;
	matrix SpotLightView;
    matrix NormalMatrix;
	float2 vResolution;
	float fFarPlane;
};


//--------------------------------------------------------------------------------------
struct VS_INPUT
{
    float4 Pos : POSITION;
    float2 Tex : TEXCOORD0;
	float4 Normal : NORMAL;
	float4 Tangent : TANGENT;
	float4 Bitangent : BITANGENT;
};

struct PS_INPUT
{
    float4 Pos : SV_POSITION;
	float4 WSPos : POSITION1;
    float2 Tex : TEXCOORD0;
    float4 Normal : NORMAL;
	float4 Tangent : TANGENT;
    float4 Bitangent : BITANGENT;
	float Depth : WHATEVER;
};

// MRT output
struct PSOut
{
	float4  depth : SV_TARGET0;
	float4	normal : SV_TARGET1;
	float4	color : SV_TARGET2;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
PS_INPUT VS( VS_INPUT input )
{
    PS_INPUT output = (PS_INPUT)0;
    output.WSPos = mul( input.Pos, World );
	output.Depth = mul( output.WSPos, SpotLightView ).z/fFarPlane; // View-space linear depth
    output.Pos = mul( output.WSPos, SpotLightViewProjection );
    output.Normal.xyz = mul( input.Normal, (float3x3)NormalMatrix );
    output.Tangent.xyz = mul( input.Tangent, (float3x3)NormalMatrix );
    output.Bitangent.xyz = mul( input.Bitangent, (float3x3)NormalMatrix );
    output.Tex = input.Tex;

    return output;
}


//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
PSOut PS( PS_INPUT input) : SV_Target
{
	input.Tangent.xyz = normalize(input.Tangent.xyz);
	input.Bitangent.xyz = normalize(input.Bitangent.xyz);
	input.Normal.xyz = normalize(input.Normal.xyz);
	matrix < float,3,3 > matTBN = {	input.Tangent.x,	input.Tangent.y,	input.Tangent.z,		
									input.Bitangent.x,	input.Bitangent.y,	input.Bitangent.z,		
									input.Normal.x,		input.Normal.y,		input.Normal.z,			
									};

	float3 normalmap = ( txNormal.Sample( samLinear, input.Tex )*2.0f - 1.0f );
	normalmap = normalize( normalmap );		// somewhat helps with compression artefacts

	float3 normals = mul( normalmap , matTBN );

	float2 SpotlightUV = input.Pos.xy/512.0f; // Not sure where the number 512 comes from, but it works

	PSOut output;
	output.depth.x = input.Depth;
	output.color = pow(txDiffuse.Sample( samLinear, input.Tex ),GAMMATOLINEAR)*txCookie.Sample( samLinear, SpotlightUV );
	output.normal = normals.xyzz*0.5f+0.5f;//txNormal.Sample( samLinear, input.Tex );//normals.xyzz;

	return output;
}
