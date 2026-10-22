// RUN: %dxc -E main -T vs_6_0 %s | FileCheck %s

// CHECK: cbufferLoadLegacy
// CHECK: i32 0
// CHECK: cbufferLoadLegacy
// CHECK: i32 1

float4 c0;
float4 c1;

float4 main(float4 a : A, out float4 b:B) : SV_POSITION {
  b = c0;
  return a+c1+b;
}
