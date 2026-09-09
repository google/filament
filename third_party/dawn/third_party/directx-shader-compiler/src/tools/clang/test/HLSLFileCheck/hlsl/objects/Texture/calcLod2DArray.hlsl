// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// CHECK: calculateLOD
// CHECK: calculateLOD
// CHECK: calculateLOD

SamplerState samp1;

Texture2D<float4> tex1;
Texture2DArray<float4> tex2;
TextureCubeArray<float4> tex3;

float4 main(float4 a : A) : SV_Target
{
  float4 r = 0;
  r += tex1.CalculateLevelOfDetail(samp1, a.xy);  // sampler, coordinates
  r += tex2.CalculateLevelOfDetail(samp1, a.xy);
  r += tex3.CalculateLevelOfDetail(samp1, a.xyz);

  return r;
}