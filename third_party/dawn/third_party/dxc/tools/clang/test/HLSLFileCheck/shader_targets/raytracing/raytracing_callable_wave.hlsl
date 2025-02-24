// RUN: %dxc -T lib_6_3 -auto-binding-space 11 %s | FileCheck %s
// Test failure expected when run with 19041 SDK DXIL.dll

// CHECK:   %[[_7_:[0-9]+]] = call %dx.types.ResRet.f32 @dx.op.sampleLevel.f32(i32 62,
// CHECK:   %[[_8_:[0-9]+]] = extractvalue %dx.types.ResRet.f32 %[[_7_]], 0
// CHECK:   %[[_9_:[0-9]+]] = extractvalue %dx.types.ResRet.f32 %[[_7_]], 1
// CHECK:   %[[_10_:[0-9]+]] = extractvalue %dx.types.ResRet.f32 %[[_7_]], 2
// CHECK:   %[[_11_:[0-9]+]] = extractvalue %dx.types.ResRet.f32 %[[_7_]], 3
// CHECK:   call float @dx.op.wavePrefixOp.f32(i32 121, float %[[_8_]], i8 1, i8 0)
// CHECK:   call float @dx.op.wavePrefixOp.f32(i32 121, float %[[_9_]], i8 1, i8 0)
// CHECK:   call float @dx.op.wavePrefixOp.f32(i32 121, float %[[_10_]], i8 1, i8 0)
// CHECK:   call float @dx.op.wavePrefixOp.f32(i32 121, float %[[_11_]], i8 1, i8 0)

struct MyParam {
  float2 coord;
  float4 output;
};

Texture2D T : register(t1);
SamplerState S : register(s1);

[shader("callable")]
void callable1(inout MyParam param)
{
  param.output = WavePrefixProduct(T.SampleLevel(S, param.coord, 0));
}
