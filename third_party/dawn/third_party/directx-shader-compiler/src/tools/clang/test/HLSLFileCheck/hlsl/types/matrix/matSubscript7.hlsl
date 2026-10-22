// RUN: %dxc -E main -T vs_6_0 %s | FileCheck %s

// CHECK: @main

float4x4 m;
uint i;
float4 main() : SV_POSITION {
  float4x4 m2 = m;
  return m[2][i] + m2[i][i];
}
