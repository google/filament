// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// CHECK: attribute 'unroll' must have a uint literal argument
// CHECK-NOT: @main

uint g_cond;

float main() : SV_Target {
  float result = 0;
  [unroll(-1)]
  for (int i = 0; i < g_cond; i++) {
    result += i;
  }

  return 0;
}

