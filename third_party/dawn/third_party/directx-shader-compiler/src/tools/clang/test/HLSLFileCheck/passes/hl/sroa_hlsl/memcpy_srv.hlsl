// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// Make sure no alloca left.
//CHECK-NOT: alloca
//CHECK:bufferLoad
struct S {
   float f[16];
};

StructuredBuffer<S> b;

float main(int2 a:A) : SV_Target {

    S s = b[a.x];
  return s.f[a.y];
}