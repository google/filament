// RUN: %dxc -T lib_6_3 -auto-binding-space 11 %s | FileCheck %s

// CHECK: ; S                                 sampler      NA          NA      S0             s1     1
// CHECK: ; T                                 texture     f32          2d      T0             t1     1

// CHECK:@"\01?T{{[@$?.A-Za-z0-9_]+}}" = external constant %"class.Texture2D<vector<float, 4> >", align 4
// CHECK:@"\01?S{{[@$?.A-Za-z0-9_]+}}" = external constant %struct.SamplerState, align 4

// CHECK: define void [[callable1:@"\\01\?callable1@[^\"]+"]](%struct.MyParam* noalias nocapture %param) #0 {
// CHECK:   %[[i_0:[0-9]+]] = load %struct.SamplerState, %struct.SamplerState* @"\01?S{{[@$?.A-Za-z0-9_]+}}", align 4
// CHECK:   %[[i_1:[0-9]+]] = load %"class.Texture2D<vector<float, 4> >", %"class.Texture2D<vector<float, 4> >"* @"\01?T{{[@$?.A-Za-z0-9_]+}}", align 4
// CHECK:   %[[i_3:[0-9]+]] = call %dx.types.Handle @"dx.op.createHandleForLib.class.Texture2D<vector<float, 4> >"(i32 160, %"class.Texture2D<vector<float, 4> >" %[[i_1]])
// CHECK:   %[[i_4:[0-9]+]] = call %dx.types.Handle @dx.op.createHandleForLib.struct.SamplerState(i32 160, %struct.SamplerState %[[i_0]])
// CHECK:   %[[i_7:[0-9]+]] = call %dx.types.ResRet.f32 @dx.op.sampleLevel.f32(i32 62, %dx.types.Handle %[[i_3]], %dx.types.Handle %[[i_4]], float %[[i_5:[0-9]+]], float %[[i_6:[0-9]+]], float undef, float undef, i32 0, i32 0, i32 undef, float 0.000000e+00)
// CHECK:   ret void

struct MyParam {
  float2 coord;
  float4 output;
};

Texture2D T : register(t1);
SamplerState S : register(s1);

[shader("callable")]
void callable1(inout MyParam param)
{
  param.output = T.SampleLevel(S, param.coord, 0);
}
