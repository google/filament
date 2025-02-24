// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// CHECK: sample


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

#include "ModelViewerRS.hlsli"

struct VSOutput
{
	float4 pos : SV_Position;
	float2 uv : TexCoord0;
};

Texture2D<float4> texDiffuse : register(t0);
SamplerState sampler0 : register(s0);

[RootSignature(ModelViewer_RootSig)]
void main(VSOutput vsOutput)
{
	if (texDiffuse.Sample(sampler0, vsOutput.uv).a < 0.5)
		discard;
}
