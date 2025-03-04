// RUN: %dxc -T ps_6_0 -E main -fspv-flatten-resource-arrays -O3  %s -spirv | FileCheck %s

// CHECK: OpDecorate %AnotherTexture Binding 5
// CHECK: OpDecorate %NextTexture Binding 6
// CHECK: OpDecorate %MyTextures_0_ Binding 0
// CHECK: OpDecorate %MyTextures_1_ Binding 1
// CHECK: OpDecorate %MyTextures_2_ Binding 2
// CHECK: OpDecorate %MyTextures_3_ Binding 3
// CHECK: OpDecorate %MyTextures_4_ Binding 4
// CHECK: OpDecorate %MySamplers_0_ Binding 7
// CHECK: OpDecorate %MySamplers_1_ Binding 8

// CHECK: %MyTextures_0_ = OpVariable %_ptr_UniformConstant_type_2d_image UniformConstant
// CHECK: %MyTextures_1_ = OpVariable %_ptr_UniformConstant_type_2d_image UniformConstant
// CHECK: %MyTextures_2_ = OpVariable %_ptr_UniformConstant_type_2d_image UniformConstant
// CHECK: %MyTextures_3_ = OpVariable %_ptr_UniformConstant_type_2d_image UniformConstant
// CHECK: %MyTextures_4_ = OpVariable %_ptr_UniformConstant_type_2d_image UniformConstant
// CHECK: %MySamplers_0_ = OpVariable %_ptr_UniformConstant_type_sampler UniformConstant
// CHECK: %MySamplers_1_ = OpVariable %_ptr_UniformConstant_type_sampler UniformConstant
Texture2D    MyTextures[5] : register(t0);
Texture2D    NextTexture;  // This is suppose to be t6.
Texture2D    AnotherTexture : register(t5);
SamplerState MySamplers[2];

float4 main(float2 TexCoord : TexCoord) : SV_Target0
{
  float4 result =
    MyTextures[0].Sample(MySamplers[0], TexCoord) +
    MyTextures[1].Sample(MySamplers[0], TexCoord) +
    MyTextures[2].Sample(MySamplers[0], TexCoord) +
    MyTextures[3].Sample(MySamplers[1], TexCoord) +
    MyTextures[4].Sample(MySamplers[1], TexCoord) +
    AnotherTexture.Sample(MySamplers[1], TexCoord) +
    NextTexture.Sample(MySamplers[1], TexCoord);
  return result;
}
