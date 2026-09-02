// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// CHECK-DAG: Could not unroll loop.
// CHECK-NOT: @main
uint g_cond;
float main() : SV_Target {

  float result = 0;
  [unroll(0)]
  for (int i = 0; i < g_cond; i++) {
    result += i;
  }
  return result;
}

