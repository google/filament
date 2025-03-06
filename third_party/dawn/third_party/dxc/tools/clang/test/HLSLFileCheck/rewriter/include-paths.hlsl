// RUN: %dxr -E main -remove-unused-globals -I inc %s | FileCheck %s

#include "include_file.h"

// CHECK-NOT: foo

float4 foo;

// CHECK: float4 main(
float4 main() : SV_Target
{
// CHECK: return 1 + 1;
  return 1 + FOO;
}
