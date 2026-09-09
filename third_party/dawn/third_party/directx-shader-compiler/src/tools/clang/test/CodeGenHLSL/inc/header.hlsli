
//--------------------------------------------------------------------------------------
// Globals
//--------------------------------------------------------------------------------------
cbuffer cbPerObject : register( b0 )
{
	float4	g_vObjectColor	: packoffset( c0 );
};

cbuffer cbPerFrame : register( b1 )
{
	float3	g_vLightDir	: packoffset( c0 );
	float	g_fAmbient	: packoffset( c0.w );
};

//--------------------------------------------------------------------------------------
// Textures and Samplers
//--------------------------------------------------------------------------------------
Texture2D	g_txDiffuse : register( t0 );
SamplerState	g_samLinear : register( s0 );

//--------------------------------------------------------------------------------------
// Input / Output structures
//--------------------------------------------------------------------------------------
struct PS_INPUT
{
	float3 vNormal		: NORMAL;
        float2 vTexcoord	: TEXCOORD0;	
};

