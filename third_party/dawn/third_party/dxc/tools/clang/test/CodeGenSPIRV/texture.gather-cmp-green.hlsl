// RUN: not %dxc -T ps_6_0 -E main -fcgl  %s -spirv  2>&1 | FileCheck %s

SamplerComparisonState gSampler : register(s5);

Texture2D<float4> myTexture : register(t1);

float4 main(float2 location: A, float comparator: B) : SV_Target {
    return myTexture.GatherCmpGreen(gSampler, location, comparator, int2(1, 2));
}

// CHECK: :8:22: error: no equivalent for GatherCmpGreen intrinsic method in Vulkan
