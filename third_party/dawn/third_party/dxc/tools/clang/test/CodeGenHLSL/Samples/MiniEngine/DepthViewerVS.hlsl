// RUN: %dxc -E main -T vs_6_0 %s | FileCheck %s

// CHECK: bufferLoad

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
// Author(s):  James Stanard
//             Alex Nankervis
//

#define USE_VERTEX_BUFFER	0

#include "ModelViewerRS.hlsli"

cbuffer VSConstants : register(b0)
{
	float4x4 modelToProjection;
};

#if USE_VERTEX_BUFFER

struct VSInput
{
	float3 position : POSITION;
	float2 texcoord0 : TEXCOORD;
	float3 normal : NORMAL;
	float3 tangent : TANGENT;
	float3 bitangent : BITANGENT;
};

#else

struct VSInput
{
	float3 position;
	float2 texcoord0;
	float3 normal;
	float3 tangent;
	float3 bitangent;
};

StructuredBuffer<VSInput> vertexArray : register(t0);

cbuffer StartVertex : register(b1)
{
	uint baseVertex;
};

#endif

struct VSOutput
{
	float4 position : SV_Position;
	float2 texcoord0 : texcoord0;
};

[RootSignature(ModelViewer_RootSig)]
#if USE_VERTEX_BUFFER
VSOutput main(VSInput vsInput)
{
#else
VSOutput main(uint vertexID : SV_VertexID)
{
	// The baseVertex argument to DrawIndexed is not automatically added to SV_VertexID...
	VSInput vsInput = vertexArray[vertexID + baseVertex];
#endif

	VSOutput vsOutput;

	vsOutput.position = mul(modelToProjection, float4(vsInput.position, 1.0));
	vsOutput.texcoord0 = vsInput.texcoord0;

	return vsOutput;
}
