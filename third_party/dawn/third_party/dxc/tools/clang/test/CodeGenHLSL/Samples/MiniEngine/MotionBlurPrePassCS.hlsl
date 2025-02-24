// RUN: %dxc -E main -T cs_6_0 %s | FileCheck %s

// CHECK: threadId
// CHECK: Sqrt
// CHECK: Round_ni

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

Texture2D<float3> ColorBuffer : register(t0);
Texture2D<float2> MotionBuffer : register(t1);
RWTexture2D<float4> PrepBuffer : register(u0);

float4 GetSampleData( uint2 st )
{
	return float4(ColorBuffer[st], 1.0) * saturate(length(MotionBuffer[st]) * 32.0 / 4.0);
}

[RootSignature(MotionBlur_RootSig)]
[numthreads( 8, 8, 1 )]
void main( uint3 Gid : SV_GroupID, uint GI : SV_GroupIndex, uint3 GTid : SV_GroupThreadID, uint3 DTid : SV_DispatchThreadID )
{
	uint2 corner = DTid.xy << 1;
	float4 sample0 = GetSampleData( corner + uint2(0, 0) );
	float4 sample1 = GetSampleData( corner + uint2(1, 0) );
	float4 sample2 = GetSampleData( corner + uint2(0, 1) );
	float4 sample3 = GetSampleData( corner + uint2(1, 1) );

	float combinedMotionWeight = sample0.a + sample1.a + sample2.a + sample3.a + 0.0001;
	PrepBuffer[DTid.xy] = floor(0.25 * combinedMotionWeight * 3.0) / 3.0 * float4(
		(sample0.rgb + sample1.rgb + sample2.rgb + sample3.rgb) / combinedMotionWeight, 1.0 );
}