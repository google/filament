// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// CHECK: fcmp fast une
// CHECK: !"dx.controlflow.hints", i32 1

float main(float2 a : A, float b : B) : SV_Target
{
  [branch]
  if (b != 1)
    return a.x;
  else
    return a.y;
}
