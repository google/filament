// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// CHECK: call void @dx.op.storeOutput.f32(i32 [[outputSigId:[0-9]+]], i32 0, i32 0, i8 0, float 0.0
// CHECK: call void @dx.op.storeOutput.f32(i32 [[outputSigId]], i32 0, i32 0, i8 1, float 0.0
// CHECK: call void @dx.op.storeOutput.f32(i32 [[outputSigId]], i32 0, i32 0, i8 2, float 0.0

float3 main() : SV_TARGET
{
  return atan2(float3(0.0f, 0.0f, 0.0f), float3(0.0f, 0.0f, 0.0f));
}
