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

#ifndef INTERLEAVE_RESULT
#define WIDE_SAMPLING 1
#endif

#ifdef INTERLEAVE_RESULT
Texture2DArray<float> DepthTex : register(t0);
#else
Texture2D<float> DepthTex : register(t0);
#endif
RWTexture2D<float> Occlusion : register(u0);
SamplerState LinearBorderSampler : register(s1);
cbuffer ConstantBuffer_x : register(b1)
{
	float4 gInvThicknessTable[3];
	float4 gSampleWeightTable[3];
	float2 gInvSliceDimension;
	float  gRejectFadeoff;
	float  gRcpAccentuation;
}

#if WIDE_SAMPLING
	// 32x32 cache size:  the 16x16 in the center forms the area of focus with the 8-pixel perimeter used for wide gathering.
	#define TILE_DIM 32
	#define THREAD_COUNT_X 16
	#define THREAD_COUNT_Y 16
#else
	// 16x16 cache size:  the 8x8 in the center forms the area of focus with the 4-pixel perimeter used for gathering.
	#define TILE_DIM 16
	#define THREAD_COUNT_X 8
	#define THREAD_COUNT_Y 8
#endif

groupshared float DepthSamples[TILE_DIM * TILE_DIM];

float TestSamplePair( float frontDepth, float invRange, uint base, int offset )
{
	// "Disocclusion" measures the penetration distance of the depth sample within the sphere.
	// Disocclusion < 0 (full occlusion) -> the sample fell in front of the sphere
	// Disocclusion > 1 (no occlusion) -> the sample fell behind the sphere
	float disocclusion1 = DepthSamples[base + offset] * invRange - frontDepth;
	float disocclusion2 = DepthSamples[base - offset] * invRange - frontDepth;

	float pseudoDisocclusion1 = saturate(gRejectFadeoff * disocclusion1);
	float pseudoDisocclusion2 = saturate(gRejectFadeoff * disocclusion2);

	return
		clamp(disocclusion1, pseudoDisocclusion2, 1.0) +
		clamp(disocclusion2, pseudoDisocclusion1, 1.0) -
		pseudoDisocclusion1 * pseudoDisocclusion2;
}

float TestSamples( uint centerIdx, uint x, uint y, float invDepth, float invThickness )
{
#if WIDE_SAMPLING
	x <<= 1;
	y <<= 1;
#endif

	float invRange = invThickness * invDepth;
	float frontDepth = invThickness - 0.5;

	if (y == 0)
	{
		// Axial
		return 0.5 * (
			TestSamplePair(frontDepth, invRange, centerIdx, x) +
			TestSamplePair(frontDepth, invRange, centerIdx, x * TILE_DIM));
	}
	else if (x == y)
	{
		// Diagonal
		return 0.5 * (
			TestSamplePair(frontDepth, invRange, centerIdx, x * TILE_DIM - x) +
			TestSamplePair(frontDepth, invRange, centerIdx, x * TILE_DIM + x));
	}
	else
	{
		// L-Shaped
		return 0.25 * (
			TestSamplePair(frontDepth, invRange, centerIdx, y * TILE_DIM + x) +
			TestSamplePair(frontDepth, invRange, centerIdx, y * TILE_DIM - x) +
			TestSamplePair(frontDepth, invRange, centerIdx, x * TILE_DIM + y) +
			TestSamplePair(frontDepth, invRange, centerIdx, x * TILE_DIM - y));
	}
}

[RootSignature(SSAO_RootSig)]
#if WIDE_SAMPLING
[numthreads( 16, 16, 1 )]
#else
[numthreads( 8, 8, 1 )]
#endif
void main( uint3 Gid : SV_GroupID, uint GI : SV_GroupIndex, uint3 GTid : SV_GroupThreadID, uint3 DTid : SV_DispatchThreadID )
{
#if WIDE_SAMPLING
	float2 QuadCenterUV = (DTid.xy + GTid.xy - 7) * gInvSliceDimension;
#else
	float2 QuadCenterUV = (DTid.xy + GTid.xy - 3) * gInvSliceDimension;
#endif

	// Fetch four depths and store them in LDS
#ifdef INTERLEAVE_RESULT
	float4 depths = DepthTex.Gather(LinearBorderSampler, float3(QuadCenterUV, DTid.z));
#else
	float4 depths = DepthTex.Gather(LinearBorderSampler, QuadCenterUV);
#endif
	int destIdx = GTid.x * 2 + GTid.y * 2 * TILE_DIM;
	DepthSamples[destIdx] = depths.w;
	DepthSamples[destIdx + 1] = depths.z;
	DepthSamples[destIdx + TILE_DIM] = depths.x;
	DepthSamples[destIdx + TILE_DIM + 1] = depths.y;

	GroupMemoryBarrierWithGroupSync();

#if WIDE_SAMPLING
	uint thisIdx = GTid.x + GTid.y * TILE_DIM + 8 * TILE_DIM + 8;
#else
	uint thisIdx = GTid.x + GTid.y * TILE_DIM + 4 * TILE_DIM + 4;
#endif
	const float invThisDepth = 1.0 / DepthSamples[thisIdx];

	float ao = 0.0;

//#define SAMPLE_EXHAUSTIVELY

#ifdef SAMPLE_EXHAUSTIVELY
	// 68 samples:  sample all cells in *within* a circular radius of 5
	ao += gSampleWeightTable[0].x * TestSamples(thisIdx, 1, 0, invThisDepth, gInvThicknessTable[0].x);
	ao += gSampleWeightTable[0].y * TestSamples(thisIdx, 2, 0, invThisDepth, gInvThicknessTable[0].y);
	ao += gSampleWeightTable[0].z * TestSamples(thisIdx, 3, 0, invThisDepth, gInvThicknessTable[0].z);
	ao += gSampleWeightTable[0].w * TestSamples(thisIdx, 4, 0, invThisDepth, gInvThicknessTable[0].w);
	ao += gSampleWeightTable[1].x * TestSamples(thisIdx, 1, 1, invThisDepth, gInvThicknessTable[1].x);
	ao += gSampleWeightTable[2].x * TestSamples(thisIdx, 2, 2, invThisDepth, gInvThicknessTable[2].x);
	ao += gSampleWeightTable[2].w * TestSamples(thisIdx, 3, 3, invThisDepth, gInvThicknessTable[2].w);
	ao += gSampleWeightTable[1].y * TestSamples(thisIdx, 1, 2, invThisDepth, gInvThicknessTable[1].y);
	ao += gSampleWeightTable[1].z * TestSamples(thisIdx, 1, 3, invThisDepth, gInvThicknessTable[1].z);
	ao += gSampleWeightTable[1].w * TestSamples(thisIdx, 1, 4, invThisDepth, gInvThicknessTable[1].w);
	ao += gSampleWeightTable[2].y * TestSamples(thisIdx, 2, 3, invThisDepth, gInvThicknessTable[2].y);
	ao += gSampleWeightTable[2].z * TestSamples(thisIdx, 2, 4, invThisDepth, gInvThicknessTable[2].z);
#else // SAMPLE_CHECKER
	// 36 samples:  sample every-other cell in a checker board pattern
	ao += gSampleWeightTable[0].y * TestSamples(thisIdx, 2, 0, invThisDepth, gInvThicknessTable[0].y);
	ao += gSampleWeightTable[0].w * TestSamples(thisIdx, 4, 0, invThisDepth, gInvThicknessTable[0].w);
	ao += gSampleWeightTable[1].x * TestSamples(thisIdx, 1, 1, invThisDepth, gInvThicknessTable[1].x);
	ao += gSampleWeightTable[2].x * TestSamples(thisIdx, 2, 2, invThisDepth, gInvThicknessTable[2].x);
	ao += gSampleWeightTable[2].w * TestSamples(thisIdx, 3, 3, invThisDepth, gInvThicknessTable[2].w);
	ao += gSampleWeightTable[1].z * TestSamples(thisIdx, 1, 3, invThisDepth, gInvThicknessTable[1].z);
	ao += gSampleWeightTable[2].z * TestSamples(thisIdx, 2, 4, invThisDepth, gInvThicknessTable[2].z);
#endif

#ifdef INTERLEAVE_RESULT
	uint2 OutPixel = DTid.xy << 2 | uint2(DTid.z & 3, DTid.z >> 2);
#else
	uint2 OutPixel = DTid.xy;
#endif
	Occlusion[OutPixel] = ao * gRcpAccentuation;
}