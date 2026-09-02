// RUN: %dxc -T cs_6_0 -Wno-attribute-type %s | FileCheck %s
// Make sure the specified warning gets turned off

// Note that This errors out anyway since the resulting numthreads is invalid.

// attribute %0 must have a uint literal argument
// CHECK-NOT: uint literal argument
[numthreads(1.0f, 0, 0)]
void main() {
  return;
}

// CHECK: warning: Group size of 0
// CHECK: error: compute entry point must have a valid numthreads attribute
