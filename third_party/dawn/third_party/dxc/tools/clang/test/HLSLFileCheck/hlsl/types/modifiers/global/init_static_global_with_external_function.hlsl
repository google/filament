// RUN: %dxc -E main -T lib_6_3 %s | FileCheck %s
// CHECK:Initializer for static global g makes disallowed call to external function.

float foo();

static float g = foo();

export float bar() {
  return g;
}