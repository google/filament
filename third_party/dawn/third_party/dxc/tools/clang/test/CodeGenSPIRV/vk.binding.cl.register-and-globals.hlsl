// RUN: %dxc -T ps_6_0 -E main -fvk-bind-register t5 0 1 2 -vkbr s3 1 3 4 -fvk-bind-globals 7 8 -fcgl  %s -spirv | FileCheck %s

// CHECK: OpDecorate %MyTexture DescriptorSet 2
// CHECK: OpDecorate %MyTexture Binding 1
Texture2D MyTexture    : register(t5);
// CHECK: OpDecorate %MySampler DescriptorSet 4
// CHECK: OpDecorate %MySampler Binding 3
SamplerState MySampler : register(s3, space1);

// CHECK: OpDecorate %_Globals DescriptorSet 8
// CHECK: OpDecorate %_Globals Binding 7
int globalInteger;
float4 globalFloat4;

float4 main() : SV_Target {
  return MyTexture.Sample(MySampler, float2(0.1, 0.2));
}

