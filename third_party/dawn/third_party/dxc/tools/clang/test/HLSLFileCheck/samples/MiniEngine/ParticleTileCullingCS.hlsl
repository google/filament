// RUN: %dxc -E main -T cs_6_0 %s | FileCheck %s

// CHECK: groupId
// CHECK: flattenedThreadIdInGroup
// CHECK: threadIdInGroup
// CHECK: bufferLoad
// CHECK: textureLoad
// CHECK: UMin
// CHECK: Countbits
// CHECK: FirstbitHi
// CHECK: barrier
// CHECK: bufferStore
// CHECK: IMax
// CHECK: IMin
// CHECK: bufferStore
// CHECK: AtomicAdd

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
// Author(s):   James Stanard 
//              Julia Careaga
//

#include "ParticleUtility.hlsli"

StructuredBuffer<uint> g_BinParticles : register(t0);
StructuredBuffer<uint> g_BinCounters : register(t1);
Texture2D<uint> g_DepthBounds : register(t2);
StructuredBuffer<ParticleScreenData> g_VisibleParticles : register(t3);

RWStructuredBuffer<uint> g_SortedParticles : register(u0);
RWByteAddressBuffer g_TileHitMasks : register(u1);
RWStructuredBuffer<uint> g_DrawPackets : register(u2);
RWStructuredBuffer<uint> g_FastDrawPackets : register(u3);
RWByteAddressBuffer g_DrawPacketCount : register(u4);

#if TILES_PER_BIN < 64
#define GROUP_THREAD_COUNT 64
#else
#define GROUP_THREAD_COUNT TILES_PER_BIN
#endif
#define GROUP_SIZE_X TILES_PER_BIN_X
#define GROUP_SIZE_Y (GROUP_THREAD_COUNT / GROUP_SIZE_X)
#define MASK_WORDS_PER_ITER (GROUP_THREAD_COUNT / 32)

groupshared uint gs_SortKeys[MAX_PARTICLES_PER_BIN];
groupshared uint gs_IntersectionMasks[TILES_PER_BIN * MASK_WORDS_PER_ITER];
groupshared uint gs_TileParticleCounts[TILES_PER_BIN];
groupshared uint gs_SlowTileParticleCounts[TILES_PER_BIN];
groupshared uint gs_MinMaxDepth[TILES_PER_BIN];

void BitonicSort(uint GI, uint NumElements, uint NextPow2, uint NumThreads)
{
	for (uint k = 2; k <= NextPow2; k *= 2)
	{
		// Align NumElements to the next multiple of k
		NumElements = (NumElements + k - 1) & ~(k - 1);

		for (uint j = k / 2; j > 0; j /= 2)
		{
			// Loop over all N/2 unique element pairs
			for (uint i = GI; i < NumElements / 2; i += NumThreads)
			{
				uint Index1 = InsertZeroBit(i, j);
				uint Index2 = Index1 | j;

				uint A = gs_SortKeys[Index1];
				uint B = gs_SortKeys[Index2];

				if ((A < B) != ((Index1 & k) == 0))
				{
					gs_SortKeys[Index1] = B;
					gs_SortKeys[Index2] = A;
				}
			}

			GroupMemoryBarrierWithGroupSync();
		}
	}
}

uint ComputeMaskOffset( uint2 Gid, uint2 GTid )
{
	// Sometimes we have more threads than tiles per bin.
	uint2 OutTileCoord = Gid.xy * uint2(TILES_PER_BIN_X, TILES_PER_BIN_Y) + uint2(GTid.x, GTid.y % TILES_PER_BIN_Y);
	uint OutTileIdx = OutTileCoord.x + OutTileCoord.y * gTileRowPitch;
	return OutTileIdx * MAX_PARTICLES_PER_BIN / 8 + GTid.y / TILES_PER_BIN_Y * 4;
}

[RootSignature(Particle_RootSig)]
[numthreads(GROUP_SIZE_X, GROUP_SIZE_Y, 1)]
void main( uint3 Gid : SV_GroupID, uint GI : SV_GroupIndex, uint3 GTid : SV_GroupThreadID )
{
	// Each group is assigned a bin
	uint BinIndex = Gid.y * gBinsPerRow + Gid.x;

	uint ParticleCountInBin = g_BinCounters[BinIndex];
	if (ParticleCountInBin == 0)	
		return;

	// Get the start location for particles in this bin
	uint BinStart = BinIndex * MAX_PARTICLES_PER_BIN;

	// Each thread is assigned a tile
	uint2 TileCoord = Gid.xy * uint2(TILES_PER_BIN_X, TILES_PER_BIN_Y) + GTid.xy;

	if (GI < TILES_PER_BIN)
	{
		gs_TileParticleCounts[GI] = 0;
		gs_SlowTileParticleCounts[GI] = 0;
		gs_MinMaxDepth[GI] = g_DepthBounds[TileCoord] << 2;
	}

	// Sometimes the counter value exceeds the actual storage size
	ParticleCountInBin = min(MAX_PARTICLES_PER_BIN, ParticleCountInBin);

	// Compute the next power of two for the bitonic sort
	uint NextPow2 = countbits(ParticleCountInBin) <= 1 ? ParticleCountInBin : (2 << firstbithigh(ParticleCountInBin));

	// Fill in the sort key array.  Each sort key has passenger data (in the least signficant
	// bits, so that as the sort keys are moved around, they retain a pointer to the particle
	// they refer to.
	for (uint k = GI; k < NextPow2; k += GROUP_THREAD_COUNT)
		gs_SortKeys[k] = k < ParticleCountInBin ? g_BinParticles[BinStart + k] : 0xffffffff;

	GroupMemoryBarrierWithGroupSync();

	// Sort the particles from front to back.
	BitonicSort(GI, ParticleCountInBin, NextPow2, GROUP_THREAD_COUNT);

	// Upper-left tile coord and lower-right coord, clamped to the screen
	const int2 StartTile = Gid.xy * uint2(TILES_PER_BIN_X, TILES_PER_BIN_Y);

	// Each thread writes the hit mask for one tile
	uint OutOffsetInBytes = ComputeMaskOffset(Gid.xy, GTid.xy);

	// Loop over all sorted particles, group-size count at a time
	for (uint Iter = 0; Iter < ParticleCountInBin; Iter += GROUP_THREAD_COUNT)
	{
		// Reset temporary particle intersection masks.  There are two words (64-bits) per thread.
    // [unroll] // Change to allow new unroll behavior.
		for (uint C = GI; C < TILES_PER_BIN * MASK_WORDS_PER_ITER; C += GROUP_THREAD_COUNT)
			gs_IntersectionMasks[C] = 0;

		GroupMemoryBarrierWithGroupSync();

		// The array index of the particle this thread will test
		uint SortIdx = Iter + GI;

		// Compute word and bit to set (from thread index)
		uint WordOffset = GI >> 5;
		uint BitOffset = GI & 31;

		// Only do the loads and stores if this is a valid index (see constant number of iterations comment above)
		if (SortIdx < ParticleCountInBin)
		{
			uint SortKey = gs_SortKeys[SortIdx];
			uint GlobalIdx = SortKey & 0x3FFFF;

			// After this phase, all we care about is its global index
			g_SortedParticles[BinStart + SortIdx] = SortKey;

			uint Bounds = g_VisibleParticles[GlobalIdx].Bounds;
			int2 MinTile = uint2(Bounds >>  0, Bounds >>  8) & 0xFF;
			int2 MaxTile = uint2(Bounds >> 16, Bounds >> 24) & 0xFF;
			MinTile = max(MinTile - StartTile, 0);
			MaxTile = min(MaxTile - StartTile, int2(TILES_PER_BIN_X, TILES_PER_BIN_Y) - 1);

			for (int y = MinTile.y; y <= MaxTile.y; y++)
			{
				for (int x = MinTile.x; x <= MaxTile.x; x++)
				{
					uint TileIndex = y * TILES_PER_BIN_X + x;
					uint TileMaxZ = gs_MinMaxDepth[TileIndex];
					uint Inside = SortKey < TileMaxZ ? 1 : 0;
					uint SlowPath = SortKey > (TileMaxZ << 16) ? Inside : 0;
					InterlockedAdd(gs_SlowTileParticleCounts[TileIndex], SlowPath);
					InterlockedOr(gs_IntersectionMasks[TileIndex * MASK_WORDS_PER_ITER + WordOffset], Inside << BitOffset);
				}
			}
		}

		GroupMemoryBarrierWithGroupSync();

#if TILES_PER_BIN < GROUP_THREAD_COUNT
		// Copy the hit masks from LDS to the output buffer.  Here, each thread copies a single word
		if (GI < TILES_PER_BIN * MASK_WORDS_PER_ITER)
		{
			uint TileIndex = GI % TILES_PER_BIN;
			uint Offset = TileIndex * MASK_WORDS_PER_ITER + (GI / TILES_PER_BIN);
			uint Mask = gs_IntersectionMasks[Offset];
			InterlockedAdd(gs_TileParticleCounts[TileIndex], countbits(Mask));
			g_TileHitMasks.Store(OutOffsetInBytes, Mask);
			OutOffsetInBytes += 8;
		}
#else
		// Copy the hit masks from LDS to the output buffer.  Here, each thread is assigned a tile.
		uint Offset = GI * MASK_WORDS_PER_ITER;
		[unroll]
		for (uint O = 0; O < MASK_WORDS_PER_ITER; O += 2)
		{
			uint Mask0 = gs_IntersectionMasks[Offset+O];
			uint Mask1 = gs_IntersectionMasks[Offset+O+1];
			InterlockedAdd(gs_TileParticleCounts[GI], countbits(Mask0) + countbits(Mask1));
			g_TileHitMasks.Store2( OutOffsetInBytes, uint2(Mask0, Mask1) );
			OutOffsetInBytes += 8;
		}
#endif

		GroupMemoryBarrierWithGroupSync();
	}

	if (GI >= TILES_PER_BIN)
		return;

	uint ParticleCountInThisThreadsTile = gs_TileParticleCounts[GI];
	if (ParticleCountInThisThreadsTile > 0)
	{
		uint SlowParticlesInThisThreadsTile = gs_SlowTileParticleCounts[GI];
		uint Packet = TileCoord.x << 16 | TileCoord.y << 24 | ParticleCountInThisThreadsTile;

		uint NewPacketIndex;
		if (SlowParticlesInThisThreadsTile > 0)
		{
			g_DrawPacketCount.InterlockedAdd(0, 1, NewPacketIndex);
			g_DrawPackets[NewPacketIndex] = Packet;
		}
		else
		{
			g_DrawPacketCount.InterlockedAdd(12, 1, NewPacketIndex);
			g_FastDrawPackets[NewPacketIndex] = Packet;
		}
	}
}
