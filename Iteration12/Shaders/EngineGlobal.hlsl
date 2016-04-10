//
//	Global definitions, settings & functions
//
#pragma once

SamplerState samPoint : register( s0 );
SamplerState samLinear : register( s1 );
SamplerComparisonState samComp : register( s2 );
SamplerState samLinearClamp : register( s3 );

#define PI 3.1415926
#define ONEDIVPI 1.0/3.1415926
#define SHADOWMAP_BIAS 0.05f // In world units
#define GAMMATOLINEAR 2.2f
#define LINEARTOGAMMA 1/2.2f
#define DEGAMMA GAMMATOLINEAR
#define GAMMA LINEARTOGAMMA

float phong_specular( float3 lightvector, float3 normal, float3 viewvector, float power )
{
	return pow(saturate( dot( normalize( reflect( normalize( -lightvector ), normal ) ), viewvector )), power);
}

float blinnphong_specular( float3 lightvector, float3 normal, float3 viewvector, float power )
{
	float3 halfvector = normalize(lightvector+viewvector);
	return pow(saturate(dot(normal,halfvector)),power);
}

float cooktorrance_specular( float3 lightvector, float3 normal, float3 viewvector, float power, float f0 )
{
	float fresnel = pow( 1.0f - saturate( dot(viewvector, normal ) ), 5.0f );
    fresnel *= ( 1.0f - f0 );
    fresnel += f0;
	float3 halfvector = normalize(lightvector+viewvector);
	return fresnel*pow(saturate(dot(normal,halfvector)),power);
}

float ggx_specular( float3 lightvector, float3 normal, float3 viewvector, float alpha, float f0 )
{
	float fresnel = pow( 1.0f - saturate( dot(viewvector, normal ) ), 5.0f );
    fresnel *= ( 1.0f - f0 );
    fresnel += f0;
	float3 halfvector = normalize(lightvector+viewvector);
	float NdotH = saturate(dot(normal,halfvector));
	float den = ((NdotH*NdotH)*(alpha*alpha*alpha*alpha-1.0f)+1.0f);
	den *= den*PI;
	return fresnel*(alpha*alpha*alpha*alpha)/den;
}

//
// Tonemapping operators
//

// All this does is scale linearly then apply the gamma curve
float3 LinearTonemap( float3 color )
{
	//color *= 16;  // Hardcoded Exposure Adjustment
	float3 retColor = pow(color,LINEARTOGAMMA);	// Gamma
	return float4(retColor,1);
}

// Simple Reinhard curve
float3 ReinhardTonemap( float3 color )
{
	//color *= 16;  // Hardcoded Exposure Adjustment
	float3 retColor = color/(color+1);  // Reinhard
	retColor = pow(retColor,LINEARTOGAMMA); // Gamma
	return retColor;
}

// Filmic curve. Optimized formula by Jim Hejl and Richard Burgess-Dawson.
// Looks really good, with nice contrast and crisp colors
float3 FilmicTonemap( float3 color )
{
	//color *= 16.0f;	// Hardcoded Exposure Adjustment
	float3 x = max(0,color-0.004);
	float3 retColor = (x*(6.2*x+.5))/(x*(6.2*x+1.7)+0.06); // Filmic Curve
	return retColor;
}
/*
// Haarm-Peter Duiker’s curve. 
// This version is very similar to the Cineon node in Digital Fusion
float3 DuikerTonemap( float3 color )
{
   float3 ld = 0.002;
   float linReference = 0.18;
   float logReference = 444;
   float logGamma = 0.45;

   float3 LogColor;
   LogColor.rgb = (log10(0.4*color.rgb/linReference)/ld*logGamma + logReference)/1023.f;
   LogColor.rgb = saturate(LogColor.rgb);
   
   float FilmLutWidth = 256;
   float Padding = .5/FilmLutWidth;

   //  apply response lookup and color grading for target display
   float3 retColor;
   retColor.r = tex2D(FilmLut, float2( lerp(Padding,1-Padding,LogColor.r), .5)).r;
   retColor.g = tex2D(FilmLut, float2( lerp(Padding,1-Padding,LogColor.g), .5)).r;
   retColor.b = tex2D(FilmLut, float2( lerp(Padding,1-Padding,LogColor.b), .5)).r;

   return retColor;
   return 0;
}*/


// Uncharted 2's formula by John Hable
static float A = 0.15;
static float B = 0.50;
static float C = 0.10;
static float D = 0.20;
static float E = 0.02;
static float F = 0.30;
static float W = 11.2;

float3 Uncharted2Curve(float3 x)
{
	return ((x*(A*x+C*B)+D*E)/(x*(A*x+B)+D*F))-E/F;
}

float3 Uncharted2Tonemap( float3 x )
{
	//x *= 16;  // Hardcoded Exposure Adjustment

	float ExposureBias = 2.0f;
	float3 curr = Uncharted2Curve(ExposureBias*x);

	float3 whiteScale = 1.0f/Uncharted2Curve(W);
	float3 color = curr*whiteScale;

	float3 retColor = pow(color,1/2.2);
	return float4(retColor,1);
}


// Combine two normal maps
// Based on http://blog.selfshadow.com/publications/blending-in-detail/
float3 combineNormals(float3 base, float3 detail)
{
	base.z += 1.0f; 
	detail.xy = -detail.xy;
	return normalize(base*dot(base, detail)/base.z - detail);
}

// Convert 3D vector in world space to uv coordinate in a latitude-longitude environment map
float2 latlong(float3 v) 
{
	//v = normalize(v);
	float theta = acos(v.z); // +z is up
	float phi = atan2(v.y, v.x) + PI;
	return float2(phi, theta) * float2(.1591549, .6366198/2.0);
}