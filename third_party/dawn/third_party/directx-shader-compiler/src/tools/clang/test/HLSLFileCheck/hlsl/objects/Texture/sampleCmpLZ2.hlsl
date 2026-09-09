// RUN: %dxilver 1.5 | %dxc -E main -T cs_6_0 -DTYPE=float4 %s                            | FileCheck %s -check-prefix=CHECK
// RUN: %dxilver 1.5 | %dxc -E main -T cs_6_2 -DTYPE=half4       -enable-16bit-types %s   | FileCheck %s -check-prefix=CHECK
// RUN: %dxilver 1.5 | %dxc -E main -T cs_6_2 -DTYPE=min16float4 -enable-16bit-types %s   | FileCheck %s -check-prefix=CHECK

SamplerComparisonState samp1 : register(s5);
Texture2D<TYPE> text1 : register(t3);
float cmpVal;
RWBuffer<TYPE> buf1;

[numthreads(8, 8, 8)]
void main(uint dtID : SV_DispatchThreadID)
{
  uint status;
  float2 a = buf1[dtID].xy;
  TYPE r = 0;
  // CHECK:  call %dx.types.ResRet.f32 @dx.op.sampleCmpLevelZero.f32(i32 65,
  r += text1.SampleCmpLevelZero(samp1, a, cmpVal);
  // CHECK:  call %dx.types.ResRet.f32 @dx.op.sampleCmpLevelZero.f32(i32 65,
  r += text1.SampleCmpLevelZero(samp1, a, cmpVal, uint2(-5, 7));
  // CHECK:  call %dx.types.ResRet.f32 @dx.op.sampleCmpLevelZero.f32(i32 65,
  r += text1.SampleCmpLevelZero(samp1, a, cmpVal, uint2(-3, 2), status);
  r += status;
  buf1[dtID] = r;
}
