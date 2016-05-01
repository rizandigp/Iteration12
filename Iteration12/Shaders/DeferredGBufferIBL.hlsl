//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------
#include "EngineGlobal.hlsl"

Texture2D txDiffuse;
Texture2D txNormal;
Texture2D txSpecular;
Texture2D txAO;
TextureCube txIBL;

cbuffer cbPerFrame
{
	matrix View;
	matrix ViewProjection;
	float4 vEyePos;
};

cbuffer cbPerObject
{
    matrix World;
    matrix NormalMatrix;
	float2 SpecularParams;
	float fFarPlane;
};

// MRT output
struct PSOut
{
	float4  backbuffer : SV_TARGET0;
	float4	gbuffer0 : SV_TARGET0;	// Albedo.xyz	| Metallic
	float4	gbuffer1 : SV_TARGET1;	// Normal.xyz	| Linear Depth
	float4	gbuffer2 : SV_TARGET2;	// Specular.xyz	| Glossiness
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
	float4 VSPos : POSITION2;
    float2 Tex : TEXCOORD0;
    float4 Normal : NORMAL;
	float4 Tangent : TANGENT;
    float4 Bitangent : BITANGENT;
};


//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
PS_INPUT VS( VS_INPUT input )
{
    PS_INPUT output = (PS_INPUT)0;
    output.WSPos = mul( input.Pos, World );
	output.VSPos = mul( output.WSPos, View );
    output.Pos = mul( output.WSPos, ViewProjection );
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
	// Some models have screwed up/unnormalized tangent spaces
	input.Tangent.xyz = normalize(input.Tangent.xyz);
	input.Bitangent.xyz = normalize(input.Bitangent.xyz);
	input.Normal.xyz = normalize(input.Normal.xyz);
	
	matrix < float,3,3 > matTBN = {	input.Tangent.x,	input.Tangent.y,	input.Tangent.z,		
									input.Bitangent.x,	input.Bitangent.y,	input.Bitangent.z,		
									input.Normal.x,		input.Normal.y,		input.Normal.z,			
									};
									
	float3 Normalmap = ( txNormal.Sample( samLinear, input.Tex )*2.0f - float3( 1.0f, 1.0f, 1.0f ) );
	Normalmap = normalize( Normalmap );		// somewhat helps with compression artefacts
	
	float specular = txSpecular.Sample( samLinear, input.Tex ).g;//*SpecularParams.x;
	float gloss = txSpecular.Sample( samLinear, input.Tex ).r*0.95f;
	float ao = pow(txAO.Sample( samLinear, input.Tex ).r,GAMMATOLINEAR*1.5f);
	float3 normals = mul( Normalmap , matTBN );
	float3 viewvector = normalize(vEyePos.xyz - input.WSPos.xyz);
	float3 reflectionvector = reflect( -viewvector, normals ).xzy;
	float fresnel = pow( 1.0f - saturate( dot( viewvector, normals ) ), 5.0f );
    fresnel *= ( 1.0f - specular );
    fresnel += specular;

	PSOut output;
	// Diffuse part of the environment lighting
	output.backbuffer = ao*pow(txDiffuse.Sample( samLinear, input.Tex ),GAMMATOLINEAR)*txIBL.SampleLevel( samLinear, normals.xzy, 7.5f );
	// Specular part of the environment lighting
	output.backbuffer += (1.0/PI)*ao*fresnel*txIBL.SampleLevel( samLinear, reflectionvector, (1.0f-gloss)*10.0f );
	
	// G-Buffer outputs
	output.gbuffer0.xyz = txDiffuse.Sample( samLinear, input.Tex ).xyz;	// Gamma-space albedo
	output.gbuffer1.xyz = normals.xyz;	// World space normals
	output.gbuffer1.w = -input.VSPos.z/fFarPlane; // View space linear depth
	output.gbuffer2.xyz = txSpecular.Sample( samLinear, input.Tex ).g;//*SpecularParams.x;
	output.gbuffer2.w = gloss;

	return output;
}
