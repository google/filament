// RUN: %dxc -E main -T ps_6_0 -HV 2018 %s | FileCheck %s

// CHECK: Frc
// CHECK: IMax
// CHECK: UMin

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

//--------------------------------------------------------------------------------------
// Simple bicubic filter
//
// http://en.wikipedia.org/wiki/Bicubic_interpolation
// http://http.developer.nvidia.com/GPUGems/gpugems_ch24.html
//
//--------------------------------------------------------------------------------------

#include "ShaderUtility.hlsli"
#include "PresentRS.hlsli"

Texture2D<float3> ColorTex : register(t0);

cbuffer Constants : register(b0)
{
	uint2 TextureSize;
	float A;
}

float W1(float x)
{
	return x * x * ((A + 2) * x - (A + 3)) + 1.0;
}

float W2(float x)
{
	return A * (x * (x * (x - 5) + 8) - 4);
}

float4 GetWeights(float d1)
{
	return float4(W2(1.0 + d1), W1(d1), W1(1.0 - d1), W2(2.0 - d1));
}

float3 GetColor(uint s, uint t)
{
#ifdef GAMMA_SPACE
	return ApplyColorProfile(ColorTex[uint2(s, t)], DISPLAY_PLANE_FORMAT);
#else
	return ColorTex[uint2(s, t)];
#endif
}

[RootSignature(Present_RootSig)]
float3 main(float4 position : SV_Position, float2 uv : TexCoord0) : SV_Target0
{
	float2 t = uv * TextureSize + 0.5;
	float2 f = frac(t);
	int2 st = int2(t.x, position.y);

	uint MaxWidth = TextureSize.x - 1;

	uint s0 = max(st.x - 2, 0);
	uint s1 = max(st.x - 1, 0);
	uint s2 = min(st.x + 0, MaxWidth);
	uint s3 = min(st.x + 1, MaxWidth);

	float4 W = GetWeights(f.x);
	float3 Color = 
		W.x * GetColor(s0, st.y) + 
		W.y * GetColor(s1, st.y) + 
		W.z * GetColor(s2, st.y) +
		W.w * GetColor(s3, st.y);

	return Color;
}
