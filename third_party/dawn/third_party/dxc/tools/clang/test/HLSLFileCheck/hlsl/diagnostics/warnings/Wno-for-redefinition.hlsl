// RUN: %dxc -T vs_6_0 -Wno-for-redefinition %s | FileCheck %s

// Make sure the specified warning gets turned off

// This function has no output semantic on purpose in order to produce an error,
// otherwise, the warnings will not be captured in the output for FileCheck.
float main() {

// redefinition of %0 shadows declaration in the outer scope; most recent declaration will be used
// CHECK-NOT: redefinition of
  for (int i=0; i<4; i++);
  for (int i=0; i<4; i++);

  return 0;
}

// CHECK: error: Semantic must be defined
