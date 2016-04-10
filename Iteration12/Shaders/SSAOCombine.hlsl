//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------
#include "EngineGlobal.hlsl"

Texture2D texSSAO;
Texture2D gbuffer0;
Texture2D gbuffer1;

cbuffer cbPerObject
{
    float2 ScreenDimensions;
};


//--------------------------------------------------------------------------------------
struct VS_INPUT
{
    float4 Pos : POSITION;
};

struct PS_INPUT
{
    float4 Pos : SV_POSITION;
};


//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
PS_INPUT VS( VS_INPUT input )
{
    PS_INPUT output = (PS_INPUT)0;
    output.Pos = input.Pos;
 
    return output;
}


//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS( PS_INPUT input) : SV_Target
{
	float2 TexCoord = input.Pos.xy/ScreenDimensions.xy;
	
	float occlusion = texSSAO.Sample( samLinear, TexCoord );
	
	return 0.0025f * occlusion * gbuffer0.Sample( samLinear, TexCoord );
}