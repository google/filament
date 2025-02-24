// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// CHECK: @main
float main(float2 a : A, int3 b : B) : SV_Target
{
  float r;
  switch(b.x)
  {
  case 1:
    r = 5.f;
    switch(b.y)
    {
    case 20:
      r = 5.5f;
      [branch]
      if(b.z == 30)
        break;
      r = 5.7f;
      [flatten]
      if(b.z == 31)
        break;
      r = 5.8f;
      break;
    default:
      r = a.y;
      break;
    }
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
