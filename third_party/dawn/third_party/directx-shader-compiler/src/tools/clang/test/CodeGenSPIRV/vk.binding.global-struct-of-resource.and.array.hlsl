// RUN: %dxc -T ps_6_6 -E main -spirv %s | FileCheck %s

// CHECK-DAG: OpName %Textures "Textures"
// CHECK-DAG: OpDecorate %Textures DescriptorSet 0
// CHECK-DAG: OpDecorate %Textures Binding 0
Texture2D<float4> Textures[10];

struct StructOfResources {
    Texture2D Texture;
    SamplerState Sampler;
};

// CHECK-DAG: OpName %TheStruct_Texture "TheStruct.Texture"
// CHECK-DAG: OpDecorate %TheStruct_Texture DescriptorSet 0
// CHECK-DAG: OpDecorate %TheStruct_Texture Binding 10

// CHECK-DAG: OpName %TheStruct_Sampler "TheStruct.Sampler"
// CHECK-DAG: OpDecorate %TheStruct_Sampler DescriptorSet 0
// CHECK-DAG: OpDecorate %TheStruct_Sampler Binding 11
StructOfResources TheStruct;

float4 main() : SV_Target
{
// CHECK: [[ptr:%[0-9]+]] = OpAccessChain %_ptr_UniformConstant_type_2d_image %Textures %int_0
// CHECK: [[tex:%[0-9]+]] = OpLoad %type_2d_image [[ptr]]
// CHECK: [[smp:%[0-9]+]] = OpLoad %type_sampler %TheStruct_Sampler
// CHECK:   [[x:%[0-9]+]] = OpSampledImage %type_sampled_image [[tex]] [[smp]]
  return Textures[0].Sample(TheStruct.Sampler, float2(0, 0))
// CHECK: [[tex:%[0-9]+]] = OpLoad %type_2d_image %TheStruct_Texture
// CHECK: [[smp:%[0-9]+]] = OpLoad %type_sampler %TheStruct_Sampler
// CHECK:   [[x:%[0-9]+]] = OpSampledImage %type_sampled_image [[tex]] [[smp]]
       + TheStruct.Texture.Sample(TheStruct.Sampler, float2(0, 0));
}
