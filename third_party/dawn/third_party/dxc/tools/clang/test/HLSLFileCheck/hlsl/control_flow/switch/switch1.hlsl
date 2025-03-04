// RUN: %dxc -E main -T ps_6_0 %s  | FileCheck %s

// CHECK: !"dx.controlflow.hints", i32 5, i32 2

float main(float2 a : A, int3 b : B) : SV_Target
{
  float r;
  [flatten]
  [forcecase]
  switch(b.x)
  {
  case 1:
    r = 5.f;
    break;
  case 2:
    r = a.x;
    break;
  default:
    r = 3.f;
    break;
  }
  return r;
}
