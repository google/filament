// RUN: %dxc -E main -O2 -T ps_6_0 %s | FileCheck %s

// CHECK: !"llvm.loop.unroll.count", i32 8

float main(float2 a : A, int3 b : B) : SV_Target
{
  float s = 0;
  [unroll(8)]
  for(int i = 0; i < b.x; i++) {
    [branch]
    if (b.y == 0)
    {
      s += 200;
      break;
    }
    s += a.x;
  }

  return s;
}
