// RUN: %dxc -T vs_6_0 -Wno-specifier-override %s | FileCheck %s

// Make sure the specified warning gets turned off

struct Struct
{
// %0 will be overridden by %1
// CHECK-NOT: overridden
  sample centroid float attr : ATTR;
};

// This function has no output semantic on purpose in order to produce an error,
// otherwise, the warnings will not be captured in the output for FileCheck.
float main() {
  return 0;
}

// CHECK: error: Semantic must be defined
