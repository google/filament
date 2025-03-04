// RUN: %dxc -E main -T cs_6_0 %s | FileCheck %s

// CHECK: groupId
// CHECK: flattenedThreadIdInGroup
// CHECK: threadId
// CHECK: textureGather
// CHECK: textureGather
// CHECK: textureGather
// CHECK: textureGather
// CHECK: FMax
// CHECK: FMin
// CHECK: barrier
// CHECK: UMax
// CHECK: atomicrmw umax
// CHECK: Saturate
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
// Author(s):  James Stanard 
//             Alex Nankervis
//

#include "ParticleUtility.hlsli"

Texture2D<float> g_Input : register(t0);
RWTexture2D<uint> g_Output8 : register(u0);
RWTexture2D<uint> g_Output16 : register(u1);
RWTexture2D<uint> g_Output32 : register(u2);

groupshared uint gs_Buffer[128];

void Max4( uint This, uint Dx )
{
	uint MM1 = gs_Buffer[This + 1 * Dx];
	uint MM2 = gs_Buffer[This + 8 * Dx];
	uint MM3 = gs_Buffer[This + 9 * Dx];
	GroupMemoryBarrierWithGroupSync();
	InterlockedMax(gs_Buffer[This], max(MM1, max(MM2, MM3)));
	GroupMemoryBarrierWithGroupSync();
}

uint PackMinMax( uint This )
{
	float Min = asfloat(~gs_Buffer[This + 64]);
	float Max = asfloat(gs_Buffer[This]);
	return f32tof16(Max) << 16 | f32tof16(saturate(Min - 0.001));
}

[RootSignature(Particle_RootSig)]
[numthreads( 8, 8, 1 )]
void main( uint3 Gid : SV_GroupID, uint GI : SV_GroupIndex, uint3 DTid : SV_DispatchThreadID )
{
	// Load 4x4 depth values (per thread) and compute the min and max of each
	float2 UV1 = (DTid.xy * 4 + 1) * gRcpBufferDim;
	float2 UV2 = UV1 + float2(2, 0) * gRcpBufferDim;
	float2 UV3 = UV1 + float2(0, 2) * gRcpBufferDim;
	float2 UV4 = UV1 + float2(2, 2) * gRcpBufferDim;
	
	float4 ZQuad1 = g_Input.Gather(gSampPointClamp, UV1);
	float4 ZQuad2 = g_Input.Gather(gSampPointClamp, UV2);
	float4 ZQuad3 = g_Input.Gather(gSampPointClamp, UV3);
	float4 ZQuad4 = g_Input.Gather(gSampPointClamp, UV4);

	float4 MaxQuad = max(max(ZQuad1, ZQuad2), max(ZQuad3, ZQuad4));
	float4 MinQuad = min(min(ZQuad1, ZQuad2), min(ZQuad3, ZQuad4));

	float maxZ = max(max(MaxQuad.x, MaxQuad.y), max(MaxQuad.z, MaxQuad.w));
	float minZ = min(min(MinQuad.x, MinQuad.y), min(MinQuad.z, MinQuad.w));

	// Parallel reduction will reduce 4:1 per iteration.  This reduces LDS loads and stores
	// and can take advantage of min3 and max3 instructions when available.

	// Because each iteration puts 3/4 of active threads to sleep, threads are quickly wasted.
	// Rather than have each active thread compute both a min and a max, it would be nice if
	// we could wake up sleeping threads to share the burden.  It turns out this is possible!
	// We can have all threads performing Max4() reductions, and by applying it to negative
	// min values, we can find the min depth.  E.g. min(a, b) = -max(-a, -b)

	// Max values to first 64, Min values to last 64
	gs_Buffer[GI] = asuint(maxZ);
	gs_Buffer[GI + 64] = ~asuint(minZ);
	GroupMemoryBarrierWithGroupSync();

	// We don't need odd numbered threads, but we could utilize more threads
	const uint This = GI * 2;

	Max4(This, 1);

	// if (X % 2 == 0 && Y % 2 == 0 && Y < 8)
	if ((This & 0x49) == 0)	
	{
		uint2 SubTile = uint2(This >> 1, This >> 4) & 3;
		g_Output8[Gid.xy * 4 + SubTile] = PackMinMax(This);
	}

	Max4(This, 2);

	// if (X % 4 == 0 && Y % 4 == 0 && Y < 8)
	if ((This & 0x5B) == 0)	
	{
		uint2 SubTile = uint2(This >> 2, This >> 5) & 1;
		g_Output16[Gid.xy * 2 + SubTile] = PackMinMax(This);
	}

	Max4(This, 4);

	if (This == 0)
		g_Output32[Gid.xy] = PackMinMax(This);
}
