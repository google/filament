// RUN: %dxc -Od -E main -T ps_6_0 %s | FileCheck %s
// CHECK-DAG: Could not unroll loop.
// CHECK-DAG: -HV 2016
// CHECK-NOT: @main

// Check that the compilation fails due to unable to
// find the loop bound.

uint g_cond;

float main() : SV_Target {
  float result = 0;
  [unroll]
  for (uint j = 0; j < g_cond; j++) {
    result += 1;
  }
  return result;
}

