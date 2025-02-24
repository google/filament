// RUN: not %dxc -T ps_6_0 -E main -fcgl -spirv %s -spirv 2>&1 | FileCheck %s

// CHECK: error: alignment argument must be a constant unsigned integer

uint64_t Address;
float4 main() : SV_Target0 {
  float4 x = vk::RawBufferLoad<float4>(Address, -16);

  return x;
}
