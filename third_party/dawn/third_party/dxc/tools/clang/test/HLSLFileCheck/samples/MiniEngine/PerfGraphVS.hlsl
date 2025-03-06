// RUN: %dxc -E main -T vs_6_0 %s | FileCheck %s

// CHECK: Saturate

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

cbuffer CBGraphColor : register(b0)
{
	float3 Color;
	float RcpXScale;
	uint NodeCount;
	uint FrameID;
};

cbuffer constants : register(b1)
{
	uint Instance;
	float RcpYScale;
}

struct VSOutput
{
	float4 pos : SV_POSITION;
	float3 col : COLOR;
};

StructuredBuffer<float> PerfTimes : register(t0);

[RootSignature(PerfGraph_RootSig)]
VSOutput main( uint VertexID : SV_VertexID )
{
	// Assume NodeCount is a power of 2
	uint offset = (FrameID + VertexID) & (NodeCount - 1);
		
	// TODO:  Stop interleaving data
	float perfTime = saturate(PerfTimes[offset] * RcpYScale) * 2.0 - 1.0;
	float frame = VertexID * RcpXScale - 1.0; 

	VSOutput output;
	output.pos = float4(frame, perfTime, 1, 1);
	output.col = Color;
	return output;

}