// RUN: %dxilver 1.5 | %dxc -E main -T ps_6_0 -DTYPE=float4 %s                            | FileCheck %s -check-prefix=CHK_FLOAT
// RUN: %dxilver 1.5 | %dxc -E main -T ps_6_2 -DTYPE=half4       -enable-16bit-types %s   | FileCheck %s -check-prefix=CHK_HALF
// RUN: %dxilver 1.5 | %dxc -E main -T ps_6_2 -DTYPE=min16float4 -enable-16bit-types %s   | FileCheck %s -check-prefix=CHK_HALF

SamplerState samp1 : register(s5);
Texture2D<TYPE> text1 : register(t3);

float2 ddx;
float2 ddy;

TYPE main(float2 a
          : A) : SV_Target {
  uint status;
  TYPE r = 0;
  // CHK_FLOAT:  call %dx.types.ResRet.f32 @dx.op.sampleGrad.f32(i32 63,
  // CHK_HALF:   call %dx.types.ResRet.f16 @dx.op.sampleGrad.f16(i32 63,
  r += text1.SampleGrad(samp1, a, ddx, ddy);
  // CHK_FLOAT:  call %dx.types.ResRet.f32 @dx.op.sampleGrad.f32(i32 63,
  // CHK_HALF:   call %dx.types.ResRet.f16 @dx.op.sampleGrad.f16(i32 63,
  r += text1.SampleGrad(samp1, a, ddx, ddy, uint2(-5, 7));
  // CHK_FLOAT:  call %dx.types.ResRet.f32 @dx.op.sampleGrad.f32(i32 63,
  // CHK_HALF:   call %dx.types.ResRet.f16 @dx.op.sampleGrad.f16(i32 63,
  r += text1.SampleGrad(samp1, a, ddx, ddy, uint2(-4, 1), 0.5f);
  // CHK_FLOAT:  call %dx.types.ResRet.f32 @dx.op.sampleGrad.f32(i32 63,
  // CHK_HALF:   call %dx.types.ResRet.f16 @dx.op.sampleGrad.f16(i32 63,
  r += text1.SampleGrad(samp1, a, ddx, ddy, uint2(-3, 2), 0.f, status);
  r += status;
  // CHK_FLOAT:  call %dx.types.ResRet.f32 @dx.op.sampleGrad.f32(i32 63,
  // CHK_HALF:   call %dx.types.ResRet.f16 @dx.op.sampleGrad.f16(i32 63,
  r += text1.SampleGrad(samp1, a, ddx, ddy, uint2(-3, 2), a.x, status);
  r += status;
  return r;
}
