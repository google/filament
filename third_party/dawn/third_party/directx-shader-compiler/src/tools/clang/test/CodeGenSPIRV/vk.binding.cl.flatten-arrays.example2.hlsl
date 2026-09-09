// RUN: %dxc -T ps_6_0 -E main -fspv-flatten-resource-arrays -fcgl  %s -spirv | FileCheck %s


// CHECK: OpDecorate %AnotherTexture Binding 3
// CHECK: OpDecorate %MyTextures Binding 4
// CHECK: OpDecorate %MyTextures2 Binding 0
// CHECK: OpDecorate %MySampler Binding 2
// CHECK: OpDecorate %MySampler2 Binding 9

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

