//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------
#include "EngineGlobal.hlsl"

Texture2D texNoise;
Texture2D gbuffer0;
Texture2D gbuffer1;

cbuffer cbPerObject
{
    float2 ScreenDimensions;
	float2 NoiseScale;
	matrix View;
	matrix Projection;
	matrix InvProjection;
	float2 FovAndAspect;
	float fFarPlane;
};

#define KERNELSIZE 16

static float3 kernel[KERNELSIZE] =
{
	float3(-0.0277914,0.0246995,0.000725406),
	float3(0.0373249,-0.0267497,0.0210379),
	float3(-0.063777,0.0218399,0.0764492),
	float3(-0.015083,0.00492459,0.0434997),
	float3(-0.006603,0.0901832,0.000363945),
	float3(0.0693033,-0.0413271,0.00740598),
	float3(-0.0280094,-0.0408815,0.14357),
	float3(0.106807,-0.0793251,0.106814),
	float3(-0.0653984,-0.145967,0.00027425),
	float3(-0.208626,-0.283179,0.117604),
	float3(0.00789806,0.110024,0.121766),
	float3(0.333863,-0.182643,0.331452),
	float3(-0.289331,-0.103818,0.213273),
	float3(0.313177,0.0246067,0.362649),
	float3(-0.0634927,0.368594,0.418209),
	float3(0.159376,-0.22651,0.149677),
};


//--------------------------------------------------------------------------------------
struct VS_INPUT
{
    float4 Pos : POSITION;
};

struct PS_INPUT
{
    float4 Pos : SV_POSITION;
	float4 VSPos : POSITION1;
};


//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
PS_INPUT VS( VS_INPUT input )
{
    PS_INPUT output = (PS_INPUT)0;
    output.Pos = input.Pos;
	
	float farplane = tan( FovAndAspect.x * 0.5f );
	output.VSPos = float4(input.Pos.x * farplane * FovAndAspect.y, input.Pos.y * farplane, 1.0f, 1.0f);
 
    return output;
}


//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS( PS_INPUT input) : SV_Target
{
	float2 TexCoord = input.Pos.xy/ScreenDimensions.xy;
	
	// Get this pixel's depth
	float depth = gbuffer1.Sample( samLinear, TexCoord*4.0f ).w;

	// Calculate the frustum ray using the view-space position.
    // Negating the Z component only necessary for right-handed coordinates
    float4 vFrustumRayVS = input.VSPos * (fFarPlane/input.VSPos.z);

	// Calculate view space position from linear depth
	float4 gbuff1 = gbuffer1.Sample( samLinear, TexCoord*4.0f);
    float4 VSPos = gbuff1.w * vFrustumRayVS;
	VSPos.w = 1.0f;

	// G-Buffers normals are in world space, so we first transform it to view space (rotation part only)
	float3 normal = mul( (float3x3)View, normalize(gbuff1.xyz) );

	// Next we need to construct a change-of-basis matrix to reorient our sample kernel along the origin's normal. 
	// We can cunningly incorporate the random rotation here, as well
	float3 rvec = texNoise.Sample( samPoint, TexCoord * NoiseScale );
	//float3 tangent = normalize(rvec - normal * dot(rvec, normal));
	float3 tangent = normalize(float3(1.0f, 0.0f, 0.0f) - normal * dot(float3(1.0f, 0.0f, 0.0f), normal));
	float3 bitangent = cross(normal, tangent);
	float3x3 tbn = float3x3(tangent, bitangent, normal);
	
	// Rotation matrix to rotate the sample kernel in along z
	float3 _1 = float3(rvec.x, -rvec.y, 0.0);
	float3 _2 = float3(rvec.y, rvec.x, 0.0);
	float3 _3 = float3(0.0, 0.0, 1.0);
	float3x3 rotation = float3x3(_1, _2, _3);

	// Next we loop through the sample kernel, sample the depth buffer and accumulate the occlusion factor
	float Radius = 3.0f;
	float occlusion = 0.0f;

	for (int i = 0; i < KERNELSIZE; ++i) 
	{
		// Get sample position
		float3 sample = mul(tbn,mul(rotation,-kernel[i]));
		sample = sample * Radius + VSPos.xyz;
  
		// Project sample position
		float4 offset = float4(sample, 1.0);
		offset = mul( Projection, offset );
		offset.xy /= offset.w;
		offset.xy = offset.xy * 0.5 + 0.5;
		offset.x = 1.0f - offset.x;
  
		// Get sample depth
		float sampleDepth = gbuffer1.Sample( samLinearClamp, offset.xy ).w * fFarPlane;
  
		// Range check & accumulate
		float rangeCheck= abs(VSPos.z - sampleDepth) < (Radius) ? 1.0 : 0.1;
		occlusion += (sampleDepth <= sample.z ? 1.0 : 0.0) * rangeCheck;
	}

	// The final step is to normalize the occlusion factor and invert it, 
	// in order to produce a value that can be used to directly scale the light contribution.
	occlusion = 1.0 - (occlusion / KERNELSIZE);

	return occlusion;
}