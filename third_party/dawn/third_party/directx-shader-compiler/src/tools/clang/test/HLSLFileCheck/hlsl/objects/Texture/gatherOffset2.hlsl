// RUN: %dxc -T ps_6_0 %s | FileCheck %s

// Test for access violation that previously occured with these gathers

// CHECK: @main

Texture2D<float> shadowMap;
SamplerComparisonState BilinearClampCmpSampler;
SamplerState BilinearClampSampler;

float4 main(float3 uv_depth: TEXCOORD0): SV_Target
{
    return shadowMap.GatherCmp(BilinearClampCmpSampler, uv_depth.xy, uv_depth.z, int2(0, 0))
    + shadowMap.GatherRed(BilinearClampSampler, uv_depth.xy, int2(0, 0));
}
