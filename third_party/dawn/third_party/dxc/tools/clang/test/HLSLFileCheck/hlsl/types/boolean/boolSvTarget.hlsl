// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// CHECK: zext i1
// CHECK: to i32

bool main(float a : A, float b : B) : SV_TARGET
{
  return a == b;
}
