// RUN: %dxc -T lib_6_3 -auto-binding-space 11 -default-linkage external %s | FileCheck %s

// CHECK: call float @dx.op.objectToWorld.f32(i32 151, i32 2, i8 0)
// CHECK: call float @dx.op.objectToWorld.f32(i32 151, i32 2, i8 1)
// CHECK: call float @dx.op.worldToObject.f32(i32 152, i32 2, i8 2)
// CHECK: call float @dx.op.worldToObject.f32(i32 152, i32 2, i8 3)

float4 emit(uint shader)  {
  float3x4 o2w = ObjectToWorld3x4();
  float3x4 w2o = WorldToObject3x4();

  return float4(o2w._m20_m21, w2o._33_34);
}