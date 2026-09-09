// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// CHECK: @main
float main(float2 a : A, int3 b : B) : SV_Target
{
  float s = 0;
  [loop]
  for(int i = 0; i < b.x; i++) {
    [flatten]
    if (b.z == 5)
      return s;

    [branch]
    if (b.z == 7)
      return s;

    s += a.x;
  }

  return s;
}
