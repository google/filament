// RUN: %dxc -E main -T ps_6_0 %s -fcgl | FileCheck %s

// Make sure no bitcast to %struct.A*.
// CHECK-NOT: to %struct.A*

struct A {
   float a;
};

struct B : A {
   float b;
};

A  ga;
float2 ib;

float main() : SV_Target
{
  B b = {ib};
  (A)b = ga;
  return ((A)b).a + b.b;
}
