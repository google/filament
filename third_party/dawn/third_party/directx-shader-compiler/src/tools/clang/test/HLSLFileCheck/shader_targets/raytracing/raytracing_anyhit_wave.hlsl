// RUN: %dxc -T lib_6_3 -auto-binding-space 11 %s | FileCheck %s

// CHECK: call i1 @dx.op.waveActiveAllEqual.i32(i32 115,

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
  if (WaveActiveAllEqual(attr.id)) {
    AcceptHitAndEndSearch();
  }
  payload.color += float4(0.125, 0.25, 0.5, 1.0);
}
