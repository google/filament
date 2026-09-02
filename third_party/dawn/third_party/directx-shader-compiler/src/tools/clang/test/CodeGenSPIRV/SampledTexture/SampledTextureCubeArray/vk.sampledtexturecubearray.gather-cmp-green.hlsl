// RUN: not %dxc -T ps_6_8 -E main -fcgl %s -spirv 2>&1 | FileCheck %s

vk::SampledTextureCUBEArray<float4> tex;

float4 main() : SV_Target {
  float4 a = tex.GatherCmpGreen(float4(0.5, 0.25, 0.75, 1.0), 0.25f);
  uint status;
  float4 b = tex.GatherCmpGreen(float4(0.5, 0.25, 0.75, 1.0), 0.25f, status);
  return a + b;
}

// CHECK: error: no equivalent for GatherCmpGreen intrinsic method in Vulkan
// CHECK: error: no equivalent for GatherCmpGreen intrinsic method in Vulkan
