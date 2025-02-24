// RUN: %dxc -E main -T ps_6_0 -HV 2018 %s | FileCheck %s
// RUN: %dxc -E main -T ps_6_0 -HV 2018 %s | %D3DReflect %s | FileCheck -check-prefix=REFL %s

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

#include "ShaderUtility.hlsli"
#include "PresentRS.hlsli"

Texture2D<float3> ColorTex : register(t0);
SamplerState PointSampler : register(s1);

cbuffer Constants : register(b0)
{
	float ScaleFactor;
}

[RootSignature(Present_RootSig)]
float3 main( float4 position : SV_Position, float2 uv : TexCoord0 ) : SV_Target0
{
	float2 ScaledUV = ScaleFactor * (uv - 0.5) + 0.5;
	return ColorTex.SampleLevel(PointSampler, ScaledUV, 0);
}

// REFL: InstructionCount: 21
// REFL: TempArrayCount: 0
// REFL: DynamicFlowControlCount: 0
// REFL: ArrayInstructionCount: 0
// REFL: TextureNormalInstructions: 1
// REFL: TextureLoadInstructions: 0
// REFL: TextureCompInstructions: 0
// REFL: TextureBiasInstructions: 0
// REFL: TextureGradientInstructions: 0
// REFL: FloatInstructionCount: 6
// REFL: IntInstructionCount: 0
// REFL: UintInstructionCount: 0
// REFL: CutInstructionCount: 0
// REFL: EmitInstructionCount: 0
// REFL: cBarrierInstructions: 0
// REFL: cInterlockedInstructions: 0
// REFL: cTextureStoreInstructions: 0
