// RUN: %dxc -E main -T ps_6_0 %s | %opt -loops -analyze -S | FileCheck %s

// Make sure no nested loop.
// CHECK-NOT: Loop at depth 2
// CHECK: Loop at depth 1

uint c;
Buffer<uint> buf;
Buffer<float2> buf2;
float main(float a:A, float b:B) : SV_Target {
   float r = c;
   uint idx = buf[0];
   while (idx)  {
     idx >>= 1;

     float2 data = buf2[idx];
     if (all(data > 0.5)) {
       r += sin(data.x);
       r *= cos(data.y);
     }
  }

  return r;

}