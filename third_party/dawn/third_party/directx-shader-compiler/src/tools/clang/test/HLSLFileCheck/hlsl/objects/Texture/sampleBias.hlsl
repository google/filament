// RUN: %dxilver 1.5 | %dxc -E main -T ps_6_0 -DTYPE=float4 %s                            | FileCheck %s -check-prefix=CHK_FLOAT
// RUN: %dxilver 1.5 | %dxc -E main -T ps_6_2 -DTYPE=half4       -enable-16bit-types %s   | FileCheck %s -check-prefix=CHK_HALF
// RUN: %dxilver 1.5 | %dxc -E main -T ps_6_2 -DTYPE=min16float4 -enable-16bit-types %s   | FileCheck %s -check-prefix=CHK_HALF

SamplerState samp1 : register(s5);
Texture2D<TYPE> text1 : register(t3);
float bias;
TYPE main(float2 a
          : A) : SV_Target {
  uint status;
  TYPE r = 0;

  // CHK_FLOAT:  call %dx.types.ResRet.f32 @dx.op.sampleBias.f32(i32 61,
  // CHK_HALF:   call %dx.types.ResRet.f16 @dx.op.sampleBias.f16(i32 61,
  r += text1.SampleBias(samp1, a, -999);
  // CHK_FLOAT:  call %dx.types.ResRet.f32 @dx.op.sampleBias.f32(i32 61,
  // CHK_HALF:   call %dx.types.ResRet.f16 @dx.op.sampleBias.f16(i32 61,
  r += text1.SampleBias(samp1, a, bias, uint2(-5, 7));
  // CHK_FLOAT:  call %dx.types.ResRet.f32 @dx.op.sampleBias.f32(i32 61,
  // CHK_HALF:   call %dx.types.ResRet.f16 @dx.op.sampleBias.f16(i32 61,
  r += text1.SampleBias(samp1, a, bias, uint2(-4, 1), 0.5f);
  // CHK_FLOAT:  call %dx.types.ResRet.f32 @dx.op.sampleBias.f32(i32 61,
  // CHK_HALF:   call %dx.types.ResRet.f16 @dx.op.sampleBias.f16(i32 61,
  r += text1.SampleBias(samp1, a, bias, uint2(-3, 2), 0.f, status);
  // CHK_FLOAT:  call %dx.types.ResRet.f32 @dx.op.sampleBias.f32(i32 61,
  // CHK_HALF:   call %dx.types.ResRet.f16 @dx.op.sampleBias.f16(i32 61,
  r += status;
  r += text1.SampleBias(samp1, a, bias, uint2(-3, 2), a.x, status);
  r += status;
  return r;
}
