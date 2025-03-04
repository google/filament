// RUN: %dxc -E main -T cs_6_0 %s | FileCheck %s

// CHECK: groupId
// CHECK: flattenedThreadIdInGroup
// CHECK: threadIdInGroup
// CHECK: threadId
// CHECK: barrier

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
// Author:  James Stanard 
//

#include "SSAORS.hlsli"

RWTexture2D<float> LinearZ : register(u0);
RWTexture2D<float2> DS2x : register(u1);
RWTexture2DArray<float> DS2xAtlas : register(u2);
RWTexture2D<float2> DS4x : register(u3);
RWTexture2DArray<float> DS4xAtlas : register(u4);
cbuffer ConstantBuffer_x : register(b0)
{
	float ZMagic;
}

Texture2D<float> Depth : register(t0);

float Linearize( uint2 st )
{
	float depth = Depth[st];
	float dist = 1.0 / (ZMagic * depth + 1.0);
	LinearZ[st] = dist;
	return dist;
}

groupshared float g_CacheW[256];

[RootSignature(SSAO_RootSig)]
[numthreads( 8, 8, 1 )]
void main( uint3 Gid : SV_GroupID, uint GI : SV_GroupIndex, uint3 GTid : SV_GroupThreadID, uint3 DTid : SV_DispatchThreadID )
{
	uint2 startST = Gid.xy << 4 | GTid.xy;
	uint destIdx = GTid.y << 4 | GTid.x;
	g_CacheW[ destIdx +  0  ] = Linearize(startST | uint2(0, 0));
	g_CacheW[ destIdx +  8  ] = Linearize(startST | uint2(8, 0));
	g_CacheW[ destIdx + 128 ] = Linearize(startST | uint2(0, 8));
	g_CacheW[ destIdx + 136 ] = Linearize(startST | uint2(8, 8));

	GroupMemoryBarrierWithGroupSync();

	uint ldsIndex = (GTid.x << 1) | (GTid.y << 5);

	float w1 = g_CacheW[ldsIndex];

	uint2 st = DTid.xy;
	uint slice = (st.x & 3) | ((st.y & 3) << 2);
	DS2x[st] = w1;
	DS2xAtlas[uint3(st >> 2, slice)] = w1;

	if ((GI & 011) == 0)
	{
		st = DTid.xy >> 1;
		slice = (st.x & 3) | ((st.y & 3) << 2);
		DS4x[st] = w1;
		DS4xAtlas[uint3(st >> 2, slice)] = w1;
	}

}
