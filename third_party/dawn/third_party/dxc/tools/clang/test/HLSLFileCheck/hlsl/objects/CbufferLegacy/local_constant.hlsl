// RUN: %dxc -T ps_6_0 -E main  -fcgl %s | FileCheck %s

// Make sure global variable is created for local constant.
// CHECK: internal constant [3 x float]
// CHECK-NOT: alloca [3 x float]

float main(uint i : A) : SV_TARGET
{
  const float cb[] = {1.3, 1.2, 3.3};
  return cb[i];
}