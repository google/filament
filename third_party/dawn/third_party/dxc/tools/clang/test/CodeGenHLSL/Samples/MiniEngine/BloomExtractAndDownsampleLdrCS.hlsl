// RUN: %dxc -E main -T cs_6_0 -HV 2018 %s | FileCheck %s

// CHECK: threadId
// CHECK: dot3
// CHECK: FMax

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
// The CS for extracting bright pixels and downsampling them to an unblurred bloom buffer.

#include "ShaderUtility.hlsli"
#include "PostEffectsRS.hlsli"

SamplerState BiLinearClamp : register( s0 );
Texture2D<float3> SourceTex : register( t0 );
RWTexture2D<float3> BloomResult : register( u0 );

cbuffer cb0
{
	float2 g_inverseOutputSize;
	float g_bloomThreshold;
}

[RootSignature(PostEffects_RootSig)]
[numthreads( 8, 8, 1 )]
void main( uint3 DTid : SV_DispatchThreadID )
{
	// We need the scale factor and the size of one pixel so that our four samples are right in the middle
	// of the quadrant they are covering.
	float2 uv = (DTid.xy + 0.5) * g_inverseOutputSize;
	float2 offset = g_inverseOutputSize * 0.25;

	// Use 4 bilinear samples to guarantee we don't undersample when downsizing by more than 2x
	float3 color1 = SourceTex.SampleLevel( BiLinearClamp, uv + float2(-offset.x, -offset.y), 0 );
	float3 color2 = SourceTex.SampleLevel( BiLinearClamp, uv + float2( offset.x, -offset.y), 0 );
	float3 color3 = SourceTex.SampleLevel( BiLinearClamp, uv + float2(-offset.x,  offset.y), 0 );
	float3 color4 = SourceTex.SampleLevel( BiLinearClamp, uv + float2( offset.x,  offset.y), 0 );

	float luma1 = RGBToLuminance(color1);
	float luma2 = RGBToLuminance(color2);
	float luma3 = RGBToLuminance(color3);
	float luma4 = RGBToLuminance(color4);

	const float kSmallEpsilon = 0.0001;

	float ScaledThreshold = g_bloomThreshold;

	// We perform a brightness filter pass, where lone bright pixels will contribute less.
	color1 *= max(kSmallEpsilon, luma1 - ScaledThreshold) / (luma1 + kSmallEpsilon);
	color2 *= max(kSmallEpsilon, luma2 - ScaledThreshold) / (luma2 + kSmallEpsilon);
	color3 *= max(kSmallEpsilon, luma3 - ScaledThreshold) / (luma3 + kSmallEpsilon);
	color4 *= max(kSmallEpsilon, luma4 - ScaledThreshold) / (luma4 + kSmallEpsilon);

	// The shimmer filter helps remove stray bright pixels from the bloom buffer by inversely weighting
	// them by their luminance.  The overall effect is to shrink bright pixel regions around the border.
	// Lone pixels are likely to dissolve completely.  This effect can be tuned by adjusting the shimmer
	// filter inverse strength.  The bigger it is, the less a pixel's luminance will matter.
	const float kShimmerFilterInverseStrength = 1.0f;
	float weight1 = 1.0f / (luma1 + kShimmerFilterInverseStrength);
	float weight2 = 1.0f / (luma2 + kShimmerFilterInverseStrength);
	float weight3 = 1.0f / (luma3 + kShimmerFilterInverseStrength);
	float weight4 = 1.0f / (luma4 + kShimmerFilterInverseStrength);
	float weightSum = weight1 + weight2 + weight3 + weight4;

	BloomResult[DTid.xy] = (color1 * weight1 + color2 * weight2 + color3 * weight3 + color4 * weight4) / weightSum;
}
