// RUN: %dxc -E main -T cs_6_0 -O0 %s | FileCheck %s

// CHECK: threadId
// CHECK: bufferLoad
// CHECK: icmp ugt
// CHECK: and
// CHECK: icmp eq
// CHECK: icmp ne

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
	uint k;		// k >= 4096
	uint j;		// j >= 2048 && j < k
};

[RootSignature(Particle_RootSig)]
[numthreads(1024, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID  )
{
	// Form unique index pair from dispatch thread ID
	uint Index1 = InsertZeroBit(DTid.x, j);
	uint Index2 = Index1 | j;

	uint A = g_SortBuffer[Index1];
	uint B = g_SortBuffer[Index2];

	if ((A > B) != ((Index1 & k) == 0))
	{
		g_SortBuffer[Index1] = B;
		g_SortBuffer[Index2] = A;
	}
}