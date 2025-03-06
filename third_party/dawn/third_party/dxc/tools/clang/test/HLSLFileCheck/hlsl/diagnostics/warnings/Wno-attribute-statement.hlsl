// RUN: %dxc -T vs_6_0 -Wno-attribute-statement %s | FileCheck %s

// Make sure the specified warning gets turned off

// CHECK: 9:1: error: Semantic must be defined

// This function has no output semantic on purpose in order to produce an error,
// otherwise, the warnings will not be captured in the output for FileCheck.
float main() {

// attribute %0 can only be applied to 'if' and 'switch' statements
// CHECK-NOT: statement
  [branch]
  do {} while(true);

  return 0;
}

