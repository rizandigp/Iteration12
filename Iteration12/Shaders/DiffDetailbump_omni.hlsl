#define NUM_PointLightS 2

//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------
#include "EngineGlobal.hlsl"

Texture2D txDiffuse;
Texture2D txNormal;
Texture2D txDetailNormal;

cbuffer cbPerFrame
{
    matrix ViewProjection;
	float4 vEyePos;
};

cbuffer cbPerObject
{
    matrix World;
    matrix NormalMatrix;
    //float4 vMeshColor;
	float4 vPointLightPos[NUM_PointLightS];
	float4 vPointLightColor[NUM_PointLightS];
	float2 SpecularParams;
	float DetailTiling;
	float DetailNormalStrength;
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
};


//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
PS_INPUT VS( VS_INPUT input )
{
    PS_INPUT output = (PS_INPUT)0;
    output.Pos = mul( input.Pos, World );
    output.WSPos = output.Pos;
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
	// Omni lights
	[unroll] for(int i=0; i<NUM_PointLightS; i++)
	{
	float3 lightvec = vPointLightPos[i].xyz-input.WSPos.xyz;
	diff = saturate( dot( normalize( lightvec ), normals ));
	spec = pow(saturate( dot( normalize( reflect( normalize( -lightvec ), normals ) ), viewdir )), SpecularParams.y)*SpecularParams.x*diff*vPointLightColor[i].xyz/(length(lightvec)*length(lightvec)+1);
	diff *= vPointLightColor[i].xyz/(length(lightvec)*length(lightvec)+1);
	diff -= diff*length(lightvec)/vPointLightPos[i].a;
	//spec *= normalization;
	spec -= spec*length(lightvec)/vPointLightPos[i].a;
	l_diffuse += max(diff,0.0f);
	l_specular += max(spec,0.0f);;
	}
	
	//return input.Normal.xyzz;
	//return (normals.xyzz+float4(0.5f,0.5f,0.5f,0.5f))/2.0f;
	//return ( txDiffuse.Sample( samLinear, input.Tex )* l_diffuse.xyzz + l_specular.xyzz );
	//return pow( (pow(txDiffuse.Sample( samLinear, input.Tex ),DEGAMMA) *l_diffuse.xyzz + l_specular.xyzz), GAMMA );
	return (pow(txDiffuse.Sample( samLinear, input.Tex ),DEGAMMA) *l_diffuse.xyzz + l_specular.xyzz);
}
