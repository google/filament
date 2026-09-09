// RUN: %dxc -T ps_6_0 -E main -spirv -Gis %s 2>&1 | FileCheck %s

// Make sure the no-contraction is added for the arithmetic operation.
// CHECK: OpDecorate [[op:%[0-9]+]] NoContraction

float4 v;
float4 main(uint col : COLOR) : SV_Target0
{
  // [[op]] = OpVectorTimesScalar
  return 3*v;
}

