// RUN: %dxc -T lib_6_3 -auto-binding-space 11 %s | FileCheck %s

// CHECK: call float @dx.op.waveActiveOp.f32(i32 119, float %{{.*}}, i8 2, i8 0)
// CHECK: call float @dx.op.waveActiveOp.f32(i32 119, float %{{.*}}, i8 2, i8 0)

struct MyPayload {
  float4 color;
  uint2 pos;
};

struct MyParam {
  float2 coord;
  float4 output;
};

[shader("closesthit")]
void closesthit1( inout MyPayload payload : SV_RayPayload,
                  in BuiltInTriangleIntersectionAttributes attr : SV_IntersectionAttributes )
{
  payload.color.xy += attr.barycentrics + WaveActiveMin(attr.barycentrics);
}
