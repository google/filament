// RUN: %dxc -T ps_6_0 -E main %s | FileCheck %s

#include <vector_utils>

RWByteAddressBuffer buf;

float4 main(uint4 id: IN0) : SV_Target
{
  vector<float, 4> fVec = buf.Load<vector<float, 4> >(0);

  float2 lhs = hlsl::slice<1, 2>(fVec);
  float2 rhs = hlsl::slice<2>(fVec);

  // CHECK: [[Val:%.*]] = call %dx.types.ResRet.f32 @dx.op.bufferLoad.f32
  // CHECK: [[V0:%.*]] = extractvalue %dx.types.ResRet.f32 [[Val]], 0
  // CHECK: [[V1:%.*]] = extractvalue %dx.types.ResRet.f32 [[Val]], 1
  // CHECK: [[V2:%.*]] = extractvalue %dx.types.ResRet.f32 [[Val]], 2
  // CHECK: call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 0, float [[V1]])
  // CHECK: call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 1, float [[V2]])
  // CHECK: call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 2, float [[V0]])
  // CHECK: call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 3, float [[V1]])

  float4 res = {lhs, rhs};
  return res;
}
