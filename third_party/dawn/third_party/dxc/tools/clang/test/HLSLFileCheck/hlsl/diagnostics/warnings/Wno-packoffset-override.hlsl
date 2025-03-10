// RUN: %dxc -T vs_6_0 -Wno-packoffset-override %s | FileCheck %s

// Make sure the specified warning gets turned off

cbuffer CBuf
{
// packoffset is overridden by another packoffset annotation
// CHECK-NOT: overridden
  float4 Element100 : packoffset(c100) : packoffset(c2);
}

// This function has no output semantic on purpose in order to produce an error,
// otherwise, the warnings will not be captured in the output for FileCheck.
float main() {
  return 0;
}

// CHECK: error: Semantic must be defined
