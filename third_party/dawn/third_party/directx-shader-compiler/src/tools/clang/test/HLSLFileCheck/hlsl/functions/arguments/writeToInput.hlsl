// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// CHECK-NOT: dx.op.loadInput

float c0;
float c1;

float4 main(float4 a : A) : SV_TARGET {
  a = c0;
  return a+c1;
}
