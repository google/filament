// RUN: not %dxc -T ps_6_1 -E main -fspv-target-env=vulkan1.1 -fspv-debug=t -fcgl  %s -spirv 2>&1 | FileCheck %s

float4 main(uint val : A) : SV_Target {
  uint a = reversebits(val);
  return a;
}

// CHECK: unknown SPIR-V debug info control parameter: t
