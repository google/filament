// RUN: %dxc -T ps_6_0 -E main -fcgl -Vd %s -spirv | FileCheck %s

// globalS.t should take binding #0.
// globalS.s should take binding #1.
//
// CHECK: OpDecorate %globalS DescriptorSet 0
// CHECK: OpDecorate %globalS Binding 0
//
// CHECK: OpDecorate %globalTexture DescriptorSet 0
// CHECK: OpDecorate %globalTexture Binding 2
//
// CHECK: OpDecorate %globalSamplerState DescriptorSet 0
// CHECK: OpDecorate %globalSamplerState Binding 3
//
// CHECK: OpDecorate %MySubpassInput DescriptorSet 0
// CHECK: OpDecorate %MySubpassInput Binding 4

// CHECK:                          %S = OpTypeStruct %type_2d_image %type_sampler
// CHECK:     %_ptr_UniformConstant_S = OpTypePointer UniformConstant %S
// CHECK-NOT:          %type__Globals =

struct S {
  Texture2D t;
  SamplerState s;
};

float4 tex2D(S x, float2 v) { return x.t.Sample(x.s, v); }

// CHECK:      %globalS = OpVariable %_ptr_UniformConstant_S UniformConstant
// CHECK-NOT: %_Globals = OpVariable
S globalS;


Texture2D globalTexture;
SamplerState globalSamplerState;
[[vk::input_attachment_index (0)]] SubpassInput<float4> MySubpassInput;

float4 main() : SV_Target {
// CHECK: [[globalS:%[0-9]+]] = OpLoad %S %globalS
// CHECK:                    OpStore %param_var_x [[globalS]]
// CHECK:                    OpFunctionCall %v4float %tex2D %param_var_x %param_var_v
  return tex2D(globalS, float2(0,0));
}

