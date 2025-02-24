// RUN: %dxc -T vs_6_0 -Wno-parentheses-equality -Wno-conversion %s | FileCheck %s

// Make sure the specified warnings get turned off

float4 foo;

// This function has no output semantic on purpose in order to produce an error,
// otherwise, the warnings will not be captured in the output for FileCheck.
float main() {
  float4 x = foo;

// CHECK-NOT: equality comparison with extraneous parentheses
  if ((x.y == 0))
  {

// CHECK-NOT: implicit truncation of vector type
    return x;

  }
  return x.y;

}

// CHECK: error: Semantic must be defined
