// RUN: %dxc -E main -T cs_6_0 %s | FileCheck %s

// CHECK: threadId
// CHECK: Sqrt
// CHECK: FMin
// CHECK: sampleLevel
// CHECK: sampleLevel
// CHECK: Round_pi
// CHECK: sampleLevel
// CHECK: sampleLevel

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

#include "MotionBlurRS.hlsli"

Texture2D<float3> SrcColor : register(t0);			// final output color (blurred and temporally blended)
Texture2D<float2> MotionBuffer : register(t1);		// full resolution motion vectors
Texture2D<float4> PrepBuffer : register(t2);		// 1/4 resolution pre-weighted blurred color samples
RWTexture2D<float3> DstColor : register(u0);		// final output color (blurred and temporally blended)

#ifdef TEMPORAL_UPSAMPLE
Texture2D<float4> TemporalIn : register(t3);		// saved result from last frame
RWTexture2D<float4> TemporalOut : register(u1);		// color to save for next frame including its validity in alpha
#endif

SamplerState LinearSampler : register(s0);


cbuffer c0 : register(b0)
{
	float2 RcpBufferDim;	// 1 / width, 1 / height
	uint MAX_SAMPLE_COUNT;
	float STEP_SIZE;
}

[RootSignature(MotionBlur_RootSig)]
[numthreads( 8, 8, 1 )]
void main( uint3 Gid : SV_GroupID, uint GI : SV_GroupIndex, uint3 GTid : SV_GroupThreadID, uint3 DTid : SV_DispatchThreadID )
{
	uint2 st = DTid.xy;
	float2 position = st + 0.5;
	float2 uv = position * RcpBufferDim;

	float2 motionVec = MotionBuffer[st] * 32;
	float3 thisColor = SrcColor[st];

	// Computing speed in this way will set the step size to two-pixel increments in the dominant
	// direction.
	float speed = length(motionVec);

	[branch]
	if (speed >= 4)
	{
		float4 accum = float4(thisColor, 1);

		// Half of the speed goes in each direction
		float halfSampleCount = min(MAX_SAMPLE_COUNT * 0.5, speed * 0.5 / STEP_SIZE);

		// Accumulate low-res, pre-weighted samples, summing their weights in alpha.
		// The center sample is skipped because we are alpha blending onto it in the
		// destination buffer.  Only its weight is considered.  Accumulating low-res
		// samples is not so egregious because the center weight is still high res.
		// Also, each of the low res samples is comprised of four pre-weighted high-
		// res samples, so they are effectively masked at full resolution.
		float2 deltaUV = motionVec / speed * RcpBufferDim * STEP_SIZE;
		float2 uv1 = uv;
		float2 uv2 = uv;

		// First accumulate the whole samples
		for (float i = halfSampleCount - 1.0; i > 0.0; i -= 1.0)
		{
			accum += PrepBuffer.SampleLevel(LinearSampler, uv1 += deltaUV, 0);
			accum += PrepBuffer.SampleLevel(LinearSampler, uv2 -= deltaUV, 0);
		}

		// This is almost the same as 'frac(halfSampleCount)' replaces 0 with 1.
		float remainder = 1 + halfSampleCount - ceil(halfSampleCount);

		// Then accumulate the fractional samples
		deltaUV *= remainder;
		accum += PrepBuffer.SampleLevel(LinearSampler, uv1 + deltaUV, 0) * remainder;
		accum += PrepBuffer.SampleLevel(LinearSampler, uv2 - deltaUV, 0) * remainder;

		thisColor = accum.rgb / accum.a;

#ifdef TEMPORAL_UPSAMPLE
		TemporalOut[st] = 0;
	}
	else
	{
		float thisValidity = 1.0 - speed * 0.25;
		float4 prevColor = TemporalIn.SampleLevel( LinearSampler, (position + motionVec) * RcpBufferDim, 0 );

	#if 1
		// 2x super sampling with no feedback
		TemporalOut[st] = float4( thisColor, thisValidity );
		thisColor = lerp( thisColor, prevColor.rgb, 0.5 * min(thisValidity, prevColor.a) );
	#else
		// 4x super sampling via controlled feedback
		thisColor = lerp( thisColor, prevColor.rgb, TemporalBlendFactor * min(thisValidity, prevColor.a));
		TemporalOut[st] = float4( thisColor, thisValidity );
	#endif
#endif
	}

	DstColor[st] = thisColor;
}
