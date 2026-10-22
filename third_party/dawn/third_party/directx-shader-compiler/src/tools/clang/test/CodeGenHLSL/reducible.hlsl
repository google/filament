// RUN: %dxc -E main -T ps_6_0 %s  | FileCheck %s

// CHECK: !"llvm.loop.unroll.disable"
uint u;
float main(float a : A, int3 b : B) : SV_Target
{
  float s = 0;
  /*
  [loop]
  for(int i = 0; i < b.x; i++) {
    s += a.x;
    if (s == 5)
      break;
  }
  */
  if (s > a)
    s -= u+b.x;
  else
    s += b.x+b.y;

  return s;
}
