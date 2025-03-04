// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// CHECK: createHandle
// CHECK: i1 true)

// CHECK: createHandle
// CHECK: i1 true)

// CHECK: createHandle
// CHECK: i1 true)

Texture1D<float4> tex[5] : register(t3);
SamplerState SS[3] : register(s2);

[RootSignature("DescriptorTable(SRV(t3, numDescriptors=5)),\
                DescriptorTable(Sampler(s2, numDescriptors=3))")]
float4 main(int i : A, float j : B) : SV_TARGET
{
  float4 r = tex[NonUniformResourceIndex(i)].Sample(SS[NonUniformResourceIndex(i)], i);
  r += tex[NonUniformResourceIndex(j)].Sample(SS[i], j+2);
  return r;
}
