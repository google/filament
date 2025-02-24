// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// CHECK-NOT: 64-Bit integer


uint a;

float4 main() :SV_TARGET {

  return 1 << a;
}