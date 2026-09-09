// RUN: %dxc -T ps_6_0 -E main -fspv-flatten-resource-arrays -O3  %s -spirv | FileCheck %s

// CHECK: OpDecorate %AnotherTexture Binding 3
// CHECK: OpDecorate %MySampler Binding 2
// CHECK: OpDecorate %MySampler2 Binding 9
// CHECK: OpDecorate %MyTextures_0_ Binding 4
// CHECK: OpDecorate %MyTextures_1_ Binding 5
// CHECK: OpDecorate %MyTextures_2_ Binding 6
// CHECK: OpDecorate %MyTextures_3_ Binding 7
// CHECK: OpDecorate %MyTextures_4_ Binding 8
// CHECK: OpDecorate %MyTextures2_0_ Binding 0
// CHECK: OpDecorate %MyTextures2_1_ Binding 1

// CHECK:  %MyTextures_0_ = OpVariable %_ptr_UniformConstant_type_2d_image UniformConstant
// CHECK:  %MyTextures_1_ = OpVariable %_ptr_UniformConstant_type_2d_image UniformConstant
// CHECK:  %MyTextures_2_ = OpVariable %_ptr_UniformConstant_type_2d_image UniformConstant
// CHECK:  %MyTextures_3_ = OpVariable %_ptr_UniformConstant_type_2d_image UniformConstant
// CHECK:  %MyTextures_4_ = OpVariable %_ptr_UniformConstant_type_2d_image UniformConstant
// CHECK: %MyTextures2_0_ = OpVariable %_ptr_UniformConstant_type_2d_image UniformConstant
// CHECK: %MyTextures2_1_ = OpVariable %_ptr_UniformConstant_type_2d_image UniformConstant

Texture2D    MyTextures[5]; // five array elements cannot fit in [0-2] binding slots, so it should take slot [4-8].
Texture2D    AnotherTexture : register(t3); // force binding number 3.
Texture2D    MyTextures2[2]; // take binding slot 0 and 1.
SamplerState MySampler; // take binding slot 2.
SamplerState MySampler2; // binding 0 to 8 are taken. The next available binding is 9.

float4 main(float2 TexCoord : TexCoord) : SV_Target0
{
  float4 result =
    MyTextures[0].Sample(MySampler, TexCoord) +
    MyTextures[1].Sample(MySampler, TexCoord) +
    MyTextures[2].Sample(MySampler, TexCoord) +
    MyTextures[3].Sample(MySampler, TexCoord) +
    MyTextures[4].Sample(MySampler, TexCoord) +
    MyTextures2[0].Sample(MySampler2, TexCoord) +
    MyTextures2[1].Sample(MySampler2, TexCoord) +
    AnotherTexture.Sample(MySampler, TexCoord);
  return result;
}

