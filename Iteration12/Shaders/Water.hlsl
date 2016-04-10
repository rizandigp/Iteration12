//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------
#include "EngineGlobal.hlsl"

Texture2D txDiffuse;
Texture2D txNormal;
Texture2D txSpecular;
Texture2D txFoam;
TextureCube txCubemap;

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
	float time;
	float fFarPlane;
};

// MRT output
struct PSOut
{
	float4  frontbuffer : SV_TARGET0;
	float4	gbuffer0 : SV_TARGET0;	// Albedo.xyz	| Metallic
	float4	gbuffer1 : SV_TARGET1;	// Normal.xyz	| (non-)Linear Depth
	float4	gbuffer2 : SV_TARGET2;	// Specular.xyz	| Glossiness
};


//--------------------------------------------------------------------------------------
struct VS_INPUT
{
    float4 Pos : POSITION;
	float4 Normal : NORMAL;
	float2 Tex : TEXCOORD;
};

struct PS_INPUT
{
    float4 Pos : SV_POSITION;
	float4 WSPos : POSITION1;
	float4 VSPos : POSITION2;
    float4 Normal : NORMAL;
	float2 Tex : TEXCOORD;
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
    output.Normal.xyz = normalize(mul( input.Normal, (float3x3)NormalMatrix ));
    output.Tex = input.Tex;
    
    return output;
}


//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
static const int NUM_STEPS = 8;
//static const float PI	 	= 3.1415;
static const float EPSILON	= 1e-3;
//static float EPSILON_NRM	= 0.1 / iResolution.x;

// sea
static const int ITER_GEOMETRY = 3;
static const int ITER_FRAGMENT = 4;
static const float SEA_HEIGHT = 0.6;
static const float SEA_CHOPPY = 4.0;
static const float SEA_SPEED = 0.4;
static const float SEA_FREQ = 0.16;
static const float3 SEA_BASE = pow(float3(0.017,0.114,0.127),GAMMATOLINEAR);//pow(float3(0.017,0.114,0.217),DEGAMMA);
static const float3 SEA_WATER_COLOR = pow(float3(0.2,0.9,0.8),DEGAMMA);
static float2x2 octave_m = float2x2(1.6,1.2,-1.2,1.6);


float hash( float2 p ) 
{
	float h = dot(p,float2(127.1,311.7));	
    return frac(sin(h)*43758.5453123);
}
float noise( in float2 p ) 
{
    float2 i = floor( p );
    float2 f = frac( p );	
	float2 u = f*f*(3.0-2.0*f);
    return -1.0+2.0*lerp( lerp( hash( i + float2(0.0,0.0) ), 
                     hash( i + float2(1.0,0.0) ), u.x),
                lerp( hash( i + float2(0.0,1.0) ), 
                     hash( i + float2(1.0,1.0) ), u.x), u.y);
}

// lighting
float diffuse(float3 n,float3 l,float p) 
{
    return pow(dot(n,l) * 0.4 + 0.6,p);
}
float specular(float3 n,float3 l,float3 e,float s) 
{    
    float nrm = (s + 8.0) / (3.1415 * 8.0);
    return pow(max(dot(reflect(e,n),l),0.0),s) * nrm;
}

// sky
float3 getSkyColor(float3 e) 
{
    e.z = max(e.z,0.0);
    float3 ret;
    ret.r = pow(1.0-e.z,2.0);
    ret.g = 1.0-e.z;
    ret.b = 0.6+(1.0-e.z)*0.4;
    return pow(ret,GAMMA);
}

// sea
float sea_octave(float2 uv, float choppy) 
{
    uv += noise(uv);        
    float2 wv = 1.0-abs(sin(uv));
    float2 swv = abs(cos(uv));    
    wv = lerp(wv,swv,wv);
    return pow(1.0-pow(wv.x * wv.y,0.65),choppy);
}

float map(float3 p) 
{
    float freq = SEA_FREQ;
    float amp = SEA_HEIGHT;
    float choppy = SEA_CHOPPY;
    float2 uv = p.xy; 
	uv.x *= 0.75;
    
    float d, h = 0.0;    
    for(int i = 0; i < ITER_GEOMETRY; i++) 
	{        
    	d = sea_octave((uv+time*SEA_SPEED)*freq,choppy);
    	d += sea_octave((uv-time*SEA_SPEED)*freq,choppy);
        h += d * amp;        
    	uv = mul(uv,octave_m); 
		freq *= 1.9; 
		amp *= 0.22;
        choppy = lerp(choppy,1.0,0.2);
    }
    return p.z - h;
}

float map_detailed(float3 p) 
{
    float freq = SEA_FREQ*1.9*1.9*1.9*1.9*1.9;
    float amp = SEA_HEIGHT*0.22*0.22;
    float choppy = lerp(SEA_CHOPPY,1.0,0.2);
    float2 uv = p.xy; 
	uv.x *= 0.75;
    
    float d, h = 0.0;    
    [unroll] for(int i = 0; i < ITER_FRAGMENT; i++) 
	{        
    	d = sea_octave((uv+time*SEA_SPEED)*freq,choppy);
    	d += sea_octave((uv-time*SEA_SPEED)*freq,choppy);
        h += d * amp;        
    	uv = mul(uv,octave_m); 
		freq *= 1.9; 
		amp *= 0.22;
        choppy = lerp(choppy,1.0,0.2);
    }
    return 1.0-h;
}

float3 getSeaColor(float3 p, float3 n, float3 l, float3 eye, float3 dist) 
{  
    float fresnel = 1.0 - max(dot(n,-eye),0.0);
    fresnel = pow(fresnel,3.0) * 0.65;
        
    float3 reflected = getSkyColor(reflect(eye,n));    
    float3 refracted = SEA_BASE + diffuse(n,l,80.0) * SEA_WATER_COLOR * 0.12; 
    
    float3 color = lerp(refracted,reflected,fresnel);
    
    float atten = max(1.0 - dot(dist,dist) * 0.001, 0.0);
    color += SEA_WATER_COLOR * (p.y - SEA_HEIGHT) * 0.18 * atten;
    
    //color += float3(specular(n,l,eye,60.0));
    
    return color;
}

// tracing
float3 getNormal(float3 p, float eps) 
{
    float3 n;
    n.z = map_detailed(p);    
    n.x = map_detailed(float3(p.x+eps,p.y,p.z)) - n.z;
    n.y = map_detailed(float3(p.x,p.y+eps,p.z)) - n.z;
    n.z = eps;
    return normalize(n);
}

PSOut PS( PS_INPUT input) : SV_Target
{

	//float3 normals = combineNormals( normalize(txNormal.Sample( samLinear, input.Tex.xy - time/4.0f ).xyz*2.0f-1.0f), normalize(txNormal.Sample( samLinear, input.Tex.xy + float2(time/2.0f,time/3.0f) + float2(0.5f,0.5f) ).xyz*2.0f-1.0f) );
	float3 normals = combineNormals( (txNormal.Sample( samLinear, input.Tex.xy/24.0f + time/24.0f ).xyz*2.0f-1.0f), (txNormal.Sample( samLinear, input.Tex.xy/24.0f - time/24.0f ).xyz*2.0f-1.0f) );
	//float3 normals2 = combineNormals( (txNormal.Sample( samLinear, input.Tex.xy/2.0f*1.9f + float2(time/12.0f,-time/12.0f) ).xyz*2.0f-1.0f), (txNormal.Sample( samLinear, input.Tex.xy/2.0f*1.9 - float2(time/12.0f,-time/12.0f) ).xyz*2.0f-1.0f) );
	//normals = combineNormals( normals, normals2 );
	//normals2 = combineNormals( (txNormal.Sample( samLinear, input.Tex.xy/32.0f + float2(time/24.0f,-time/24.0f) ).xyz*2.0f-1.0f), (txNormal.Sample( samLinear, input.Tex.xy/32.0f - float2(time/24.0f,-time/24.0f) ).xyz*2.0f-1.0f) );
	//normals = combineNormals( normals, normals2 );
	//float3 normals = 0.5f*((txNormal.Sample( samLinear, input.Tex.xy/8.0f + time/24.0f ).xyz*2.0f-1.0f) + (txNormal.Sample( samLinear, input.Tex.xy/8.0f - time/24.0f ).xyz*2.0f-1.0f) );
	normals.z *= 4.0f;
	normals = normalize(normals);
	//float dist = distance(input.WSPos.xyz,vEyePos.xyz);
	//float3 normals = getNormal(float3(input.Tex.xy,input.WSPos.z),dist*dist*0.0001);
	
	normals = normalize(combineNormals(normalize(input.Normal.xyz),normals));
	//float3 normals = float3(0.0,0.0,1.0);

	float3 viewvector = normalize(vEyePos.xyz-input.WSPos.xyz);
	float3 halfvector = normalize(normals+viewvector);
	float fresnel = pow( 1.0f - saturate( dot( viewvector, normals ) ), 5.0f )/PI + 0.0002;

	float3 reflected = 0.65f*txCubemap.SampleLevel( samLinear, reflect( -viewvector, normals ).xzy, 0.0f ); 
    float3 refracted = 0.1f*SEA_BASE*txCubemap.SampleLevel( samLinear, normals.xzy, 4.5f );
    float3 color = lerp(refracted,reflected,fresnel);

    //float atten = max(1.0 - dot(dist,dist) * 0.001, 0.0);
    //color += SEA_WATER_COLOR * max(input.WSPos.z,0.0) * 0.009 * (1.0f-fresnel);
	color += SEA_WATER_COLOR * 0.03 * (1.0f-dot(normals,float3(0.0f,0.0f,1.0f))) * (1.0f-fresnel);
	//float foam = txFoam.Sample( samLinear, input.Tex.xy ).x * max(input.WSPos.z,0.0);

	PSOut output;
	output.frontbuffer = color.xyzz;// + txCubemap.Sample( samLinear, reflect(viewdir,normals) );//fresnel*float4(0.8f,0.8f,1.0f,1.0f);//+max((input.WSPos.z+1.0f)/4.0f,0.0f)*float4(0.0f,0.1f,0.2f,1.0f)*0.01f;//pow(saturate(dot( viewdir, normals )), 3)*input.WSPos.z/50.0f*pow( txDiffuse.Sample( samLinear, input.Tex ), DEGAMMA );//float4(0.0f,0.4f,0.4f,1.0f);
	output.gbuffer0.xyz = SEA_WATER_COLOR * max(input.WSPos.z,0.0) * 0.07f;//max((input.WSPos.z+2.0f)/4.0f,0.1f)*float3(0.0f,0.07f,0.15f);//txDiffuse.Sample( samLinear, input.Tex ).xyz;
	output.gbuffer1.xyz = normals;
	output.gbuffer1.w = -input.VSPos.z/fFarPlane;
	output.gbuffer2.xyz = 0.04f;//txSpecular.Sample( samLinear, input.Tex ).g*SpecularParams.x;
	output.gbuffer2.w = 0.75f;//txSpecular.Sample( samLinear, input.Tex ).r;

	return output;
}
