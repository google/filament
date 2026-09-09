// RUN: %dxc -T ps_6_0 %s  | FileCheck %s

// CHECK: error: resource MyTexture could not be allocated

Texture2D Textures[];
Texture2D MyTexture;

sampler samp : register(s0);

float4 main() : SV_Target
{
 MyTexture = Textures[0];
 return MyTexture.Sample(samp, 0.0);
}
