// RUN: %dxc -T lib_6_3 -auto-binding-space 11 -default-linkage external %s | FileCheck %s

// CHECK: call i32 @dx.op.dispatchRaysIndex.i32(i32 145, i8 0)
// CHECK: call i32 @dx.op.dispatchRaysIndex.i32(i32 145, i8 1)

// CHECK: call i32 @dx.op.dispatchRaysDimensions.i32(i32 146, i8 0)
// CHECK: call i32 @dx.op.dispatchRaysDimensions.i32(i32 146, i8 1)

// CHECK: call float @dx.op.worldRayOrigin.f32(i32 147, i8 0)
// CHECK: call float @dx.op.worldRayOrigin.f32(i32 147, i8 1)
// CHECK: call float @dx.op.worldRayOrigin.f32(i32 147, i8 2)

// CHECK: call float @dx.op.worldRayDirection.f32(i32 148, i8 0)
// CHECK: call float @dx.op.worldRayDirection.f32(i32 148, i8 1)
// CHECK: call float @dx.op.worldRayDirection.f32(i32 148, i8 2)

// CHECK: call float @dx.op.objectRayOrigin.f32(i32 149, i8 0)
// CHECK: call float @dx.op.objectRayOrigin.f32(i32 149, i8 1)
// CHECK: call float @dx.op.objectRayOrigin.f32(i32 149, i8 2)

// CHECK: call float @dx.op.objectRayDirection.f32(i32 150, i8 0)
// CHECK: call float @dx.op.objectRayDirection.f32(i32 150, i8 1)
// CHECK: call float @dx.op.objectRayDirection.f32(i32 150, i8 2)

// CHECK: call float @dx.op.rayTMin.f32(i32 153)
// CHECK: call float @dx.op.rayTCurrent.f32(i32 154)
// CHECK: call i32 @dx.op.primitiveIndex.i32(i32 161)
// CHECK: call i32 @dx.op.instanceID.i32(i32 141)
// CHECK: call i32 @dx.op.instanceIndex.i32(i32 142)
// CHECK: call i32 @dx.op.hitKind.i32(i32 143)
// CHECK: call i32 @dx.op.rayFlags.i32(i32 144)

float4 emit(uint shader)  {
  uint2 a = DispatchRaysIndex();
  a += DispatchRaysDimensions();
  float3 b = WorldRayOrigin();
  b += WorldRayDirection();
  b += ObjectRayOrigin();
  b += ObjectRayDirection();

  float4 r = float4(b, a.x+a.y);

  r.w += RayTMin();
  r.w += RayTCurrent();
  r.w += PrimitiveIndex();
  r.w += InstanceID();
  r.w += InstanceIndex();
  r.w += HitKind();
  r.w += RayFlags();

   return r;
}