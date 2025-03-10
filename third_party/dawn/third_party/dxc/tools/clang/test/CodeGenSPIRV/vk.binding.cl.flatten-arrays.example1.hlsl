// RUN: %dxc -T ps_6_0 -E main -fspv-flatten-resource-arrays -fcgl  %s -spirv | FileCheck %s

// CHECK: OpDecorate %MyTextures Binding 0
// CHECK: OpDecorate %AnotherTexture Binding 5
// CHECK: OpDecorate %NextTexture Binding 6
// CHECK: OpDecorate %MySamplers Binding 7
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
