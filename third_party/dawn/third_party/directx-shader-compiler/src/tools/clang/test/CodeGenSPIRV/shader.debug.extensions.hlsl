// RUN: %dxc -T ps_6_0 -E main -fspv-debug=vulkan -fspv-target-env=vulkan1.3 -fcgl  %s -spirv | FileCheck %s

// CHECK-NOT:         OpExtension "SPV_KHR_non_semantic_info"

float4 main(float4 color : COLOR) : SV_TARGET
{
  return color;
}

