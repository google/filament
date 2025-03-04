// RUN: %dxc -E main -T cs_6_0 %s | FileCheck %s

// CHECK: flattenedThreadIdInGroup
// CHECK: barrier
// CHECK: addrspace(3)
// CHECK: barrier
// CHECK: FMax
// CHECK: Exp
// CHECK: FMin
// CHECK: Round_ni
// CHECK: FAbs

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
// The group size is 16x16, but one group iterates over an entire 16-wide column of pixels (384 pixels tall)
// Assuming the total workspace is 640x384, there will be 40 thread groups computing the histogram in parallel.
// The histogram measures logarithmic luminance ranging from 2^-12 up to 2^4.  This should provide a nice window
// where the exposure would range from 2^-4 up to 2^4.

#include "PostEffectsRS.hlsli"

ByteAddressBuffer Histogram : register(t0);
RWStructuredBuffer<float> Exposure : register(u0);

cbuffer cb0 : register(b1)
{
	float TargetLuminance;
	float AdaptationRate;
	float MinExposure;
	float MaxExposure;
	float PeakIntensity;
	float PixelCount; 
}

groupshared float gs_Accum[256];

[RootSignature(PostEffects_RootSig)]
[numthreads( 256, 1, 1 )]
void main( uint GI : SV_GroupIndex )
{
	float WeightedSum = (float)GI * (float)Histogram.Load(GI * 4);

	[unroll]
	for (uint i = 1; i < 256; i *= 2)
	{
		gs_Accum[GI] = WeightedSum;					// Write
		GroupMemoryBarrierWithGroupSync();			// Sync
		WeightedSum += gs_Accum[(GI + i) % 256];	// Read
		GroupMemoryBarrierWithGroupSync();			// Sync
	}

	float MinLog = Exposure[4];
	float MaxLog = Exposure[5];
	float LogRange = Exposure[6];
	float RcpLogRange = Exposure[7];

	// Average histogram value is the weighted sum of all pixels divided by the total number of pixels
	// minus those pixels which provided no weight (i.e. black pixels.)
	float weightedHistAvg = WeightedSum / (max(1, PixelCount - Histogram.Load(0))) - 1.0;
	float logAvgLuminance = exp2(weightedHistAvg * LogRange / 254.0 + MinLog);
	float targetExposure = TargetLuminance / logAvgLuminance;
	float exposure = Exposure[0];
	exposure = lerp(exposure, targetExposure, AdaptationRate);
	exposure = clamp(exposure, MinExposure, MaxExposure);

	if (GI == 0)
	{
		Exposure[0] = exposure;
		Exposure[1] = 1.0 / exposure;
		Exposure[2] = exposure / PeakIntensity;
		Exposure[3] = weightedHistAvg;

		// First attempt to recenter our histogram around the log-average.
		float biasToCenter = (floor(weightedHistAvg) - 128.0) / 255.0;//
		if (abs(biasToCenter) > 0.1)
		{
			MinLog += biasToCenter * RcpLogRange;
			MaxLog += biasToCenter * RcpLogRange;
		}

		// TODO:  Increase or decrease the log range to better fit the range of values.
		// (Idea) Look at intermediate log-weighted sums for under- or over-represented
		// extreme bounds.  I.e. break the for loop into two pieces to compute the sum of
		// groups of 16, check the groups on each end, then finish the recursive summation.

		Exposure[4] = MinLog;
		Exposure[5] = MaxLog;
		Exposure[6] = LogRange;
		Exposure[7] = 1.0 / LogRange;
	}
}
