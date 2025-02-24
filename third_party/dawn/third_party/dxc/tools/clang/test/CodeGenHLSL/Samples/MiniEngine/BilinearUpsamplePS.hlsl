// RUN: %dxc -E main -T ps_6_0 -O0 -HV 2018 %s | FileCheck %s

// CHECK: sampleLevel
// CHECK: Log
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

#include "ShaderUtility.hlsli"
#include "PresentRS.hlsli"

Texture2D<float3> ColorTex : register(t0);

SamplerState BilinearFilter : register(s0);

[RootSignature(Present_RootSig)]
float3 main( float4 position : SV_Position, float2 uv : TexCoord0 ) : SV_Target0
{
	float3 LinearRGB = LinearizeColor(ColorTex.SampleLevel(BilinearFilter, uv, 0), LDR_COLOR_FORMAT);
	return ApplyColorProfile(LinearRGB, DISPLAY_PLANE_FORMAT);
}
