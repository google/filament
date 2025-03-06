// RUN: %dxc -T ps_6_0 -E main -fcgl -Vd %s -spirv | FileCheck %s

// globalS[0].t[0] should take binding #0.
// globalS[0].t[1] should take binding #1.
// globalS[0].s[0] should take binding #2.
// globalS[0].s[1] should take binding #3.
// globalS[0].s[2] should take binding #4.
// globalS[1].t[0] should take binding #5.
// globalS[1].t[1] should take binding #6.
// globalS[1].s[0] should take binding #7.
// globalS[1].s[1] should take binding #8.
// globalS[1].s[2] should take binding #9.
//
// CHECK: OpDecorate %globalS DescriptorSet 0
// CHECK: OpDecorate %globalS Binding 0
//
// CHECK: OpDecorate %globalTexture DescriptorSet 0
// CHECK: OpDecorate %globalTexture Binding 10
//
// CHECK: OpDecorate %globalSamplerState DescriptorSet 0
// CHECK: OpDecorate %globalSamplerState Binding 11


// CHECK:                                  %S = OpTypeStruct %_arr_type_2d_image_uint_2 %_arr_type_sampler_uint_3
// CHECK:                      %_arr_S_uint_2 = OpTypeArray %S %uint_2
// CHECK: %_ptr_UniformConstant__arr_S_uint_2 = OpTypePointer UniformConstant %_arr_S_uint_2
// CHECK-NOT:                  %type__Globals =

struct S {
  Texture2D t[2];
  SamplerState s[3];
};

float4 tex2D(S x, float2 v) { return x.t[0].Sample(x.s[0], v); }

// CHECK:      %globalS = OpVariable %_ptr_UniformConstant__arr_S_uint_2 UniformConstant
// CHECK-NOT: %_Globals = OpVariable
S globalS[2];

// %globalTexture = OpVariable %_ptr_UniformConstant_type_2d_image UniformConstant
Texture2D globalTexture;

// CHECK: %globalSamplerState = OpVariable %_ptr_UniformConstant_type_sampler UniformConstant
SamplerState globalSamplerState;

float4 main() : SV_Target {
  return tex2D(globalS[0], float2(0,0));
}

