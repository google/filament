// RUN: %dxc -E main -T cs_6_0 -HV 2018 %s | FileCheck %s

// CHECK: threadId
// CHECK: dot3
// CHECK: Exp

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

#include "ShaderUtility.hlsli"
#include "PostEffectsRS.hlsli"

Texture2D<float3> SrcColor : register( t0 );
Texture2D<float3> Bloom : register( t1 );
RWTexture2D<float3> DstColor : register( u0 );
RWTexture2D<float> OutLuma : register( u1 );
SamplerState LinearSampler : register( s0 );

cbuffer ConstantBuffer_x : register( b0 )
{
	float2 g_RcpBufferDim;
	float g_BloomStrength;
	float g_LumaGamma;
};

[RootSignature(PostEffects_RootSig)]
[numthreads( 8, 8, 1 )]
void main( uint3 Gid : SV_GroupID, uint GI : SV_GroupIndex, uint3 GTid : SV_GroupThreadID, uint3 DTid : SV_DispatchThreadID )
{
	float2 TexCoord = (DTid.xy + 0.5) * g_RcpBufferDim;

	// Load LDR and bloom
	float3 ldrColor = SrcColor[DTid.xy] + g_BloomStrength * Bloom.SampleLevel( LinearSampler, TexCoord, 0 );

	DstColor[DTid.xy] = ldrColor;
	OutLuma[DTid.xy] = RGBToLogLuminance( ldrColor, g_LumaGamma );
}
