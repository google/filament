// RUN: %dxilver 1.5 | %dxc -E main -T ps_6_0 -DTYPE=float4 %s                            | FileCheck %s -check-prefix=CHK_FLOAT
// RUN: %dxilver 1.5 | %dxc -E main -T ps_6_2 -DTYPE=half4       -enable-16bit-types %s   | FileCheck %s -check-prefix=CHK_HALF
// RUN: %dxilver 1.5 | %dxc -E main -T ps_6_2 -DTYPE=min16float4 -enable-16bit-types %s   | FileCheck %s -check-prefix=CHK_HALF

SamplerState samp1 : register(s5);
Texture2D<TYPE> text1 : register(t3);
TextureCubeArray<TYPE> text2 : register(t5);

int LOD;
TYPE main(float2 a
            : A) : SV_Target {
  uint status;
  TYPE r = 0;

  // CHK_FLOAT:  call %dx.types.ResRet.f32 @dx.op.sampleLevel.f32(i32 62,
  // CHK_HALF:   call %dx.types.ResRet.f16 @dx.op.sampleLevel.f16(i32 62,
  r += text1.SampleLevel(samp1, a, LOD);
  // CHK_FLOAT:  call %dx.types.ResRet.f32 @dx.op.sampleLevel.f32(i32 62,
  // CHK_HALF:   call %dx.types.ResRet.f16 @dx.op.sampleLevel.f16(i32 62,
  r += text1.SampleLevel(samp1, a, LOD, uint2(-5, 7));
  // CHK_FLOAT:  call %dx.types.ResRet.f32 @dx.op.sampleLevel.f32(i32 62,
  // CHK_HALF:   call %dx.types.ResRet.f16 @dx.op.sampleLevel.f16(i32 62,
  // CHK_FLOAT:  call i1 @dx.op.checkAccessFullyMapped.i32(i32 71,
  // CHK_HALF:   call i1 @dx.op.checkAccessFullyMapped.i32(i32 71,
  r += text1.SampleLevel(samp1, a, LOD, uint2(-3, 2), status);
  r += CheckAccessFullyMapped(status);
  // CHK_FLOAT:  call %dx.types.ResRet.f32 @dx.op.sampleLevel.f32(i32 62,
  // CHK_HALF:   call %dx.types.ResRet.f16 @dx.op.sampleLevel.f16(i32 62,
  r += text2.SampleLevel(samp1, a.xyxy, LOD);
  // CHK_FLOAT:  call %dx.types.ResRet.f32 @dx.op.sampleLevel.f32(i32 62,
  // CHK_HALF:   call %dx.types.ResRet.f16 @dx.op.sampleLevel.f16(i32 62,
  // CHK_FLOAT:  call i1 @dx.op.checkAccessFullyMapped.i32(i32 71,
  // CHK_HALF:   call i1 @dx.op.checkAccessFullyMapped.i32(i32 71,
  r += text2.SampleLevel(samp1, a.xyxy, LOD * 0.5, status);
  r += CheckAccessFullyMapped(status);

  return r;
}
