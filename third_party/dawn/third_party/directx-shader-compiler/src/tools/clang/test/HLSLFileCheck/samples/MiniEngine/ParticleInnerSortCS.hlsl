// RUN: %dxc -E main -T cs_6_0 %s | FileCheck %s

// CHECK: groupId
// CHECK: flattenedThreadIdInGroup
// CHECK: bufferLoad
// CHECK: addrspace(3)
// CHECK: barrier
// CHECK: addrspace(3)
// CHECK: barrier
// CHECK: bufferStore

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

#include "ParticleUtility.hlsli"

RWStructuredBuffer<uint> g_SortBuffer : register(u0);

cbuffer CB : register(b0)
{
	uint k; // k >= 4096
};

groupshared uint gs_SortKeys[2048];

[RootSignature(Particle_RootSig)]
[numthreads(1024, 1, 1)]
void main( uint3 Gid : SV_GroupID, uint GI : SV_GroupIndex )
{
	const uint RangeStart = Gid.x * 2048;
	gs_SortKeys[GI] = g_SortBuffer[RangeStart + GI];
	gs_SortKeys[GI + 1024] = g_SortBuffer[RangeStart + GI + 1024];

	GroupMemoryBarrierWithGroupSync();

	for (uint j = 1024; j > 0; j >>= 1)
	{
		uint Index1 = InsertZeroBit(GI, j);
		uint Index2 = Index1 | j;

		uint A = gs_SortKeys[Index1];
		uint B = gs_SortKeys[Index2];

		if ((A > B) != ((RangeStart & k) == 0))
		{
			gs_SortKeys[Index1] = B;
			gs_SortKeys[Index2] = A;
		}

		GroupMemoryBarrierWithGroupSync();
	}

	g_SortBuffer[RangeStart + GI] = gs_SortKeys[GI];
	g_SortBuffer[RangeStart + GI + 1024] = gs_SortKeys[GI + 1024];
}