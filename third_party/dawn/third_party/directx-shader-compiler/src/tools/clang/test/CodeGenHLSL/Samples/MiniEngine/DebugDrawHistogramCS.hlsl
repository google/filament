// RUN: %dxc -E main -T cs_6_0 %s | FileCheck %s

// CHECK: flattenedThreadIdInGroup
// CHECK: threadId
// CHECK: bufferLoad
// CHECK: barrier
// CHECK: UMax
// CHECK: getDimensions
// CHECK: textureStore

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

#include "PostEffectsRS.hlsli"

ByteAddressBuffer Histogram : register( t0 );
StructuredBuffer<float> Exposure : register( t1 );
RWTexture2D<float3> ColorBuffer : register( u0 );

groupshared uint gs_hist[256];

[RootSignature(PostEffects_RootSig)]
[numthreads( 256, 1, 1 )]
void main( uint GI : SV_GroupIndex, uint3 DTid : SV_DispatchThreadID )
{
	uint histValue = Histogram.Load(GI * 4);

	// Compute the maximum histogram value, but don't include the black pixel
	gs_hist[GI] = GI == 0 ? 0 : histValue;
	GroupMemoryBarrierWithGroupSync();
	gs_hist[GI] = max(gs_hist[GI], gs_hist[(GI + 128) % 256]);
	GroupMemoryBarrierWithGroupSync();
	gs_hist[GI] = max(gs_hist[GI], gs_hist[(GI + 64) % 256]);
	GroupMemoryBarrierWithGroupSync();
	gs_hist[GI] = max(gs_hist[GI], gs_hist[(GI + 32) % 256]);
	GroupMemoryBarrierWithGroupSync();
	gs_hist[GI] = max(gs_hist[GI], gs_hist[(GI + 16) % 256]);
	GroupMemoryBarrierWithGroupSync();
	gs_hist[GI] = max(gs_hist[GI], gs_hist[(GI + 8) % 256]);
	GroupMemoryBarrierWithGroupSync();
	gs_hist[GI] = max(gs_hist[GI], gs_hist[(GI + 4) % 256]);
	GroupMemoryBarrierWithGroupSync();
	gs_hist[GI] = max(gs_hist[GI], gs_hist[(GI + 2) % 256]);
	GroupMemoryBarrierWithGroupSync();
	gs_hist[GI] = max(gs_hist[GI], gs_hist[(GI + 1) % 256]);
	GroupMemoryBarrierWithGroupSync();

	uint maxHistValue = gs_hist[GI];

	uint2 BufferDim;
	ColorBuffer.GetDimensions(BufferDim.x, BufferDim.y);

	const uint2 RectCorner = uint2(BufferDim.x / 2 - 512, BufferDim.y - 256);
	const uint2 GroupCorner = RectCorner + DTid.xy * 4;

	uint height = 127 - DTid.y * 4;
	uint threshold = histValue * 128 / max(1, maxHistValue);

	float3 OutColor = (GI == (uint)Exposure[3]) ? float3(1.0, 1.0, 0.0) : float3(0.5, 0.5, 0.5);

	for (uint i = 0; i < 4; ++i)
	{
		float3 MaskedColor = (height - i) < threshold ? OutColor : float3(0, 0, 0);

		// 4-wide column with 2 pixels for the histogram bar and 2 for black spacing
		ColorBuffer[GroupCorner + uint2(0, i)] = MaskedColor;
		ColorBuffer[GroupCorner + uint2(1, i)] = MaskedColor;
		ColorBuffer[GroupCorner + uint2(2, i)] = float3(0, 0, 0);
		ColorBuffer[GroupCorner + uint2(3, i)] = float3(0, 0, 0);
	}
}
