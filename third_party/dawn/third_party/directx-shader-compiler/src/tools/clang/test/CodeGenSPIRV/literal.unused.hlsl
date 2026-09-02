// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// Unused literals should not be evaluated at 64-bit width.
// Their usage should not result in Int64 or Float64 capabilities.

// CHECK-NOT: OpCapability Int64
// CHECK-NOT: OpCapability Float64

float4 main() : SV_Target {

// CHECK: %int_2 = OpConstant %int 2
  2;
// CHECK: %uint_2 = OpConstant %uint 2
  2u;
// CHECK: %float_0 = OpConstant %float 0
  0.0;

  return 0;
}

