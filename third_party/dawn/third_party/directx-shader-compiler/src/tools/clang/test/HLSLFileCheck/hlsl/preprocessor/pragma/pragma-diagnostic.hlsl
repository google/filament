// RUN: %dxc -T vs_6_0 %s | FileCheck %s

// Verify pragma control for warnings, plus push/pop behavior

float4 foo;

float main() : OUT {
  float4 x = foo;

#pragma dxc diagnostic push
// CHECK-NOT: equality comparison with extraneous parentheses
#pragma dxc diagnostic ignored "-Wparentheses-equality"
  if ((x.y == 0))
  {
// CHECK: error: implicit truncation of vector type
#pragma dxc diagnostic error "-Wconversion"
    return x;
  }
#pragma dxc diagnostic pop

// Verify restoration of diagnostics

// CHECK: warning: equality comparison with extraneous parentheses
  if ((x.z == 0))
  {
// CHECK: warning: implicit truncation of vector type
    return x.yzwx;
  }

}
