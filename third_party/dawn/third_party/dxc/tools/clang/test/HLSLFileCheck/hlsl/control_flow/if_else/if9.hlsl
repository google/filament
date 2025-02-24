// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// CHECK: Not all elements of output SV_Depth were written

float main(float2 a : A, float b : B, out float d : SV_Depth) : SV_Target
{
  [branch]
  if (b != 1)
    return a.x;
  else
    return a.y;
}
