//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------
#include "EngineGlobal.hlsl"

#define SPECULAR_POWER_SCALE 2048.0f

Texture2D txGBuffer0;
Texture2D txGBuffer1;
Texture2D txGBuffer2;
Texture2D txShadowmap;
Texture2D txRSMNormal;
Texture2D txRSMColor;

cbuffer cbPerFrame
{
    matrix ViewProjection;
};

cbuffer cbPerObject
{
	matrix InvViewProjection;
	matrix SpotlightViewProjection;
	matrix InvSpotlightViewProjection;
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
/*
static const float2 poissonDisk[64];
poissonDisk[0] = float2(-0.613392, 0.617481);
poissonDisk[1] = float2(0.170019, -0.040254);
poissonDisk[2] = float2(-0.299417, 0.791925);
poissonDisk[3] = float2(0.645680, 0.493210);
poissonDisk[4] = float2(-0.651784, 0.717887);
poissonDisk[5] = float2(0.421003, 0.027070);
poissonDisk[6] = float2(-0.817194, -0.271096);
poissonDisk[7] = float2(-0.705374, -0.668203);
poissonDisk[8] = float2(0.977050, -0.108615);
poissonDisk[9] = float2(0.063326, 0.142369);
poissonDisk[10] = float2(0.203528, 0.214331);
poissonDisk[11] = float2(-0.667531, 0.326090);
poissonDisk[12] = float2(-0.098422, -0.295755);
poissonDisk[13] = float2(-0.885922, 0.215369);
poissonDisk[14] = float2(0.566637, 0.605213);
poissonDisk[15] = float2(0.039766, -0.396100);
poissonDisk[16] = float2(0.751946, 0.453352);
poissonDisk[17] = float2(0.078707, -0.715323);
poissonDisk[18] = float2(-0.075838, -0.529344);
poissonDisk[19] = float2(0.724479, -0.580798);
poissonDisk[20] = float2(0.222999, -0.215125);
poissonDisk[21] = float2(-0.467574, -0.405438);
poissonDisk[22] = float2(-0.248268, -0.814753);
poissonDisk[23] = float2(0.354411, -0.887570);
poissonDisk[24] = float2(0.175817, 0.382366);
poissonDisk[25] = float2(0.487472, -0.063082);
poissonDisk[26] = float2(-0.084078, 0.898312);
poissonDisk[27] = float2(0.488876, -0.783441);
poissonDisk[28] = float2(0.470016, 0.217933);
poissonDisk[29] = float2(-0.696890, -0.549791);
poissonDisk[30] = float2(-0.149693, 0.605762);
poissonDisk[31] = float2(0.034211, 0.979980);
poissonDisk[32] = float2(0.503098, -0.308878);
poissonDisk[33] = float2(-0.016205, -0.872921);
poissonDisk[34] = float2(0.385784, -0.393902);
poissonDisk[35] = float2(-0.146886, -0.859249);
poissonDisk[36] = float2(0.643361, 0.164098);
poissonDisk[37] = float2(0.634388, -0.049471);
poissonDisk[38] = float2(-0.688894, 0.007843);
poissonDisk[39] = float2(0.464034, -0.188818);
poissonDisk[40] = float2(-0.440840, 0.137486);
poissonDisk[41] = float2(0.364483, 0.511704);
poissonDisk[42] = float2(0.034028, 0.325968);
poissonDisk[43] = float2(0.099094, -0.308023);
poissonDisk[44] = float2(0.693960, -0.366253);
poissonDisk[45] = float2(0.678884, -0.204688);
poissonDisk[46] = float2(0.001801, 0.780328);
poissonDisk[47] = float2(0.145177, -0.898984);
poissonDisk[48] = float2(0.062655, -0.611866);
poissonDisk[49] = float2(0.315226, -0.604297);
poissonDisk[50] = float2(-0.780145, 0.486251);
poissonDisk[51] = float2(-0.371868, 0.882138);
poissonDisk[52] = float2(0.200476, 0.494430);
poissonDisk[53] = float2(-0.494552, -0.711051);
poissonDisk[54] = float2(0.612476, 0.705252);
poissonDisk[55] = float2(-0.578845, -0.768792);
poissonDisk[56] = float2(-0.772454, -0.090976);
poissonDisk[57] = float2(0.504440, 0.372295);
poissonDisk[58] = float2(0.155736, 0.065157);
poissonDisk[59] = float2(0.391522, 0.849605);
poissonDisk[60] = float2(-0.620106, -0.328104);
poissonDisk[61] = float2(0.789239, -0.419965);
poissonDisk[62] = float2(-0.545396, 0.538133);
poissonDisk[63] = float2(-0.178564, -0.596057);
*/
//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS( PS_INPUT input) : SV_Target
{
	float2 poissonDisk[16];
	poissonDisk[0] = float2(-0.613392, 0.617481);
	poissonDisk[1] = float2(0.170019, -0.040254);
	poissonDisk[2] = float2(-0.299417, 0.791925);
	poissonDisk[3] = float2(0.645680, 0.493210);
	poissonDisk[4] = float2(-0.651784, 0.717887);
	poissonDisk[5] = float2(0.421003, 0.027070);
	poissonDisk[6] = float2(-0.817194, -0.271096);
	poissonDisk[7] = float2(-0.705374, -0.668203);
	poissonDisk[8] = float2(0.977050, -0.108615);
	poissonDisk[9] = float2(0.063326, 0.142369);
	poissonDisk[10] = float2(0.203528, 0.214331);
	poissonDisk[11] = float2(-0.667531, 0.326090);
	poissonDisk[12] = float2(-0.098422, -0.295755);
	poissonDisk[13] = float2(-0.885922, 0.215369);
	poissonDisk[14] = float2(0.566637, 0.605213);
	poissonDisk[15] = float2(0.039766, -0.396100);

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

	float GIRadius = 1.0f;
	float4 GIdiffuse = 0.0f;

	for (int i=0; i<16; i++)
	{
		// VPL coordinate
		float2 GISampleUV = SpotlightUV;// + poissonDisk[i]/2.0f;

		// Get clip space position
		float _z = txShadowmap.Sample( samLinear, GISampleUV ).r;
		float _x = GISampleUV.x * 2 - 1;
		float _y = (1 - GISampleUV.y) * 2 - 1;

		// Un-project from clip space back into world space
		float4 _ProjectedPos = float4(x, y, z, 1.0f );
		float4 _WSPos = mul(ProjectedPos, InvSpotlightViewProjection );
		_WSPos.xyz = _WSPos.xyz/_WSPos.w;
		_WSPos.w = 1.0f;

		// Variables for lighting
		float3 normal = txGBuffer1.Sample( samLinear, ScreenCoord ).xyz;
		float4 rsmcolor = txRSMColor.Sample( samLinearClamp, GISampleUV );
		float3 lightVec = _WSPos.xyz - WSPos.xyz;
		float dist_attenuation = (1-length(lightVec)/GIRadius)/(length(lightVec)*length(lightVec)+1); // Inverse squared with a linear limiting factor

		// Calculate diffuse lighting
		float4 diffuse = saturate( dot( normalize( lightVec ), normal )); // Lambert
		diffuse *= dist_attenuation*rsmcolor;
		GIdiffuse += z;//max(diffuse,0.0f);
	}
	// Multiply with albedo and return
	return (pow(txGBuffer0.Sample( samLinear, ScreenCoord ),GAMMATOLINEAR))*GIdiffuse+0.1f;
}