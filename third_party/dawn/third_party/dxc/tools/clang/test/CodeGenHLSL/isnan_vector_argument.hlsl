// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// CHECK: call void @dx.op.storeOutput.i32(i32 [[outputSigId:[0-9]+]], i32 0, i32 0, i8 0, i32 0)
// CHECK: call void @dx.op.storeOutput.i32(i32 [[outputSigId]], i32 0, i32 0, i8 1, i32 0)
// CHECK: call void @dx.op.storeOutput.i32(i32 [[outputSigId]], i32 0, i32 0, i8 2, i32 0)

bool3 main() : SV_TARGET
{
  return isnan(float3(0.0f, 0.0f, 0.0f));
}
