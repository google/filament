// RUN: %dxc -T ps_6_0 -E main -O0  %s -spirv | FileCheck %s

//CHECK: OpDecorate %foo_x DescriptorSet 0
//CHECK: OpDecorate %foo_x Binding 0
//CHECK: OpDecorate %foo_x RelaxedPrecision
//CHECK: OpDecorate %foo_y DescriptorSet 0
//CHECK: OpDecorate %foo_y Binding 1

struct S {
  Texture2D<min16float4> x;
  SamplerState y;
};

min16float4 SampleS(S s, float2 v) {
  return s.x.Sample (s.y, v);
}

S foo;

min16float4 main(float2 uv : TEXCOORD) : SV_Target
{
  min16float4 color = SampleS(foo, uv);
  return color;
}
