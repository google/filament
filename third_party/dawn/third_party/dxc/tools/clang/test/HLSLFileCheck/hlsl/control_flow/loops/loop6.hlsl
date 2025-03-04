// RUN: %dxc -E main -O2 -T ps_6_0 %s | FileCheck %s

// CHECK: !"llvm.loop.unroll.count", i32 8

float main(float2 a : A, int3 b : B) : SV_Target
{
  float s = 0;
  const uint l = 8;
  if (b.y > 3) {
    while (a.y > 1) {
      [unroll(l)]
      for(int i = 0; i < b.x; i++) {
        s += a.x;
      }
      a.y--;
    }
  }
  return s;
}
