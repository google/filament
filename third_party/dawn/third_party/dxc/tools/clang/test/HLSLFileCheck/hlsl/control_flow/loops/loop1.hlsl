// RUN: %dxc -E main -T ps_6_0 %s  | FileCheck %s

// CHECK: !"llvm.loop.unroll.disable"

float main(float2 a : A, int3 b : B) : SV_Target
{
  float s = 0;
  [loop]
  for(int i = 0; i < b.x; i++) {
    s += a.x;
  }

  return s;
}
