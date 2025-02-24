// RUN: %dxc -E main -T vs_6_0 %s | FileCheck %s

// CHECK: @main
cbuffer cbPerObject : register( b0 )
{
  float4x4		g_mWorldViewProjection;
  float4x4		g_mWorld;
};

//--------------------------------------------------------------------------------------
// Input / Output structures
//--------------------------------------------------------------------------------------
struct VS_INPUT
{
	float3 vPosition	: POSITION;
	float3 vNormal		: NORMAL;
 	float2 vTexcoord	: TEXCOORD0;
};

struct VS_OUTPUT
{
	float4 vPosition	: SV_POSITION;
};




//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
VS_OUTPUT main( VS_INPUT Input )
{
	VS_OUTPUT Output;
	float4x4 tmp = mul(g_mWorldViewProjection, g_mWorld);
	Output.vPosition = mul( float4( Input.vPosition, 1.0 ), tmp );
	
	return Output;
}

