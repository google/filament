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
// Description:  A Compute-optimized implementation of FXAA 3.11 (PC Quality).  The
// improvements take advantage of work queues (RWStructuredBuffer with atomic counters)
// for these benefits:
// 
// 1) Split horizontal and vertical edge searches into separate dispatches to reduce
// shader complexity and incoherent branching.
// 2) Delay writing new pixel colors until after the source buffer has been fully
// analyzed.  This avoids the write-after-scattered-read hazard.
// 3) Modify source buffer in-place rather than ping-ponging buffers, which reduces
// bandwidth and memory demands.
//
// In addition to the above-mentioned benefits of using UAVs, the first pass also
// takes advantage of groupshared memory for storing luma values, further reducing
// fetches and bandwidth.
//
// Another optimization is in the generation of perceived brightness (luma) of pixels.
// The original implementation used sRGB as a good approximation of log-luminance.  A
// more precise representation of log-luminance allows the algorithm to operate with a
// higher threshold value while still finding perceivable edges across the full range
// of brightness.  The approximation used here is (1 - 2^(-4L)) * 16/15, where L =
// dot( LinearRGB, float3(0.212671, 0.715160, 0.072169) ).  A threshold of 0.2 is
// recommended with log-luminance computed this way.
//

// Original Boilerplate:
//
/*============================================================================


                    NVIDIA FXAA 3.11 by TIMOTHY LOTTES


------------------------------------------------------------------------------
COPYRIGHT (C) 2010, 2011 NVIDIA CORPORATION. ALL RIGHTS RESERVED.
------------------------------------------------------------------------------
TO THE MAXIMUM EXTENT PERMITTED BY APPLICABLE LAW, THIS SOFTWARE IS PROVIDED
*AS IS* AND NVIDIA AND ITS SUPPLIERS DISCLAIM ALL WARRANTIES, EITHER EXPRESS
OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL NVIDIA
OR ITS SUPPLIERS BE LIABLE FOR ANY SPECIAL, INCIDENTAL, INDIRECT, OR
CONSEQUENTIAL DAMAGES WHATSOEVER (INCLUDING, WITHOUT LIMITATION, DAMAGES FOR
LOSS OF BUSINESS PROFITS, BUSINESS INTERRUPTION, LOSS OF BUSINESS INFORMATION,
OR ANY OTHER PECUNIARY LOSS) ARISING OUT OF THE USE OF OR INABILITY TO USE
THIS SOFTWARE, EVEN IF NVIDIA HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH
DAMAGES.
*/

#include "FXAARootSignature.hlsli"

cbuffer ConstantBuffer_x : register( b0 )
{
	float2 RcpTextureSize;
	float ContrastThreshold;	// default = 0.2, lower is more expensive
	float SubpixelRemoval;		// default = 0.75, lower blurs less
};

RWStructuredBuffer<uint> HWork : register(u0);
RWStructuredBuffer<uint> VWork : register(u2);
RWBuffer<float3> HColor : register(u1);
RWBuffer<float3> VColor : register(u3);
Texture2D<float3> Color : register(t0);
SamplerState LinearSampler : register(s0);

#define BOUNDARY_SIZE 1
#define ROW_WIDTH (8 + BOUNDARY_SIZE * 2)
groupshared float gs_LumaCache[ROW_WIDTH * ROW_WIDTH];

// If pre-computed, source luminance as a texture, otherwise write it out for Pass2
#ifdef USE_LUMA_INPUT_BUFFER
	Texture2D<float> Luma : register(t1);
#else
	RWTexture2D<float> Luma : register(u4);
#endif

//
// Helper functions
//
float RGBToLogLuminance( float3 LinearRGB )
{
	float Luma = dot( LinearRGB, float3(0.212671, 0.715160, 0.072169) );
	return log2(1 + Luma * 15) / 4;
}

[RootSignature(FXAA_RootSig)]
[numthreads( 8, 8, 1 )]
void main( uint3 Gid : SV_GroupID, uint GI : SV_GroupIndex, uint3 GTid : SV_GroupThreadID, uint3 DTid : SV_DispatchThreadID )
{
#ifdef USE_LUMA_INPUT_BUFFER
	// Load 4 lumas per thread into LDS (but only those needed to fill our pixel cache)
	if (max(GTid.x, GTid.y) < ROW_WIDTH / 2)
	{
		int2 ThreadUL = DTid.xy + GTid.xy - (BOUNDARY_SIZE - 1);
		float4 Luma4 = Luma.Gather(LinearSampler, ThreadUL * RcpTextureSize);
		uint LoadIndex = (GTid.x + GTid.y * ROW_WIDTH) * 2;
		gs_LumaCache[LoadIndex                ] = Luma4.w;
		gs_LumaCache[LoadIndex + 1            ] = Luma4.z;
		gs_LumaCache[LoadIndex + ROW_WIDTH    ] = Luma4.x;
		gs_LumaCache[LoadIndex + ROW_WIDTH + 1] = Luma4.y;
	}
#else
	// Because we can't use Gather() on RGB, we make each thread read two pixels (but only those needed).
	if (GI < ROW_WIDTH * ROW_WIDTH / 2)
	{
		uint LdsCoord = GI;
		int2 UavCoord = uint2(GI % ROW_WIDTH, GI / ROW_WIDTH) + Gid.xy * 8 - BOUNDARY_SIZE;
		float Luma1 = RGBToLogLuminance( Color[UavCoord] );
		Luma[UavCoord] = Luma1;
		gs_LumaCache[LdsCoord] = Luma1;

		LdsCoord += ROW_WIDTH * ROW_WIDTH / 2;
		UavCoord += int2(0, ROW_WIDTH / 2);
		float Luma2 = RGBToLogLuminance( Color[UavCoord] );
		Luma[UavCoord] = Luma2;
		gs_LumaCache[LdsCoord] = Luma2;
	}
#endif

	GroupMemoryBarrierWithGroupSync();

	uint CenterIdx = (GTid.x + BOUNDARY_SIZE) + (GTid.y + BOUNDARY_SIZE) * ROW_WIDTH;

	// Load the ordinal and center luminances
	float lumaN  = gs_LumaCache[CenterIdx - ROW_WIDTH];
	float lumaW  = gs_LumaCache[CenterIdx - 1];
	float lumaM  = gs_LumaCache[CenterIdx];
	float lumaE  = gs_LumaCache[CenterIdx + 1];
	float lumaS  = gs_LumaCache[CenterIdx + ROW_WIDTH];

	// Contrast threshold test
	float rangeMax = max(max(lumaN, lumaW), max(lumaE, max(lumaS, lumaM)));
	float rangeMin = min(min(lumaN, lumaW), min(lumaE, min(lumaS, lumaM)));
	float range = rangeMax - rangeMin;
	if (range < ContrastThreshold)
		return;

	// Load the corner luminances
	float lumaNW = gs_LumaCache[CenterIdx - ROW_WIDTH - 1];
	float lumaNE = gs_LumaCache[CenterIdx - ROW_WIDTH + 1];
	float lumaSW = gs_LumaCache[CenterIdx + ROW_WIDTH - 1];
	float lumaSE = gs_LumaCache[CenterIdx + ROW_WIDTH + 1];

	// Pre-sum a few terms so the results can be reused
	float lumaNS = lumaN + lumaS;
	float lumaWE = lumaW + lumaE;
	float lumaNWSW = lumaNW + lumaSW;
	float lumaNESE = lumaNE + lumaSE;
	float lumaSWSE = lumaSW + lumaSE;
	float lumaNWNE = lumaNW + lumaNE;

	// Compute horizontal and vertical contrast; see which is bigger
	float edgeHorz = abs(lumaNWSW - 2.0 * lumaW) + abs(lumaNS - 2.0 * lumaM) * 2.0 + abs(lumaNESE - 2.0 * lumaE);
	float edgeVert = abs(lumaSWSE - 2.0 * lumaS) + abs(lumaWE - 2.0 * lumaM) * 2.0 + abs(lumaNWNE - 2.0 * lumaN);

	// Also compute local contrast in the 3x3 region.  This can identify standalone pixels that alias.
	float avgNeighborLuma = ((lumaNS + lumaWE) * 2.0 + lumaNWSW + lumaNESE) / 12.0;
	float subpixelShift = saturate(pow(smoothstep(0, 1, abs(avgNeighborLuma - lumaM) / range), 2) * SubpixelRemoval * 2);

	float NegGrad = (edgeHorz >= edgeVert ? lumaN : lumaW) - lumaM;
	float PosGrad = (edgeHorz >= edgeVert ? lumaS : lumaE) - lumaM;
	uint GradientDir = abs(PosGrad) >= abs(NegGrad) ? 1 : 0;
	uint Subpix = uint(subpixelShift * 254.0) & 0xFE;
	uint PixelCoord = DTid.y << 20 | DTid.x << 8;

	// Packet header: [ 12 bits Y | 12 bits X | 7 bit Subpix | 1 bit dir(Grad) ]
	uint WorkHeader = PixelCoord | Subpix | GradientDir;

	if (edgeHorz >= edgeVert)
	{
		uint WorkIdx = HWork.IncrementCounter();
		HWork[WorkIdx] = WorkHeader;
		HColor[WorkIdx] = Color[DTid.xy + uint2(0, 2 * GradientDir - 1)];
	}
	else
	{
		uint WorkIdx = VWork.IncrementCounter();
		VWork[WorkIdx] = WorkHeader;
		VColor[WorkIdx] = Color[DTid.xy + uint2(2 * GradientDir - 1, 0)];
	}
}
