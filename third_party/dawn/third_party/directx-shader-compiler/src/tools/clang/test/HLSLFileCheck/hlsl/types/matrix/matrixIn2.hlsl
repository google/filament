// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// CHECK: %[[v0x:.*]] = call float @dx.op.loadInput.f32(i32 4, i32 0, i32 0, i8 0
// CHECK: %[[v0y:.*]] = call float @dx.op.loadInput.f32(i32 4, i32 0, i32 0, i8 1
// CHECK: %[[v1x:.*]] = call float @dx.op.loadInput.f32(i32 4, i32 0, i32 1, i8 0
// CHECK: %[[v1y:.*]] = call float @dx.op.loadInput.f32(i32 4, i32 0, i32 1, i8 1

// fxc will generate v0.xy v1.xy

// CHECK: call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 0, float %[[v0x]])
// CHECK: call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 1, float %[[v0y]])
// CHECK: call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 2, float %[[v1x]])
// CHECK: call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 3, float %[[v1y]])

float4 main(row_major float2x2 m : M) : SV_Target
{
  return m;
}