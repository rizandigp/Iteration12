//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------
#include "EngineGlobal.hlsl"

Texture2D texSource;
Texture2D gbuffer0;
Texture2D gbuffer1;
Texture2D gbuffer2;

cbuffer cbPerObject
{
    float2 ScreenDimensions;
	float fExposure;
};

//#define TONEMAPPING_OPERATOR 2

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
	float3 source = texSource.Sample( samLinear, TexCoord )*fExposure;

#if TONEMAPPING_OPERATOR == 1
	return float4( FilmicTonemap( source ), 0.0f );	// Looks really good
#elif TONEMAPPING_OPERATOR == 2
	return float4( ReinhardTonemap( source ), 0.0f );
#elif TONEMAPPING_OPERATOR == 3
	return float4( Uncharted2Tonemap( source ), 0.0f );
#endif

	return float4( LinearTonemap( source ), 0.0f );
}