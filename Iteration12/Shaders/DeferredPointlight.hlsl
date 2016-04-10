//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------
#include "EngineGlobal.hlsl"

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
	matrix InvProjection;
	matrix InvViewProjection;
    	matrix World;
	float4 vPointLightColor;
	float4 vEyePos;
	float2 ScreenDimensions;
	float PointLightRadius;
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
    output.Pos = mul( input.World );
    output.Pos = mul( output.Pos, ViewProjection );

    return output;
}


//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS( PS_INPUT input) : SV_Target
{
	float2 ScreenCoord = input.Pos.xy/ScreenDimensions.xy;

	// Get linear depth
	float z = txGBuffer1.Sample( samLinear, ScreenCoord ).w;
	float x = ScreenCoord.x * 2 - 1;
    	float y = (1 - ScreenCoord.y) * 2 - 1;

	// Un-project from clip space to view-space position
	float4 ProjectedPos = float4(x, y, z, 1.0f );
	float4 VSPos = mul(ProjectedPos, InvProjection );
	
	// Un-project into world space
	float4 WSPos = mul(ProjectedPos, InvViewProjection );
	WSPos.xyz = WSPos.xyz/WSPos.w;
	WSPos.w = 1.0f;
	
	// Calculate diffuse & specular lighting
	float4 Specmap = txGBuffer2.Sample( samLinear, ScreenCoord );
	float power = pow(1024.0f,Specmap.w);
	float3 normals = txGBuffer1.Sample( samLinear, ScreenCoord ).xyz;
	float3 lightPos = float3( World._41, World._42, World._43 );
	float3 lightvec = lightPos.xyz - WSPos.xyz;
	float3 viewdir = normalize(vEyePos.xyz - WSPos.xyz);
	float normalization = (power + 4.0f )/(8.0f);
	float dist_attenuation = 1-length(lightvec)/PointLightRadius;//lightPos.w;
	float4 diff = saturate( dot( normalize( lightvec ), normals ));
	float4 spec = blinnphong_specular( normalize( lightvec ), normals, viewdir, power )*diff;
	diff *= vPointLightColor.xyzw/(length(lightvec)*length(lightvec)+1);
	diff *= dist_attenuation;
	spec *= Specmap.xyzw*vPointLightColor.xyzw/(length(lightvec)*length(lightvec)+1);
	spec *= dist_attenuation;
	spec *= normalization;
	float4 l_diffuse = max(diff,0.0f);//*pow(txCookie.Sample( samLinear, ShadowTexC ),DEGAMMA)*shadowing;
	float4 l_specular = max(spec,0.0f);//*pow(txCookie.Sample( samLinear, ShadowTexC ),DEGAMMA)*shadowing;

	return (pow(txGBuffer0.Sample( samLinear, ScreenCoord ),DEGAMMA))*l_diffuse + l_specular;
}