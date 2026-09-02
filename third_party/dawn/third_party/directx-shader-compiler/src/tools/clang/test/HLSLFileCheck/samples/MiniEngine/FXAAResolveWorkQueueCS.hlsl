// RUN: %dxc -E main -T cs_6_0 %s | FileCheck %s

// CHECK: flattenedThreadIdInGroup
// CHECK: bufferLoad
// CHECK: br i1
// CHECK: bufferStore
// CHECK: br label

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
// Used with FXAA to resolve the lengths of the two work queues and to generate DispatchIndirect parameters.
// The work queues are also padded out to a multiple of 64 with dummy work items.
//

#include "FXAARootSignature.hlsli"

ByteAddressBuffer WorkCounterH : register(t0);
ByteAddressBuffer WorkCounterV : register(t1);
RWByteAddressBuffer IndirectParams : register(u0);
RWStructuredBuffer<uint> WorkQueueH : register(u1);
RWStructuredBuffer<uint> WorkQueueV : register(u2);

[RootSignature(FXAA_RootSig)]
[numthreads( 64, 1, 1 )]
void main( uint3 Gid : SV_GroupID, uint GI : SV_GroupIndex, uint3 GTid : SV_GroupThreadID, uint3 DTid : SV_DispatchThreadID )
{
	uint PixelCountH = WorkCounterH.Load(0);
	uint PixelCountV = WorkCounterV.Load(0);

	uint PaddedCountH = (PixelCountH + 63) & ~63;
	uint PaddedCountV = (PixelCountV + 63) & ~63;

	// Write out padding to the buffer
	if (GI + PixelCountH < PaddedCountH)
		WorkQueueH[PixelCountH + GI] = 0xffffffff;

	// Write out padding to the buffer
	if (GI + PixelCountV < PaddedCountV)
		WorkQueueV[PixelCountV + GI] = 0xffffffff;

	if (GI == 0)
	{
		IndirectParams.Store(0 , PaddedCountH >> 6);
		IndirectParams.Store(12, PaddedCountV >> 6);
	}
}