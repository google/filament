// RUN: %dxc -T vs_6_0 %s | FileCheck %s -check-prefix=CHECK -check-prefix=CHK_OPTION
// RUN: %dxc -T vs_6_0 -fdiagnostics-show-option %s | FileCheck %s -check-prefix=CHECK -check-prefix=CHK_OPTION
// RUN: %dxc -T vs_6_0 -fno-diagnostics-show-option %s | FileCheck %s -check-prefix=CHECK -check-prefix=CHK_NO_OPTION

// Make sure the option name for the warning is printed.

float4 foo;

// This function has no output semantic on purpose in order to produce an error,
// otherwise, the warnings will not be captured in the output for FileCheck.
float main() {
  float4 x = foo;

// CHECK: equality comparison with extraneous parentheses
// CHK_OPTION: -Wparentheses-equality
// CHK_NO_OPTION-NOT
  if ((x.y == 0))
  {

// CHECK: implicit truncation of vector type
// CHK_OPTION: -Wconversion
// CHK_NO_OPTION-NOT: -Wconversion
    return x;

  }
  return x.y;

}

// CHECK: error: Semantic must be defined
