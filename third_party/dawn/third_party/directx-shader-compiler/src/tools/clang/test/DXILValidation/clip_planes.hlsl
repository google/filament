// RUN: %dxc -E main -T vs_6_0 %s | FileCheck %s

// TODO: validate the debug info for clip plane.

// CHECK: SV_ClipDistance
// CHECK: SV_ClipDistance
// CHECK: SV_ClipDistance
// CHECK: dx.op.dot4.f32
// CHECK: dx.op.dot4.f32
// CHECK: dx.op.dot4.f32

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
	float4 vPosition	: SV_POSITION;
};

[clipplanes(c.clipPlane1, c.c0.clipPlane2, clipPlane4)]
VS_OUTPUT main(VS_INPUT Input)
{

	VS_OUTPUT Output;
	
	Output.vPosition = float4( Input.vPosition, 1.0 );
	Output.vNormal = Input.vNormal;
	Output.vTexcoord = Input.vTexcoord;
 
       return Output;
}