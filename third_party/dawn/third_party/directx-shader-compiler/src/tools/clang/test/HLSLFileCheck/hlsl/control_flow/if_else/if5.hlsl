// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// CHECK: @main
float main(float2 a : A, int3 b : B) : SV_Target
{
  float x;

  [branch]
  if (b.x == 1)
  {
    [branch]
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
