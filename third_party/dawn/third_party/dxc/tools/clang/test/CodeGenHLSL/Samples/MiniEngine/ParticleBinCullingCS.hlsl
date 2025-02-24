// RUN: %dxc -E main -T cs_6_0 %s | FileCheck %s

// CHECK: groupId
// CHECK: flattenedThreadIdInGroup
// CHECK: threadIdInGroup
// CHECK: bufferLoad
// CHECK: UMin
// CHECK: addrspace(3)
// CHECK: barrier
// CHECK: UMax
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
// Author(s):  James Stanard 
//             Julia Careaga
//

#include "ParticleUtility.hlsli"

#define THREAD_GROUP_SIZE 256

StructuredBuffer<ParticleScreenData> g_VisibleParticles : register( t0 );
StructuredBuffer<uint> g_LargeBinParticles : register( t1 );
StructuredBuffer<uint> g_LargeBinCounters : register( t2 );
RWStructuredBuffer<uint> g_BinParticles : register( u0 );
RWStructuredBuffer<uint> g_BinCounters : register( u1 );

groupshared uint gs_BinCounters[16];

cbuffer CB : register(b0)
{
	uint2 LogTilesPerBin;
};

[RootSignature(Particle_RootSig)]
[numthreads(4, THREAD_GROUP_SIZE / 4, 1)]
void main( uint3 Gid : SV_GroupID, uint GI : SV_GroupIndex, uint3 GTid : SV_GroupThreadID )
{
	uint LargeBinsPerRow = (gBinsPerRow + 3) / 4;
	uint ParticlesPerLargeBin = MAX_PARTICLES_PER_BIN * 16;

	uint LargeBinIndex = Gid.y * LargeBinsPerRow + Gid.x;
	uint ParticleCountInLargeBin = min(g_LargeBinCounters[LargeBinIndex], ParticlesPerLargeBin);

	// Get the start location for particles in this bin
	uint LargeBinStart = LargeBinIndex * ParticlesPerLargeBin;
	uint2 FirstBin = Gid.xy * 4;

	if (GI < 16)
		gs_BinCounters[GI] = 0;

	GroupMemoryBarrierWithGroupSync();

	for (uint idx = GI; idx < ParticleCountInLargeBin; idx += THREAD_GROUP_SIZE)
	{
		uint SortKey = g_LargeBinParticles[LargeBinStart + idx]; 
		uint GlobalIdx = SortKey & 0x3FFFF;

		uint Bounds = g_VisibleParticles[GlobalIdx].Bounds;
		uint2 MinTile = uint2(Bounds >>  0, Bounds >>  8) & 0xFF;
		uint2 MaxTile = uint2(Bounds >> 16, Bounds >> 24) & 0xFF;
		uint2 MinBin = max(MinTile >> LogTilesPerBin, FirstBin);
		uint2 MaxBin = min(MaxTile >> LogTilesPerBin, FirstBin + 3);

		for (uint y = MinBin.y; y <= MaxBin.y; ++y)
		{
			for (uint x = MinBin.x; x <= MaxBin.x; ++x)
			{
				uint CounterIdx = (x & 3) | (y & 3) << 2;
				uint BinOffset = (x + y * gBinsPerRow) * MAX_PARTICLES_PER_BIN;
				uint AllocIdx;
				InterlockedAdd(gs_BinCounters[CounterIdx], 1, AllocIdx);
				AllocIdx = min(AllocIdx, MAX_PARTICLES_PER_BIN - 1);
				g_BinParticles[BinOffset + AllocIdx] = SortKey;
			}
		}
	}

	GroupMemoryBarrierWithGroupSync();

	if (GI < 16)
	{
		uint2 OutBin = FirstBin + GTid.xy;
		g_BinCounters[OutBin.x + OutBin.y * gBinsPerRow] = gs_BinCounters[GI];
	}
}