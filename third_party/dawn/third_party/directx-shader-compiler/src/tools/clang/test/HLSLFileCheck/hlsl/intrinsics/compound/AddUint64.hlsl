// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// CHECK: binaryWithCarry
// CHECK: binaryWithCarry

float4 main(uint4 a : A, uint4 b :B) : SV_TARGET {
  uint2 c2 = AddUint64(a.xy, b.xy);
  uint4 c4 = AddUint64(a, b);
  return c2.xxyy + c4;
}
