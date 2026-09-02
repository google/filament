// RUN: %dxc -T lib_6_3 -auto-binding-space 11 -default-linkage external %s | FileCheck %s

// CHECK: call float @dx.op.objectToWorld.f32(i32 151, i32 2, i8 0)
// CHECK: call float @dx.op.objectToWorld.f32(i32 151, i32 2, i8 1)
// CHECK: call float @dx.op.worldToObject.f32(i32 152, i32 2, i8 2)
// CHECK: call float @dx.op.worldToObject.f32(i32 152, i32 2, i8 3)

float4 emit(uint shader)  {
  float4x3 o2w = ObjectToWorld4x3();
  float4x3 w2o = WorldToObject4x3();

  return float4(o2w._m02_m12, w2o._33_43);
}