// RUN: %dxc -T lib_6_3 -auto-binding-space 11 %s | FileCheck %s

// CHECK:   call float @dx.op.wavePrefixOp.f32(i32 121, float %{{.*}}, i8 1, i8 0)
// CHECK:   call float @dx.op.wavePrefixOp.f32(i32 121, float %{{.*}}, i8 1, i8 0)
// CHECK:   call float @dx.op.wavePrefixOp.f32(i32 121, float %{{.*}}, i8 1, i8 0)
// CHECK:   call float @dx.op.wavePrefixOp.f32(i32 121, float %{{.*}}, i8 1, i8 0)

struct MyPayload {
  float4 color;
  uint2 pos;
};

[shader("miss")]
void miss1(inout MyPayload payload : SV_RayPayload)
{
  payload.color = WavePrefixProduct(payload.color);
}
