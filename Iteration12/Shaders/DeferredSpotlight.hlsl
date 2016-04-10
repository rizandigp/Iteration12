//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------
#include "LightingCommon.hlsl"

#define SPECULAR_POWER_SCALE 2048.0f

Texture2D txGBuffer0;
Texture2D txGBuffer1;
Texture2D txGBuffer2;
Texture2D txShadowmap;
Texture2D txCookie;

cbuffer cbPerFrame
{
	matrix View;
    matrix ViewProjection;
};

cbuffer cbPerObject
{
    matrix World;
	matrix InvView;
	matrix InvViewProjection;
	matrix SpotlightView;
	matrix SpotlightViewProjection;
	matrix InvSpotlightProjection;
	float4 vSpotLightColor;
	float4 vEyePos;
	float2 ScreenDimensions;
	float SpotLightRadius; // Radius is also the far plane
	float fFarPlane;
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

	// Calculate the frustum ray using the view-space position.
    // Negating the Z component only necessary for right-handed coordinates
    float4 vFrustumRayVS = input.VSPos * (fFarPlane/-input.VSPos.z);

	// Calculate view space position from linear depth
    float4 VSPos = txGBuffer1.Sample( samLinear, ScreenCoord ).w * vFrustumRayVS;
	VSPos.w = 1.0f;

	// Un-project to world space
	float4 WSPos = mul( VSPos, InvView );
	WSPos.w = 1.0f;

	// Then project to spotlight's clip space
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
		float shadowing = txShadowmap.SampleCmp( samComp, SpotlightUV, SpotlightCSPos.w-SHADOWMAP_BIAS/SpotLightRadius );
		shadowing += txShadowmap.SampleCmp( samComp, SpotlightUV, SpotlightCSPos.w-SHADOWMAP_BIAS/SpotLightRadius, int2(-1,-1) );
		//shadowing += txShadowmap.SampleCmp( samComp, SpotlightUV, SpotlightCSPos.w-SHADOWMAP_BIAS/SpotLightRadius, int2(0,-1) );
		shadowing += txShadowmap.SampleCmp( samComp, SpotlightUV, SpotlightCSPos.w-SHADOWMAP_BIAS/SpotLightRadius, int2(1,-1) );
		//shadowing += txShadowmap.SampleCmp( samComp, SpotlightUV, SpotlightCSPos.w-SHADOWMAP_BIAS/SpotLightRadius, int2(-1,0) );
		//shadowing += txShadowmap.SampleCmp( samComp, SpotlightUV, SpotlightCSPos.w-SHADOWMAP_BIAS/SpotLightRadius, int2(1,0) );
		shadowing += txShadowmap.SampleCmp( samComp, SpotlightUV, SpotlightCSPos.w-SHADOWMAP_BIAS/SpotLightRadius, int2(-1,1) );
		//shadowing += txShadowmap.SampleCmp( samComp, SpotlightUV, SpotlightCSPos.w-SHADOWMAP_BIAS/SpotLightRadius, int2(0,1) );
		shadowing += txShadowmap.SampleCmp( samComp, SpotlightUV, SpotlightCSPos.w-SHADOWMAP_BIAS/SpotLightRadius, int2(1,1) );
		shadowing /= 5.0f;

		// Variables for lighting
		float4 Cookie = pow(txCookie.Sample( samLinear, SpotlightUV ),GAMMATOLINEAR);
		float4 Specmap = txGBuffer2.Sample( samLinear, ScreenCoord );
		float3 Normal = txGBuffer1.Sample( samLinear, ScreenCoord ).xyz;
		float3 LightPos = float3( World._41, World._42, World._43 );
		float3 LightVec = LightPos.xyz - WSPos.xyz;
		float3 ViewVec = normalize(vEyePos.xyz - WSPos.xyz);
		float DistanceAttenuation = (1.0-length(LightVec)/SpotLightRadius)/(length(LightVec)*length(LightVec)+1); // Inverse squared with a linear limiting factor

		float NdotL = saturate( dot( normalize( LightVec ), Normal ));
		float NdotH = saturate(dot(Normal, normalize(normalize( LightVec )+ViewVec)));
		float VdotH = saturate(dot(ViewVec, normalize(normalize( LightVec )+ViewVec)));
		float NdotV = saturate(dot(ViewVec, Normal));

		float3 specular = D_GGX( 1.0f-Specmap.w, NdotH ) * F_Schlick(vSpotLightColor.xyzw*Specmap.xyz, NdotV ) * Vis_Implicit();//Vis_Smith( 1.0f-Specmap.w, NdotV, NdotL );
		
		//float3 diffuse = Diffuse_Lambert((pow(txGBuffer0.Sample( samLinear, ScreenCoord ),GAMMATOLINEAR))*vSpotLightColor.xyz);
		//float3 diffuse = Diffuse_Burley( (pow(txGBuffer0.Sample( samLinear, ScreenCoord ).xyz,GAMMATOLINEAR))*vSpotLightColor.xyz, 1.0f-Specmap.w, NdotV, NdotL, VdotH );
		//float3 diffuse = Diffuse_OrenNayar( (pow(txGBuffer0.Sample( samLinear, ScreenCoord ),GAMMATOLINEAR))*vSpotLightColor.xyz, 1.0f-Specmap.w, NdotV, NdotL, VdotH );
		float3 diffuse = Diffuse_Lambert( (pow(txGBuffer0.Sample( samLinear, ScreenCoord ).xyz,GAMMATOLINEAR))*vSpotLightColor.xyz );
		
		diffuse = max(diffuse*NdotL*DistanceAttenuation,0.0f) * Cookie * shadowing;
		specular = max(specular*NdotL*DistanceAttenuation,0.0f) * Cookie * shadowing;
		
		return float4(diffuse,0.0f) + float4(specular,0.0f);
	}

	return 0.0f;
}