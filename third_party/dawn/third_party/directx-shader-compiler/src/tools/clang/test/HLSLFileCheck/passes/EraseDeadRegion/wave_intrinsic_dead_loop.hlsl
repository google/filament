// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// CHECK:@main
// Make sure no cond branch which is for loop.
// CHECK-NOT:br i1

float main(uint2 i:I) : SV_Target {

  int a = WaveReadLaneFirst(i.x);
  float r = a;
  for (;;) {

    uint b = WaveReadLaneFirst(i.y);

    if (a == b)
      break;



  }
  return r;
}