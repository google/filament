// RUN: %dxc -T ps_6_0 -E main -fcgl -Vd %s -spirv | FileCheck %s

// globalS[0][0].t should take binding #0.
// globalS[0][0].s should take binding #1.
// globalS[0][1].t should take binding #2.
// globalS[0][1].s should take binding #3.
// globalS[0][2].t should take binding #4.
// globalS[0][2].s should take binding #5.
// globalS[1][0].t should take binding #6.
// globalS[1][0].s should take binding #7.
// globalS[1][1].t should take binding #8.
// globalS[1][1].s should take binding #9.
// globalS[1][2].t should take binding #10.
// globalS[1][2].s should take binding #11.
//
// CHECK: OpDecorate %globalS DescriptorSet 0
// CHECK: OpDecorate %globalS Binding 0
//
// CHECK: OpDecorate %globalTexture DescriptorSet 0
// CHECK: OpDecorate %globalTexture Binding 12
//
// CHECK: OpDecorate %globalSamplerState DescriptorSet 0
// CHECK: OpDecorate %globalSamplerState Binding 13


// CHECK:                                              %S = OpTypeStruct %type_2d_image %type_sampler
// CHECL:                                  %_arr_S_uint_3 = OpTypeArray %S %uint_3
// CHECL:                      %_arr__arr_S_uint_3_uint_2 = OpTypeArray %_arr_S_uint_3 %uint_2
// CHECL: %_ptr_UniformConstant__arr__arr_S_uint_3_uint_2 = OpTypePointer UniformConstant %_arr__arr_S_uint_3_uint_2
// CHECK-NOT:                              %type__Globals =

struct S {
  Texture2D t;
  SamplerState s;
};

float4 tex2D(S x, float2 v) { return x.t.Sample(x.s, v); }

// CHECK:      %globalS = OpVariable %_ptr_UniformConstant__arr__arr_S_uint_3_uint_2 UniformConstant
// CHECK-NOT: %_Globals = OpVariable
S globalS[2][3];


Texture2D globalTexture;
SamplerState globalSamplerState;

float4 main() : SV_Target {
// CHECK:  [[base:%[0-9]+]] = OpAccessChain %_ptr_UniformConstant__arr_S_uint_3 %globalS %int_0
// CHECK:  [[ptr:%[0-9]+]] = OpAccessChain %_ptr_UniformConstant_S [[base]] %int_0
// CHECK: [[elem:%[0-9]+]] = OpLoad %S [[ptr]]
// CHECK:                 OpStore %param_var_x [[elem]]
// CHECK:                 OpFunctionCall %v4float %tex2D %param_var_x %param_var_v
  return tex2D(globalS[0][0], float2(0,0));
}

