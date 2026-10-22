// RUN: not %dxc -T ps_6_0 -E main -fcgl  %s -spirv  2>&1 | FileCheck %s

// CHECK: 5:19: error: SV_InnerCoverage must be of uint type.

float4 main(float inCov : SV_InnerCoverage) : SV_Target {
  return inCov;
}

