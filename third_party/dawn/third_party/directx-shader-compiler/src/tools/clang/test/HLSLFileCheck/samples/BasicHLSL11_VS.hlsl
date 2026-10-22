// RUN: %dxc -E main -T vs_6_0 %s | FileCheck %s

// Skip input
// CHECK: POSITION
// CHECK: xyz
// CHECK: NORMAL
// CHECK: xyz
// CHECK: TEXCOORD
// CHECK: xy

// Make sure used match output mask.
// CHECK: NORMAL
// CHECK: xyz
// CHECK: xyz
// CHECK: TEXCOORD
// CHECK: xy
// CHECK: xy
// CHECK: SV_Position
// CHECK: xyzw
// CHECK: xyzw

// CHECK: OutputPositionPresent=1
// CHECK: dx.op.createHandle(i32 57, i8 2, i32 0, i32 5, i1 false)

//--------------------------------------------------------------------------------------
// File: BasicHLSL11_VS.hlsl
//
// The vertex shader file for the BasicHLSL11 sample.  
// 
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
// Globals
//--------------------------------------------------------------------------------------
cbuffer cbPerObject : register( b5 )
{
	matrix		g_mWorldViewProjection	: packoffset( c0 );
	column_major  matrix		g_mWorld		: packoffset( c4 );
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
	float3 vNormal		: NORMAL;
	float2 vTexcoord	: TEXCOORD0;
	float4 vPosition	: SV_POSITION;
};




//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
VS_OUTPUT main( VS_INPUT Input )
{
	VS_OUTPUT Output;
	
	Output.vPosition = mul( float4( Input.vPosition, 1.0 ), g_mWorldViewProjection );
	Output.vNormal = mul( Input.vNormal, (float3x3)g_mWorld );
	Output.vTexcoord = Input.vTexcoord;
	
	return Output;
}

