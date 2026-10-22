// RUN: %dxc -E main -T vs_6_0 %s | FileCheck %s

// CHECK: lshr
// CHECK: and
// CHECK: and

//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Developed by Minigraph
//
// Author:  Julia Careaga
//

#include "PerfGraphRS.hlsli"

struct VSOutput
{
	float4 pos : SV_POSITION;
	float3 col : COLOR;
};

cbuffer CB : register(b1)
{
	float RecSize;
}

[RootSignature(PerfGraph_RootSig)]
VSOutput main( uint vertexID : SV_VertexID, uint instanceID : SV_InstanceID )
{
	//VSOutput Output;
	//float2 uv = float2( (vertexID >> 1) & 1, vertexID & 1 );
	//float2 Corner = lerp( float2(-1.0f, 1.0f), float2(1.0f, RecSize), uv );
	//Corner.y -= 0.45f * instanceID;
	//Output.pos = float4(Corner.xy, 1.0,1);
	//Output.col = float3(0.0, 0.0, 0.0);
	//return Output;

	VSOutput Output;
	float2 uv = float2( (vertexID >> 1) & 1, vertexID & 1 );
	float2 Corner = lerp( float2(-1.0f, 1.0f), float2(1.0f, -1), uv );
	Output.pos = float4(Corner.xy, 1.0,1);
	Output.col = float3(0.0, 0.0, 0.0);
	return Output;

}

