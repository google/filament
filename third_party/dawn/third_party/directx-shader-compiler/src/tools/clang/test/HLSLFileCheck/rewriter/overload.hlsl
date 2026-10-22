// RUN: %dxr -E main -remove-unused-globals %s | FileCheck %s

// CHECK-NOT:foo
// CHECK:float main

float foo(float a) { return a; }

float2 foo(float2 a) { return a; }

float foo(float a);

float a;

float main() : SV_Target {
  return 1;
}