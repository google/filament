// RUN: %dxc -T lib_6_3 -auto-binding-space 11 %s | FileCheck %s

// CHECK: %[[color:[^ ]+]] = getelementptr inbounds %struct.MyPayload, %struct.MyPayload* %{{[^ ,]+}}, i32 0, i32 0
// CHECK: store <4 x float> <float 1.250000e-01, float 2.500000e-01, float 5.000000e-01, float 1.000000e+00>, <4 x float>* %[[color]]
// CHECK: call void @dx.op.ignoreHit
// CHECK: unreachable
// CHECK: %[[pos:[^ ]+]] = getelementptr inbounds %struct.MyPayload, %struct.MyPayload* %{{[^ ,]+}}, i32 0, i32 1
// CHECK: store <2 x i32> <i32 1, i32 2>, <2 x i32>* %[[pos]]
// CHECK: call void @dx.op.ignoreHit
// CHECK: unreachable

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
  payload.color = float4(0.125, 0.25, 0.5, 1.0);
  if (hitLocation.x < 0)
    IgnoreHit();   // aborts function
  payload.pos = uint2(1,2);
  IgnoreHit();   // aborts function
}
