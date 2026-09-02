// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// Make sure not crash.
// CHECK:error: no member named 'AutoToStat' in 'S'

struct S
{
 float AutoToStatic;
 float b;
};

ConstantBuffer<S> s;

float foo(float a, float b) {
  return a + b;
}

float main() : SV_Target {
  return foo(s.AutoToStatic, s.AutoToStat);
}