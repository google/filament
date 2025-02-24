// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// CHECK-NOT: OpDecorate %buf3

// CHECK: OpDecorate %buf0 DescriptorSet 0
// CHECK: OpDecorate %buf0 Binding 0
// CHECK: OpDecorate %buf1 DescriptorSet 0
// CHECK: OpDecorate %buf1 Binding 4
// CHECK: OpDecorate %y DescriptorSet 0
// CHECK: OpDecorate %y Binding 1
// CHECK: OpDecorate %z DescriptorSet 0
// CHECK: OpDecorate %z Binding 2
// CHECK: OpDecorate %buf2 DescriptorSet 0
// CHECK: OpDecorate %buf2 Binding 3
// CHECK: OpDecorate %w DescriptorSet 0
// CHECK: OpDecorate %w Binding 5

// CHECK: %type_buf0 = OpTypeStruct %v4float
cbuffer buf0 : register(b0) {
  float4 foo;
};

// CHECK: %type_buf1 = OpTypeStruct %v4float
cbuffer buf1 : register(b4) {
  float4 bar;
};

// CHECK: %type_buf2 = OpTypeStruct %v4float
// CHECK: %type_buf3 = OpTypeStruct

// CHECK: %y = OpVariable %_ptr_UniformConstant_type_2d_image UniformConstant
// CHECK: %z = OpVariable %_ptr_UniformConstant_type_sampler UniformConstant
cbuffer buf2 {
  float4 x;
  Texture2D y;
  SamplerState z;
};

// CHECK: %w = OpVariable %_ptr_UniformConstant_type_sampler UniformConstant
cbuffer buf3 : register(b2) {
  SamplerState w;
}

float4 main(float2 uv : TEXCOORD) : SV_TARGET {
// CHECK: [[ptr_x:%[0-9]+]] = OpAccessChain %_ptr_Uniform_v4float %buf2 %int_0
// CHECK: [[x:%[0-9]+]] = OpLoad %v4float [[ptr_x]]

// CHECK: [[y:%[0-9]+]] = OpLoad %type_2d_image %y
// CHECK: [[z:%[0-9]+]] = OpLoad %type_sampler %z
// CHECK: [[yz:%[0-9]+]] = OpSampledImage %type_sampled_image [[y]] [[z]]
// CHECK: [[sample0:%[0-9]+]] = OpImageSampleImplicitLod %v4float [[yz]]
// CHECK: [[add0:%[0-9]+]] = OpFAdd %v4float [[x]] [[sample0]]

// CHECK: [[y_0:%[0-9]+]] = OpLoad %type_2d_image %y
// CHECK: [[w:%[0-9]+]] = OpLoad %type_sampler %w
// CHECK: [[yw:%[0-9]+]] = OpSampledImage %type_sampled_image [[y_0]] [[w]]
// CHECK: [[sample1:%[0-9]+]] = OpImageSampleImplicitLod %v4float [[yw]]
// CHECK: OpFAdd %v4float [[add0]] [[sample1]]
  return x + y.Sample(z, uv) + y.Sample(w, uv);
}
