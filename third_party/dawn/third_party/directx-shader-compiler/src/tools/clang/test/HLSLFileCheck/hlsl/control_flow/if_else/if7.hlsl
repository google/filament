// RUN: %dxc -E main -T ps_6_0 /Gfp %s | FileCheck %s

// CHECK: !"dx.controlflow.hints", i32 1
// CHECK: !"dx.controlflow.hints", i32 1

float main(float2 a : A, int3 b : B) : SV_Target
{
  float x=0;

  if (b.x == 1)
  {
    if (b.y)
      x = a.x + 5.;
    else
      x = a.x - 2.;
  }
  else
  {
    [branch]
    if (b.z)
      x = a.y - 77.;
  }

  return x;
}
