//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------
#include "EngineGlobal.hlsl"

#define SPECULAR_POWER_SCALE 2048.0f

Texture2D txGBuffer0;
Texture2D txGBuffer1;
Texture2D txGBuffer2;
Texture2D txShadowmap;
Texture2D txCookie;

cbuffer cbPerFrame
{
    matrix ViewProjection;
};

cbuffer cbPerObject
{
    matrix World;
	matrix InvViewProjection;
	matrix SpotlightView;
	matrix SpotlightViewProjection;
	matrix InvSpotlightProjection;
	float4 vSpotLightColor;
	float4 vEyePos;
	float2 ScreenDimensions;
	float SpotLightRadius; // Radius is also the far plane
};


//--------------------------------------------------------------------------------------
struct VS_INPUT
{
    float4 Pos : POSITION;
};

struct PS_INPUT
{
    float4 Pos : SV_POSITION;
	float4 Pos : POSITION1;
};


//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
PS_INPUT VS( VS_INPUT input )
{
    PS_INPUT output = (PS_INPUT)0;
	output.Pos = mul( input.Pos, InvSpotlightProjection);
    output.Pos = mul( output.Pos, World );
	output.VSPos = mul( output.Pos, View );
    output.Pos = mul( output.Pos, ViewProjection );

    return output;
}


//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS( PS_INPUT input) : SV_Target
{
	float2 ScreenCoord = input.Pos.xy/ScreenDimensions.xy;
	
	// Get clip space position
	float z = txGBuffer1.Sample( samLinear, ScreenCoord ).w;
	float x = ScreenCoord.x * 2 - 1;
    float y = (1 - ScreenCoord.y) * 2 - 1;

	// Un-project from clip space back into world space
	float4 ProjectedPos = float4(x, y, z, 1.0f );
	float4 WSPos = mul(ProjectedPos, InvViewProjection );
	WSPos.xyz = WSPos.xyz/WSPos.w;
	WSPos.w = 1.0f;

	//Then project to spotlight's clip space
	float4 SpotlightCSPos = mul( WSPos, SpotlightViewProjection );

	// Transform from clip space to normalized device coordinates
	float2 SpotlightUV = 0.5f * SpotlightCSPos.xy / SpotlightCSPos.ww + float2( 0.5f, 0.5f );
	SpotlightUV.y = 1.0f - SpotlightUV.y;
	
	// Depth for comparison is linear in View Space
	// TODO : Optimize with Crytek's method
	float4 VS = mul( WSPos, SpotlightView );
	SpotlightCSPos.w = VS.z/SpotLightRadius; // Radius is also the far plane

	// Only light this pixel if it's inside the spotlight's frustum
	if (!(SpotlightUV.x <= 0.0f || SpotlightUV.x >= 1.0f || SpotlightUV.y <= 0.0f || SpotlightUV.y >= 1.0f))
	{
		// Sample shadowmap
		float shadowing = txShadowmap.SampleCmp( samComp, SpotlightUV, SpotlightCSPos.w-SHADOWMAP_BIAS );
		shadowing += txShadowmap.SampleCmp( samComp, SpotlightUV, SpotlightCSPos.w-SHADOWMAP_BIAS, int2(-1,-1) );
		//shadowing += txShadowmap.SampleCmp( samComp, SpotlightUV, SpotlightCSPos.w-SHADOWMAP_BIAS, int2(0,-1) );
		shadowing += txShadowmap.SampleCmp( samComp, SpotlightUV, SpotlightCSPos.w-SHADOWMAP_BIAS, int2(1,-1) );
		//shadowing += txShadowmap.SampleCmp( samComp, SpotlightUV, SpotlightCSPos.w-SHADOWMAP_BIAS, int2(-1,0) );
		//shadowing += txShadowmap.SampleCmp( samComp, SpotlightUV, SpotlightCSPos.w-SHADOWMAP_BIAS, int2(1,0) );
		shadowing += txShadowmap.SampleCmp( samComp, SpotlightUV, SpotlightCSPos.w-SHADOWMAP_BIAS, int2(-1,1) );
		//shadowing += txShadowmap.SampleCmp( samComp, SpotlightUV, SpotlightCSPos.w-SHADOWMAP_BIAS, int2(0,1) );
		shadowing += txShadowmap.SampleCmp( samComp, SpotlightUV, SpotlightCSPos.w-SHADOWMAP_BIAS, int2(1,1) );
		shadowing /= 5.0f;

		// Variables for lighting
		float4 Cookie = pow(txCookie.Sample( samLinear, SpotlightUV ),GAMMATOLINEAR);
		float4 Specmap = txGBuffer2.Sample( samLinear, ScreenCoord );
		float power = pow(SPECULAR_POWER_SCALE,Specmap.w);	// Specular power
		float3 normal = txGBuffer1.Sample( samLinear, ScreenCoord ).xyz;
		float3 lightPos = float3( World._41, World._42, World._43 );
		float3 lightVec = lightPos.xyz - WSPos.xyz;
		float3 viewVec = normalize(vEyePos.xyz - WSPos.xyz);
		float dist_attenuation = (1-length(lightVec)/SpotLightRadius)/(length(lightVec)*length(lightVec)+1); // Inverse squared with a linear limiting factor
		float normalization = (power + 4.0f )/(8.0f);	// Blinn-phong normalization factor

		// Calculate diffuse & specular lighting
		float4 diffuse = saturate( dot( normalize( lightVec ), normal )); // Lambert
		//float4 specular = blinnphong_specular( normalize( lightVec ), normal, viewVec, power )*diffuse;
		float4 specular = cooktorrance_specular( normalize( lightVec ), normal, viewVec, power, Specmap.x );
		diffuse *= vSpotLightColor.xyzw;
		diffuse *= dist_attenuation;
		specular *= Specmap.xyzw*vSpotLightColor.xyzw;
		specular *= dist_attenuation;
		specular *= normalization;

		diffuse = max(diffuse,0.0f)*Cookie*shadowing;
		specular = max(specular,0.0f)*Cookie*shadowing;
		
		// Multiply with albedo and return
		return (pow(txGBuffer0.Sample( samLinear, ScreenCoord ),GAMMATOLINEAR))*diffuse + specular; //SpotlightCSPos.w;//txShadowmap.Sample( samLinear, SpotlightUV ).x;////
	}

	return 0.0f;
}