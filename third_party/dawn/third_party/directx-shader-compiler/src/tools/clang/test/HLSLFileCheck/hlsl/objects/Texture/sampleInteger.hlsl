// RUN: %dxc -T ps_6_7 %s -DSKIP_SAMPLE_CMP -DTYPE=uint | FileCheck %s
// RUN: %dxc -T ps_6_7 %s -DSKIP_SAMPLE_CMP -DTYPE=int | FileCheck %s
// RUN: %dxc -T ps_6_6 %s -DTYPE=uint | FileCheck %s -check-prefix=CHKFAIL
// RUN: %dxc -T ps_6_7 %s -DSKIP_SAMPLE_CMP -DTYPE=uint16_t -enable-16bit-types | FileCheck %s -check-prefix=CHECK16
// RUN: %dxc -T ps_6_7 %s -DSKIP_SAMPLE_CMP -DTYPE=int16_t -enable-16bit-types | FileCheck %s -check-prefix=CHECK16
// RUN: %dxc -T ps_6_6 %s -DTYPE=uint16_t -enable-16bit-types | FileCheck %s -check-prefix=CHKFAIL

SamplerState g_samp : register(s0);
SamplerComparisonState g_sampCmp : register(s01);
Texture2D<TYPE> g_tex0 : register(t0);

// CHECK: call %dx.types.ResRet.i32 @dx.op.sample.i32(i32 60
// CHECK: call %dx.types.ResRet.i32 @dx.op.sampleBias.i32(i32 61
// CHECK: call %dx.types.ResRet.i32 @dx.op.sampleLevel.i32(i32 62
// CHECK: call %dx.types.ResRet.i32 @dx.op.sampleGrad.i32(i32 63

// CHECK16: call %dx.types.ResRet.i16 @dx.op.sample.i16(i32 60
// CHECK16: call %dx.types.ResRet.i16 @dx.op.sampleBias.i16(i32 61
// CHECK16: call %dx.types.ResRet.i16 @dx.op.sampleLevel.i16(i32 62
// CHECK16: call %dx.types.ResRet.i16 @dx.op.sampleGrad.i16(i32 63

// CHKFAIL: error: cannot Sample from resource containing
// CHKFAIL: error: cannot SampleBias from resource containing
// CHKFAIL: error: cannot SampleLevel from resource containing
// CHKFAIL: error: cannot SampleGrad from resource containing
// CHKFAIL: error: cannot SampleCmp from resource containing
// CHKFAIL: error: cannot SampleCmpLevelZero from resource containing
// CHKFAIL: error: cannot SampleCmpLevel from resource containing

float4 main(float2 coord : TEXCOORD0) : SV_Target {
  float res = 0;     
  res = g_tex0.Sample(g_samp, coord);
  res += g_tex0.SampleBias(g_samp, coord, 0.0);
  res += g_tex0.SampleLevel(g_samp, coord, 0.0);
  res += g_tex0.SampleGrad(g_samp, coord, 0.0, 0.0);
#ifndef SKIP_SAMPLE_CMP
  res += g_tex0.SampleCmp(g_sampCmp, coord, 0.0);
  res += g_tex0.SampleCmpLevelZero(g_sampCmp, coord, 0.0);
  res += g_tex0.SampleCmpLevel(g_sampCmp, coord, 0.0, 0.0);
#endif
  return res;
}
