// RUN: %dxc -T vs_6_0 -Wno-comma-in-init %s | FileCheck %s

// Make sure the specified warning gets turned off

// This function has no output semantic on purpose in order to produce an error,
// otherwise, the warnings will not be captured in the output for FileCheck.
float main() {

// comma expression used where a constructor list may have been intended
// CHECK-NOT: comma expression
  int a = 1, b = 2;
  int c = (a, b);

  return 0;
}

// CHECK: error: Semantic must be defined
