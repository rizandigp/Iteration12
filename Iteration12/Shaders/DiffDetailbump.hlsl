#define NUM_PointLightS 2

//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------
#include "EngineGlobal.hlsl"

Texture2D txDiffuse;
Texture2D txNormal;
Texture2D txDetailNormal;
Texture2D txCookie;
Texture2D txCookie2;
Texture2D txShadowmap;
Texture2D txShadowmap2;

cbuffer cbPerFrame
{
    matrix ViewProjection;
	float4 vEyePos;
};

cbuffer cbPerObject
{
    matrix World;
    matrix NormalMatrix;
	float4 vPointLightPos[NUM_PointLightS];
	float4 vPointLightColor[NUM_PointLightS];
	matrix SpotLightViewProjection[2];
	float4 vSpotLightPos[2];
	float4 vSpotLightColor[2];
	int iSpotLightShadowEnabled[2];
	float2 SpecularParams;
	float DetailTiling;
	float DetailNormalStrength;
};

// MRT output
struct PSOut
{
	float4  frontbuffer : SV_TARGET0;
	float4	gbuffer0 : SV_TARGET1;	// Albedo.xyz	| Metallic
	float4	gbuffer1 : SV_TARGET2;	// Normal.xyz	| Linear Depth
	float4	gbuffer2 : SV_TARGET3;	// Specular		| Glossiness
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
	float4 CSPos[2] : POSITION2;
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
    output.Pos = mul( input.Pos, World );
    output.WSPos = output.Pos;
	output.CSPos[0] = mul( output.Pos, SpotLightViewProjection[0] );
	output.CSPos[1] = mul( output.Pos, SpotLightViewProjection[1] );
    output.Pos = mul( output.Pos, ViewProjection );
    output.Normal.xyz = normalize(mul( input.Normal, (float3x3)NormalMatrix ));
    output.Tangent.xyz = normalize(mul( input.Tangent, (float3x3)NormalMatrix ));
    output.Bitangent.xyz = normalize(mul( input.Bitangent, (float3x3)NormalMatrix ));
    output.Tex = input.Tex;
    
    return output;
}


//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS( PS_INPUT input) : SV_Target
{
	matrix < float,3,3 > matTBN = {	input.Tangent.x,	input.Tangent.y,	input.Tangent.z,		
					input.Bitangent.x,	input.Bitangent.y,	input.Bitangent.z,		
					input.Normal.x,		input.Normal.y,		input.Normal.z,			
					}; 

	float3 normalmap = txNormal.Sample( samLinear, input.Tex )*2.0f - float3( 1.0f, 1.0f, 1.0f );
	//normalmap.z /= 2;
	//normalmap = normalize(normalmap);
	float3 detailnormal = txDetailNormal.Sample( samLinear, input.Tex*DetailTiling )*2.0f - float3( 1.0f, 1.0f, 1.0f );
	detailnormal.z /= DetailNormalStrength;
	detailnormal = normalize( detailnormal );
	//normalmap.xy = -normalmap.xy;
	//normalmap.z = 1.0f;
	normalmap = (normalmap + detailnormal)/2.0f;
	float3 normals = mul( normalmap , matTBN );
	float3 viewdir = vEyePos.xyz - input.WSPos.xyz;
	viewdir = normalize( viewdir );
	float3 l_diffuse = float3(0.0f, 0.0f, 0.0f);//saturate(dot( viewdir, normals ))*0.05f;
	float3 l_specular = float3(0.0f, 0.0f, 0.0f);//pow(saturate(dot( viewdir, normals )), 12)*0.05f;
	
	float3 diff, spec;
	float3 lightvec;
	float dist_attenuation;
	//float ndotv = 1.8f*(1.0f-saturate(dot( normals, viewdir ))*0.4f);
	//float normalization = ((SpecularParams.y,ndotv) + 2)/(3.14*2.0f);
	
	// Omni lights
	[unroll] for(int i=0; i<NUM_PointLightS; i++)
	{
	float3 lightvec = vPointLightPos[i].xyz-input.WSPos.xyz;
	dist_attenuation = 1-length(lightvec)/vPointLightPos[i].a;
	diff = saturate( dot( normalize( lightvec ), normals ));
	spec = blinnphong_specular( normalize( lightvec ), normals, viewdir, SpecularParams.y )*diff;
	diff *= vPointLightColor[i].xyz/(length(lightvec)*length(lightvec)+1);
	diff *= dist_attenuation;
	//spec *= normalization;
	spec *= SpecularParams.x*vPointLightColor[i].xyz/(length(lightvec)*length(lightvec));
	spec *= dist_attenuation;
	l_diffuse += max(diff,0.0f);
	l_specular += max(spec,0.0f);;
	}

	float2 ShadowTexC = 0.5f * input.CSPos[0].xy / input.CSPos[0].ww + float2( 0.5f, 0.5f );
	ShadowTexC.y = 1.0f - ShadowTexC.y;

	if (!(ShadowTexC.x <= 0.0f || ShadowTexC.x >= 1.0f || ShadowTexC.y <= 0.0f || ShadowTexC.y >= 1.0f || input.CSPos[0].w < 0))
	{
		float2 ditheredUV = ShadowTexC;// + (frac(input.Pos.xy * 0.5) > 0.5)/2048.0f;
		float shadowing = 1.0f;
		if (iSpotLightShadowEnabled[0]!=0)	{
		shadowing = txShadowmap.SampleCmp( samComp, ditheredUV, input.CSPos[0].w-SHADOWMAP_BIAS );
		shadowing += txShadowmap.SampleCmp( samComp, ditheredUV, input.CSPos[0].w-SHADOWMAP_BIAS, int2(-1,-1) );
		//shadowing += txShadowmap.SampleCmp( samComp, ditheredUV, input.CSPos[0].w-SHADOWMAP_BIAS, int2(0,-1) );
		shadowing += txShadowmap.SampleCmp( samComp, ditheredUV, input.CSPos[0].w-SHADOWMAP_BIAS, int2(1,-1) );
		//shadowing += txShadowmap.SampleCmp( samComp, ditheredUV, input.CSPos[0].w-SHADOWMAP_BIAS, int2(-1,0) );
		//shadowing += txShadowmap.SampleCmp( samComp, ditheredUV, input.CSPos[0].w-SHADOWMAP_BIAS, int2(1,0) );
		shadowing += txShadowmap.SampleCmp( samComp, ditheredUV, input.CSPos[0].w-SHADOWMAP_BIAS, int2(-1,1) );
		//shadowing += txShadowmap.SampleCmp( samComp, ditheredUV, input.CSPos[0].w-SHADOWMAP_BIAS, int2(0,1) );
		shadowing += txShadowmap.SampleCmp( samComp, ditheredUV, input.CSPos[0].w-SHADOWMAP_BIAS, int2(1,1) );
		shadowing /= 5.0f;	}

		lightvec = vSpotLightPos[0].xyz-input.WSPos.xyz;
		dist_attenuation = 1-length(lightvec)/vSpotLightPos[0].a;
		diff = saturate( dot( normalize( lightvec ), normals ));
		//spec = blinnphong_specular( normalize( lightvec ), normals, viewdir, SpecularParams.y )*diff;
		spec = cooktorrance_specular( normalize( lightvec ), normals, viewdir, SpecularParams.y, SpecularParams.x )*diff;
		diff *= vSpotLightColor[0].xyz/(length(lightvec)*length(lightvec)+1);
		diff *= dist_attenuation;
		//spec = blinnphong_specular( normalize( lightvec ), normals, viewdir, SpecularParams.y );
		//spec *= SpecularParams.x*vSpotLightColor[0].xyz/(length(lightvec)*length(lightvec)+1);
		spec *= vSpotLightColor[0].xyz/(length(lightvec)*length(lightvec)+1);
		spec *= dist_attenuation;
		l_diffuse += max(diff,0.0f)*pow(txCookie.Sample( samLinear, ShadowTexC ),DEGAMMA) * shadowing;
		l_specular += max(spec,0.0f)*pow(txCookie.Sample( samLinear, ShadowTexC ),DEGAMMA) * shadowing;
	}


	ShadowTexC = 0.5f * input.CSPos[1].xy / input.CSPos[1].ww + float2( 0.5f, 0.5f );
	ShadowTexC.y = 1.0f - ShadowTexC.y;
	
	if (!(ShadowTexC.x <= 0.0f || ShadowTexC.x >= 1.0f || ShadowTexC.y <= 0.0f || ShadowTexC.y >= 1.0f || input.CSPos[1].w < 0))

	{
		float2 ditheredUV = ShadowTexC;// + (frac(input.Pos.xy * 0.5) > 0.5)/2048.0f;
		float shadowing = 1.0f;
		if (iSpotLightShadowEnabled[1]!=0)	{
		shadowing = txShadowmap2.SampleCmp( samComp, ditheredUV, input.CSPos[1].w-SHADOWMAP_BIAS );
		shadowing += txShadowmap2.SampleCmp( samComp, ditheredUV, input.CSPos[1].w-SHADOWMAP_BIAS, int2(-1,-1) );
		//shadowing += txShadowmap2.SampleCmp( samComp, ditheredUV, input.CSPos[1].w-SHADOWMAP_BIAS, int2(0,-1) );
		shadowing += txShadowmap2.SampleCmp( samComp, ditheredUV, input.CSPos[1].w-SHADOWMAP_BIAS, int2(1,-1) );
		//shadowing += txShadowmap2.SampleCmp( samComp, ditheredUV, input.CSPos[1].w-SHADOWMAP_BIAS, int2(-1,0) );
		//shadowing += txShadowmap2.SampleCmp( samComp, ditheredUV, input.CSPos[1].w-SHADOWMAP_BIAS, int2(1,0) );
		shadowing += txShadowmap2.SampleCmp( samComp, ditheredUV, input.CSPos[1].w-SHADOWMAP_BIAS, int2(-1,1) );
		//shadowing += txShadowmap2.SampleCmp( samComp, ditheredUV, input.CSPos[1].w-SHADOWMAP_BIAS, int2(0,1) );
		shadowing += txShadowmap2.SampleCmp( samComp, ditheredUV, input.CSPos[1].w-SHADOWMAP_BIAS, int2(1,1) );
		shadowing /= 5.0f;	}

		lightvec = vSpotLightPos[1].xyz-input.WSPos.xyz;
		dist_attenuation = 1-length(lightvec)/vSpotLightPos[1].a;
		diff = saturate( dot( normalize( lightvec ), normals ));
		//spec = blinnphong_specular( normalize( lightvec ), normals, viewdir, SpecularParams.y )*diff;
		spec = cooktorrance_specular( normalize( lightvec ), normals, viewdir, SpecularParams.y, SpecularParams.x )*diff;
		diff *= vSpotLightColor[1].xyz/(length(lightvec)*length(lightvec)+1);
		diff *= dist_attenuation;
		//spec = blinnphong_specular( normalize( lightvec ), normals, viewdir, SpecularParams.y );
		//spec *= SpecularParams.x*vSpotLightColor[1].xyz/(length(lightvec)*length(lightvec)+1);
		spec *= vSpotLightColor[1].xyz/(length(lightvec)*length(lightvec)+1);
		spec *= dist_attenuation;
		l_diffuse += max(diff,0.0f)*pow(txCookie2.Sample( samLinear, ShadowTexC ),DEGAMMA) * shadowing;
		l_specular += max(spec,0.0f)*pow(txCookie2.Sample( samLinear, ShadowTexC ),DEGAMMA) * shadowing;
	}
	
	//return input.Normal.xyzz;
	//return (normals.xyzz+float4(0.5f,0.5f,0.5f,0.5f))/2.0f;
	//return l_diffuse.xyzz*0.18f + l_specular.xyzz;
	//return l_diffuse.xyzz + l_specular.xyzz;
	//return ( txDiffuse.Sample( samLinear, input.Tex )* l_diffuse.xyzz + l_specular.xyzz );
	//return pow( (l_diffuse.xyzz*0.18f + l_specular.xyzz), GAMMA );
	//return pow( (pow(txDiffuse.Sample( samLinear, input.Tex ),DEGAMMA) *l_diffuse.xyzz ), GAMMA ) + l_specular.xyzz;
	//return pow( (pow(txDiffuse.Sample( samLinear, input.Tex ),DEGAMMA) *l_diffuse.xyzz + l_specular.xyzz), GAMMA );
	//return txDiffuse.Sample( samLinear, input.Tex );
	return (pow(txDiffuse.Sample( samLinear, input.Tex ),DEGAMMA) *l_diffuse.xyzz + l_specular.xyzz);
	/*
	PSOut output;
	output.frontbuffer = (pow(txDiffuse.Sample( samLinear, input.Tex ),DEGAMMA) *l_diffuse.xyzz + l_specular.xyzz);

	return output;*/
}
