// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s
// CHECK: @main

// Confirm that loops with integral constants calculated via intrinsics should be able to be unrolled

[RootSignature("")]
float main(float y : Y) : SV_Target {
  float x = 0;

  [unroll]
  for (uint i = 0; i < max(10u, 5u); ++i)
  {
    x = x * x + y;
  }
  return x;
}
