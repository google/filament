// RUN: %dxr -E main -remove-unused-globals %s | FileCheck %s

// CHECK:foo
// CHECK-NOT:foo(float2
// CHECK:float main
// CHECK:foo(1.2)

float foo(float a) { return a; }

float2 foo(float2 a) { return a; }

float main() : SV_Target {
  return foo(1.2);
}