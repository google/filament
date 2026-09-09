// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// CHECK: @main
float main(float2 a : A, int3 b : B) : SV_Target
{
  float r;
  switch(b.x)
  {
  case 1:
    r = 5.f;
    if(b.y == 11)
      break;
    r = 5.5f;
    break;
  case 2:
    [flatten]
    if(b.y == 12)
      break;
    r = a.x;
    break;
  default:
    r = 3.f;
    break;
  }
  return r;
}
