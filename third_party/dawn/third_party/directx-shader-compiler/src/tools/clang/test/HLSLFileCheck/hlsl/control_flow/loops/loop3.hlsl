// RUN: %dxc -E main -O2 -T ps_6_0 %s | FileCheck %s

// CHECK: !"llvm.loop.unroll.disable"

float main(float2 a : A, int3 b : B) : SV_Target
{
  float s = 0;
  [loop]
  for(int i = 0; i < b.x; i++) {
    [branch]
    if (b.z == 9)
      break;
    [allow_uav_condition]
    for(int j = 0; j <= 16; j++)
    {
      [branch]
      if (b.z == 16)
        break;

      s += a.x;
    }
  }

  return s;
}
