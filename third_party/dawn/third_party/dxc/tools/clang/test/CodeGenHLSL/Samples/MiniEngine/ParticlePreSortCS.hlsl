// RUN: %dxc -E main -T cs_6_0 %s | FileCheck %s

// CHECK: groupId
// CHECK: flattenedThreadIdInGroup
// CHECK: FAbs
// CHECK: FMax
// CHECK: Saturate
// CHECK: barrier
// CHECK: addrspace(3)
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

#include "ParticleUtility.hlsli"

StructuredBuffer<ParticleVertex> g_VertexBuffer : register( t0 );
ByteAddressBuffer g_VertexCount : register(t1);
RWStructuredBuffer<uint> g_SortBuffer : register(u0);
//RWByteAddressBuffer g_VisibleCount : register(u1);

groupshared uint gs_SortKeys[2048];

void FillSortKey( uint GroupStart, uint Offset, uint VertexCount )
{
	if (GroupStart + Offset >= VertexCount)
	{
		gs_SortKeys[Offset] = 0;		// Z = 0 will sort to the end of the list (back to front)
		return;
	}

	uint VertexIdx = GroupStart + Offset;
	ParticleVertex Sprite = g_VertexBuffer[VertexIdx];

	// Frustum cull before adding this particle to list of visible particles (for rendering)
	float4 HPos = mul( gViewProj, float4(Sprite.Position, 1) );
	float Height = Sprite.Size * gVertCotangent;
	float Width = Height * gAspectRatio;
	float3 Extent = abs(HPos.xyz) - float3(Width, Height, 0);

	// Frustum cull rather than sorting and rendering every particle
	if (max(max(0.0, Extent.x), max(Extent.y, Extent.z)) <= HPos.w)
	{
		// Encode depth as 14 bits because we only need [0, 1] at half precision.
		// This gives us 18-bit indices--up to 256k particles.
		float Depth = saturate(HPos.w * gRcpFarZ);
		gs_SortKeys[Offset] = f32tof16(Depth) << 18 | VertexIdx;

		// We should keep track of how many visible particles there are, but it probably would require a separate
		// shader pass so that we don't end up sorting non-visible particles.
		//g_VisibleCount.InterlockedAdd(0, 1);
	}
	else
	{
		// Until we can remove non-visible particles, we need to provide the actual index of the off-screen
		// particle.  It will get culled again by the rasterizer.
		gs_SortKeys[Offset] = VertexIdx;
	}
}

[RootSignature(Particle_RootSig)]
[numthreads(1024, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID, uint3 Gid : SV_GroupID, uint GI : SV_GroupIndex )
{
	uint VisibleParticles = g_VertexCount.Load(4);

	uint GroupStart = Gid.x * 2048;

	if (GroupStart > VisibleParticles)
	{
		g_SortBuffer[GroupStart + GI] = 0;
		g_SortBuffer[GroupStart + GI + 1024] = 0;
		return;
	}

	FillSortKey(GroupStart, GI, VisibleParticles);
	FillSortKey(GroupStart, GI + 1024, VisibleParticles);

	GroupMemoryBarrierWithGroupSync();

	for (uint k = 2; k <= 2048; k <<= 1)
	{
		for (uint j = k >> 1; j > 0; j >>= 1)
		{
			uint Index1 = InsertZeroBit(GI, j);
			uint Index2 = Index1 | j;

			uint A = gs_SortKeys[Index1];
			uint B = gs_SortKeys[Index2];

			if ((A > B) != (((GroupStart + Index1) & k) == 0))
			{
				gs_SortKeys[Index1] = B;
				gs_SortKeys[Index2] = A;
			}

			GroupMemoryBarrierWithGroupSync();
		}
	}


	g_SortBuffer[GroupStart + GI] = gs_SortKeys[GI];
	g_SortBuffer[GroupStart + GI + 1024] = gs_SortKeys[GI + 1024];
}