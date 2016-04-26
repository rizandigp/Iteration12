//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------
#include "LightingCommon.hlsl"

TextureCube texSource;

cbuffer cbPerFrame
{
    matrix ViewProjection;
};

cbuffer cbPerObject
{
    matrix World;
	float2 vDimensions;
	float fRoughness;
	float N;
};

float2 importance_sample_phong(float n, float2 xi)
{
  float phi = 2.0f * PI * xi.x;
  float theta = acos(pow(1.0f - xi.y, 1.0f/(n+1.0f)));
  return float2(phi, theta);
}

float2 importance_sample_ggx(float a, float2 xi)
{
  float phi = 2.0f * PI * xi.x;
  float theta = acos(sqrt((1.0f - xi.y)/
                          ((a*a - 1.0f) * xi.y + 1.0f)
                         ));
  return float2(phi, theta);
}

float3x3 makeRotationDir(float3 direction, float3 up)
{
    float3 xaxis = normalize(cross(up, direction));

    float3 yaxis = normalize(cross(direction, xaxis));

	float3x3 mat;

    mat[0][0] = xaxis.x;
    mat[1][0] = yaxis.x;
    mat[2][0] = direction.x;

    mat[0][1] = xaxis.y;
    mat[1][1] = yaxis.y;
    mat[2][1] = direction.y;

    mat[0][2] = xaxis.z;
    mat[1][2] = yaxis.z;
    mat[2][2] = direction.z;

	return mat;
}

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
	float2 ScreenCoord = input.Pos.xy/vDimensions*2.0f - 1.0f;
	ScreenCoord.y = -ScreenCoord.y;

	float4 result = 0;
	float p = 0.0f;

	for(int i=0; i<N; i++)
	{
		for(int j=0; j<N; j++)
		{
			float2 phi_theta = importance_sample_ggx( fRoughness, float2(i/N, (j+0.25f)/(N*1.5f)) );
			float3 sample = float3( cos(phi_theta.x)*sin(phi_theta.y), sin(phi_theta.x)*sin(phi_theta.y), cos(phi_theta.y) );
			float3 direction = normalize(float3(ScreenCoord.x, ScreenCoord.y, 1.0f));
			float3 local = mul( sample, makeRotationDir( direction, float3(0.0f, 1.0f, 0.0f) ) );
			float3 world = mul( local, (float3x3)World );

			result += texSource.SampleLevel( samLinear, world, 0.0f ) * D_GGX( fRoughness, cos(phi_theta.y) ) * cos(phi_theta.y); // cos(phi_theta.y) equals NdotL
			p += D_GGX( fRoughness, cos(phi_theta.y) );
		}
	}
	
	// Center sample
	float3 direction = normalize(float3(ScreenCoord.x, ScreenCoord.y, 1.0f));
	float3 world = mul( direction, (float3x3)World );
	result += texSource.SampleLevel( samLinear, world, 0.0f )*D_GGX( fRoughness, 1.0f );
	p += D_GGX( fRoughness, 1.0f );
	
	return result/(p);
}
