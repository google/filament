// RUN: %dxc -E main -T cs_6_0 %s | FileCheck %s

// CHECK: threadId
// CHECK: bufferLoad
// CHECK: FAbs
// CHECK: FMax
// CHECK: Log
// CHECK: Saturate
// CHECK: UMin
// CHECK: bufferUpdateCounter
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
// Author(s):  James Stanard 
//             Julia Careaga
//

#include "ParticleUtility.hlsli"

#define MAX_PARTICLES_PER_LARGE_BIN (16 * MAX_PARTICLES_PER_BIN)

StructuredBuffer<ParticleVertex> g_VertexBuffer : register(t0);
ByteAddressBuffer g_VertexCount : register(t1);
RWStructuredBuffer<uint> g_LargeBinParticles : register(u0);
RWStructuredBuffer<uint> g_LargeBinCounters : register(u1);
RWStructuredBuffer<ParticleScreenData> g_VisibleParticles : register( u2 );

cbuffer CB : register(b0)
{
	uint2 LogTilesPerLargeBin;
};

[RootSignature(Particle_RootSig)]
[numthreads(64, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
	uint VertexIdx = DTid.x;

	if (VertexIdx >= g_VertexCount.Load(4))
		return;

	//
	// Transform and cull the sprite
	//
	ParticleVertex Sprite = g_VertexBuffer[VertexIdx];

	// Frustum cull before adding this particle to list of visible particles (for rendering)
	float4 HPos = mul( gViewProj, float4(Sprite.Position, 1) );
	float Height = Sprite.Size * gVertCotangent;
	float Width = Height * gAspectRatio;
	float3 Extent = abs(HPos.xyz) - float3(Width, Height, 0);

	// Technically, we should check for HPos.z > 0 because this is D3D.  But there is only a tiny
	// window of space between the eye and the near plane where this could be true.
	if (max(max(0.0, Extent.x), max(Extent.y, Extent.z)) > HPos.w)
		return;

	//
	// Generate tile-relevant draw data
	//

	ParticleScreenData Particle;

	float RcpW = 1.0 / HPos.w;

	// Compute texture LOD for this sprite
	float ScreenSize = Height * RcpW * gBufferDim.y;
	float TextureLevel = (float)firstbithigh(MaxTextureSize) - log2(ScreenSize);

	Particle.Corner = float2(HPos.x - Width, -HPos.y - Height) * RcpW * 0.5 + 0.5;
	Particle.RcpSize = HPos.w / float2(Width, Height);
	Particle.Depth = saturate(HPos.w * gRcpFarZ);
	Particle.Color = Sprite.Color;
	Particle.TextureIndex = (float)Sprite.TextureID;
	Particle.TextureLevel = TextureLevel;

	float2 TopLeft = max(Particle.Corner * gBufferDim, 0.0);
	float2 BottomRight = max(TopLeft + gBufferDim / Particle.RcpSize, 0.0);
	uint2 EdgeTile = uint2(gTilesPerRow, gTilesPerCol) - 1;
	uint2 MinTile = uint2(TopLeft) / TILE_SIZE;
	uint2 MaxTile = min(EdgeTile, uint2(BottomRight) / TILE_SIZE);
	Particle.Bounds = MinTile.x | MinTile.y << 8 | MaxTile.x << 16 | MaxTile.y << 24;

	uint GlobalIdx = g_VisibleParticles.IncrementCounter();

	g_VisibleParticles[GlobalIdx] = Particle;

	//
	// Insert the particle into all large bins it occupies
	//

	uint LargeBinsPerRow = (gBinsPerRow + 3) / 4;
	uint2 MinLargeBin = MinTile >> LogTilesPerLargeBin;
	uint2 MaxLargeBin = MaxTile >> LogTilesPerLargeBin;

	uint SortKey = f32tof16(Particle.Depth) << 18 | GlobalIdx;

	for (uint y = MinLargeBin.y; y <= MaxLargeBin.y; y++)
	{
		for (uint x = MinLargeBin.x; x <= MaxLargeBin.x; x++)
		{
			uint LargeBinIndex = y * LargeBinsPerRow + x;
			uint AllocIdx;
			InterlockedAdd(g_LargeBinCounters[LargeBinIndex], 1, AllocIdx);
			AllocIdx = min(AllocIdx, MAX_PARTICLES_PER_LARGE_BIN - 1);
			g_LargeBinParticles[LargeBinIndex * MAX_PARTICLES_PER_LARGE_BIN + AllocIdx] = SortKey;
		}
	}
}