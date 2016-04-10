//#include "EngineGlobal.hlsl"
#include "LightingCommon.hlsl"

#define SPECULAR_POWER_SCALE 2048.0f

Texture2D txGBuffer0;
Texture2D txGBuffer1;
Texture2D txGBuffer2;

//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------
cbuffer cbPerFrame
{
    matrix ViewProjection;
};

cbuffer cbPerObject
{
	matrix InvViewProjection;
	float4 vColor;
	float4 vLightVector;
	float4 vEyePos;
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
	float2 ScreenCoord = input.Pos.xy/ScreenDimensions.xy;

	// Get clip space position
	float z = txGBuffer1.Sample( samLinear, ScreenCoord ).w;
	float x = ScreenCoord.x * 2 - 1;
    float y = (1 - ScreenCoord.y) * 2 - 1;

	// Un-project from clip space into world space
	float4 ProjectedPos = float4(x, y, z, 1.0f );
	float4 WSPos = mul(ProjectedPos, InvViewProjection );
	WSPos.xyz = WSPos.xyz/WSPos.w;
	WSPos.w = 1.0f;

	// Calculate diffuse & specular lighting
	float4 Specmap = txGBuffer2.Sample( samLinear, ScreenCoord );
	//float power = pow(SPECULAR_POWER_SCALE,Specmap.w);
	float3 normals = txGBuffer1.Sample( samLinear, ScreenCoord ).xyz;
	float3 lightvec = vLightVector.xyz;
	float3 viewdir = normalize(vEyePos.xyz - WSPos.xyz);

	//float normalization = (power + 4.0f )/(8.0f);
	//float4 specular = blinnphong_specular( normalize( lightvec ), normals, viewdir, power )*diffuse;
	//float4 specular = ggx_specular( normalize( lightvec ), normals, viewdir, 1.0f-Specmap.w, Specmap.x )*diffuse;
	//float4 specular = cooktorrance_specular( normalize( lightvec ), normals, viewdir, power, Specmap.x );

	float NdotL = saturate( dot( normalize( lightvec ), normals ));
	float NdotH = saturate(dot(normals, normalize(normalize( lightvec )+viewdir)));
	float VdotH = saturate(dot(viewdir, normalize(normalize( lightvec )+viewdir)));
	float NdotV = saturate(dot(viewdir, normals));

	float3 specular = D_GGX( 1.0f-Specmap.w, NdotH ) * F_Schlick(vColor.xyzw*Specmap.xyz, NdotV ) * Vis_Implicit();//Vis_Smith( 1.0f-Specmap.w, NdotV, NdotL );
	//float3 diffuse = Diffuse_Lambert((pow(txGBuffer0.Sample( samLinear, ScreenCoord ),DEGAMMA))*vColor.xyz);
	float3 diffuse = Diffuse_Burley( (pow(txGBuffer0.Sample( samLinear, ScreenCoord ),DEGAMMA))*vColor.xyz, 1.0f-Specmap.w, NdotV, NdotL, VdotH );
	//float3 diffuse = Diffuse_OrenNayar( (pow(txGBuffer0.Sample( samLinear, ScreenCoord ),DEGAMMA))*vColor.xyz, 1.0f-Specmap.w, NdotV, NdotL, VdotH );

	//diffuse *= vColor.xyzw;
	//specular *= vColor.xyzw*Specmap.xyzw;
	//specular *= normalization;
	diffuse = max(diffuse*NdotL,0.0f);
	specular = max(specular*NdotL,0.0f);
	
	return float4(diffuse,0.0f) + float4(specular,0.0f);
}