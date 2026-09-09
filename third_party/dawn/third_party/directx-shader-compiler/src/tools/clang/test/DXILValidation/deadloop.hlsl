// RUN: %dxc -E main -T ps_6_0 %s  | FileCheck %s

// CHECK: !"llvm.loop.unroll.disable"

int i;

float main(float2 a : A, int3 b : B) : SV_Target
{
  float s = 0;
  
  while(i < b.x) {
    s += a.x;
  }

  return s;
}
