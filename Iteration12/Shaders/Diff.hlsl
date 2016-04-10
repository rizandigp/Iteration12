#define NUM_PointLightS 2

//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------
#include "EngineGlobal.hlsl"

Texture2D txDiffuse;

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
	//float2 SpecularParams;
};


//--------------------------------------------------------------------------------------
struct VS_INPUT
{
    float4 Pos : POSITION;
    float2 Tex : TEXCOORD0;
	float4 Normal : NORMAL;
};

struct PS_INPUT
{
    float4 Pos : SV_POSITION;
    float4 WSPos : POSITION1;
    float2 Tex : TEXCOORD0;
    float4 Normal : NORMAL;
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
    output.Tex = input.Tex;
    
    return output;
}


//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS( PS_INPUT input) : SV_Target
{
	float3 normals = normalize( input.Normal.xyz );
	float3 viewdir = vEyePos.xyz - input.WSPos.xyz;
	viewdir = normalize( viewdir );
	float3 l_diffuse = float3(0.0f,0.0f,0.0f);// = saturate(dot( viewdir, normals ))*0.05f;
	float3 l_specular = float3(0.0f,0.0f,0.0f);// = pow(saturate(dot( viewdir, normals )), 12)*0.01f;
	
	// Omni lights
	[unroll] for(int i=0; i<NUM_PointLightS; i++)
	{
	float3 lightvec = vPointLightPos[i].xyz-input.WSPos.xyz;
	l_diffuse += saturate(dot( normalize( lightvec ), normals ))*vPointLightColor[i].xyz/(pow(length(lightvec),1)+1);
	l_specular += pow(saturate(dot( normalize( reflect( normalize( -lightvec ), normals ) ), viewdir )), 15)*0.15f*vPointLightColor[i].xyz/(pow(length(lightvec),1)+1);
	}
	
	//return input.Normal.xyzz;
	//return ( txDiffuse.Sample( samLinear, input.Tex ) *l_diffuse.xyzz + l_specular.xyzz );
	return (pow(txDiffuse.Sample( samLinear, input.Tex ),DEGAMMA) *l_diffuse.xyzz + l_specular.xyzz);
}
