// RUN: %dxc -T lib_6_3 -auto-binding-space 11 %s | FileCheck %s

// CHECK: define void [[anyhit1:@"\\01\?anyhit1@[^\"]+"]](%struct.MyPayload* noalias nocapture %payload, %struct.MyAttributes* nocapture readonly %attr) #0 {
// CHECK:   call float @dx.op.objectRayOrigin.f32(i32 149, i8 2)
// CHECK:   call float @dx.op.objectRayDirection.f32(i32 150, i8 2)
// CHECK:   call float @dx.op.rayTCurrent.f32(i32 154)
// CHECK:   call void @dx.op.acceptHitAndEndSearch(i32 156)
// CHECK:   call void @dx.op.ignoreHit(i32 155)
// CHECK:   %[[color:[^ ]+]] = getelementptr inbounds %struct.MyPayload, %struct.MyPayload* %payload, i32 0, i32 0
// CHECK:   store <4 x float> {{.*}}, <4 x float>* %[[color]], align 4
// CHECK:   ret void

struct MyPayload {
  float4 color;
  uint2 pos;
};

struct MyAttributes {
  float2 bary;
  uint id;
};

[shader("anyhit")]
void anyhit1( inout MyPayload payload : SV_RayPayload,
              in MyAttributes attr : SV_IntersectionAttributes )
{
  float3 hitLocation = ObjectRayOrigin() + ObjectRayDirection() * RayTCurrent();
  if (hitLocation.z < attr.bary.x)
    AcceptHitAndEndSearch();         // aborts function
  if (hitLocation.z < attr.bary.y)
    IgnoreHit();   // aborts function
  payload.color += float4(0.125, 0.25, 0.5, 1.0);
}
