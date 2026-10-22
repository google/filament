// RUN: %dxilver 1.2 | %dxc -E main -T ps_6_2 -enable-16bit-types %s | FileCheck %s

// CHECK: call %dx.types.ResRet.f32 @dx.op.textureLoad.f32
// CHECK: call i1 @dx.op.checkAccessFullyMapped.i32
// CHECK: call %dx.types.ResRet.f16 @dx.op.textureLoad.f16
// CHECK: call i1 @dx.op.checkAccessFullyMapped.i32

RWTexture2D<float4> uav1 : register(u3);
RWTexture1D<half4> uav2 : register(u5);

float4 main(uint2 a : A, uint2 b : B) : SV_Target
{
  float4 r = 0;
  uint status;
  r += uav1[b];
  r += uav1.Load(a);
  uav1.Load(a, status);
  if (CheckAccessFullyMapped(status)) {
    r += 3;
  }

  r += uav2[b.x];
  r += uav2.Load(a);
  uav2.Load(a, status);
  if (CheckAccessFullyMapped(status)) {
    r += 3;
  }

  uav1[b] = r;
  uav2[b.x] = r;
  return r;
}
