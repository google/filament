// RUN: %dxr -E main -remove-unused-globals %s | FileCheck %s

// CHECK:float foo
// CHECK-NOT:float2 foo
// CHECK:float main
// CHECK:foo(1.2)


float foo(float a) { return a; }

float2 foo(float2 a) { return a; }
float foo(float a);
float2 foo(float2 a);
float main() : SV_Target {
  return foo(1.2);
}