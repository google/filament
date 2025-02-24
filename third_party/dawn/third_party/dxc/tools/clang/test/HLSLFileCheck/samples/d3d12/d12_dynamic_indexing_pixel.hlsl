// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// CHECK: unbounded
// CHECK: cbufferLoad
// CHECK: createHandle
// CHECK: sample
// CHECK: [0 x %"class.Texture2D<vector<float, 4> >"]

//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************

struct PSInput
{
	float4 position	: SV_POSITION;
	float2 uv		: TEXCOORD0;
};

struct MaterialConstants
{
	uint matIndex;	// Dynamically set index for looking up from g_txMats[].
};

ConstantBuffer<MaterialConstants> materialConstants : register(b0, space0);
Texture2D		g_txDiffuse	: register(t0);
Texture2D		g_txMats[]	: register(t1);
SamplerState	g_sampler	: register(s0);

float4 main(PSInput input) : SV_TARGET
{
	float3 diffuse = g_txDiffuse.Sample(g_sampler, input.uv).rgb;
	float3 mat = g_txMats[materialConstants.matIndex].Sample(g_sampler, input.uv).rgb;
	return float4(diffuse * mat, 1.0f);
}