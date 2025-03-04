// RUN: %dxc -E main -T vs_6_0 %s | FileCheck %s

// CHECK: error: Cannot use clipplanes attribute without specifying a 4-component SV_Position output

struct C0 {

       float4 clipPlane2;
       float4 clipPlane3;
};

struct C {
       float4 clipPlane1;
       C0 c0;
};

cbuffer ClipPlaneConstantBuffer 
{
    C c;
    
       float4 clipPlane4;
};

struct VS_INPUT
{
	float3 vPosition	: POSITION;
	float3 vNormal		: NORMAL;
 	float2 vTexcoord	: TEXCOORD0;
};

struct VS_OUTPUT
{
	float3 vNormal		: NORMAL;
	float2 vTexcoord	: TEXCOORD0;
};

[clipplanes(c.clipPlane1, c.c0.clipPlane2, clipPlane4)]
VS_OUTPUT main(VS_INPUT Input)
{

	VS_OUTPUT Output;
	
	Output.vNormal = Input.vNormal;
	Output.vTexcoord = Input.vTexcoord;
 
       return Output;
}