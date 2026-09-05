// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// CHECK: sample
// CHECK: sample
// CHECK: sample
// CHECK: storeOutput

//--------------------------------------------------------------------------------------
// File: PSApproach.hlsl
//
// The PSs for doing post-processing, used in PS path of 
// HDRToneMappingCS11 sample
// 
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------
static const float4 LUM_VECTOR = float4(.299, .587, .114, 0);
static const float  MIDDLE_GRAY = 0.72f;
static const float  LUM_WHITE = 1.5f;
static const float  BRIGHT_THRESHOLD = 0.5f;

SamplerState PointSampler : register (s0);
SamplerState LinearSampler : register (s1);

struct QuadVS_Output
{
    float4 Pos : SV_POSITION;              
    float2 Tex : TEXCOORD0;
};

Texture2D s0 : register(t0);
Texture2D s1 : register(t1);
Texture2D s2 : register(t2);


float4 main( QuadVS_Output Input ) : SV_TARGET
{   
    //float4 vColor = 0;
    float4 vColor = s0.Sample( PointSampler, Input.Tex );
    float4 vLum = s1.Sample( PointSampler, float2(0,0) );
    float3 vBloom = s2.Sample( LinearSampler, Input.Tex );       
    
    // Tone mapping
    vColor.rgb *= MIDDLE_GRAY / (vLum.r + 0.001f);
    vColor.rgb *= (1.0f + vColor/LUM_WHITE);
    vColor.rgb /= (1.0f + vColor);
    
    vColor.rgb += 0.6f * vBloom;
    vColor.a = 1.0f;    
    
    return vColor;
}
