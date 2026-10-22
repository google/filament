// REQUIRES: dxil-1-9
// RUN: %dxc -T ps_6_9 -E main %s | FileCheck %s

#include <vector_utils>

RWByteAddressBuffer buf;

float4 main(uint4 id: IN0) : SV_Target
{
  vector<float, 256> fVec = buf.Load<vector<float, 256> >(0);

  // Ensure calls are lowered to a single shufflevector

  // CHECK: shufflevector <256 x float> %{{.*}}, <256 x float> undef, <4 x i32> <i32 7, i32 8, i32 9, i32 10>
  float4 lhs = hlsl::slice<7, 4>(fVec);

  // CHECK: shufflevector <256 x float> %{{.*}}, <256 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3>
  float4 rhs = hlsl::slice<4>(fVec);

  return lhs + rhs;
}
