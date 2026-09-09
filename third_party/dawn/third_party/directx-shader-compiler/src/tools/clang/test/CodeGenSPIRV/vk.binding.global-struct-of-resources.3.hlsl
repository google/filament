// RUN: %dxc -T ps_6_0 -E main -fcgl -Vd %s -spirv | FileCheck %s

// globalS.t should take binding #0.
// globalS.s should take binding #1.
// CHECK: OpDecorate %globalS DescriptorSet 0
// CHECK: OpDecorate %globalS Binding 0
//
// gCounter (and hence $Globals cbuffer) appears after globalS, and before
// globalTexture, therefore, it takes binding #2.
// CHECK: OpDecorate %_Globals DescriptorSet 0
// CHECK: OpDecorate %_Globals Binding 2
//
// CHECK: OpDecorate %globalTexture DescriptorSet 0
// CHECK: OpDecorate %globalTexture Binding 3

// CHECK:                          %S = OpTypeStruct %type_2d_image %type_sampler
// CHECK:     %_ptr_UniformConstant_S = OpTypePointer UniformConstant %S

// Struct S (globalS) and Texture2D (globalTexture) should NOT be placed in the $Globals cbuffer.
// The $Globals cbuffer should only contain one float (gCounter).
//
// CHECK:              %type__Globals = OpTypeStruct %float
// CHECK: %_ptr_Uniform_type__Globals = OpTypePointer Uniform %type__Globals

struct S {
  Texture2D t;
  SamplerState s;
};

float4 tex2D(S x, float2 v) { return x.t.Sample(x.s, v); }

// CHECK: %globalS = OpVariable %_ptr_UniformConstant_S UniformConstant
S globalS;

// %_Globals = OpVariable %_ptr_Uniform_type__Globals Uniform
float gCounter;

// %globalTexture = OpVariable %_ptr_UniformConstant_type_2d_image UniformConstant
Texture2D globalTexture;

float4 main() : SV_Target {
  return tex2D(globalS, float2(0,0));
}

