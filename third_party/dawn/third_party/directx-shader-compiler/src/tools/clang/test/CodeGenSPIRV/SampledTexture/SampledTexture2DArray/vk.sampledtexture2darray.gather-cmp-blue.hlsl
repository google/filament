// RUN: not %dxc -T ps_6_0 -E main -fcgl  %s -spirv  2>&1 | FileCheck %s

vk::SampledTexture2DArray<float4> myTexture : register(t1);

float4 main(float3 location: A, float comparator: B) : SV_Target {
    return myTexture.GatherCmpBlue(location, comparator, int2(1, 2));
}

// CHECK: :6:22: error: no equivalent for GatherCmpBlue intrinsic method in Vulkan
