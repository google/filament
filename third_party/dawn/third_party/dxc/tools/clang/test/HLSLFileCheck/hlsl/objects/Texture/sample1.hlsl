// RUN: %dxc -E main -T ps_6_0 %s | tee | FileCheck %s

// This test uses 'tee' simply to exist as a sample; typically it's only
// desirable for looking at the disassembly during investigations.

// CHECK:     unused_but_part_of_cbuffer
// CHECK-NOT: !"samp_unused"
// CHECK:     !"samp1"
// CHECK-NOT: !"samp_unused"

cbuffer Foo
{
  float4 g0;
  float4 g1;
  float4 g2;
};

int unused_but_part_of_cbuffer;
int used_part;
SamplerState samp_unused : register(s6);
SamplerState samp1 : register(s5);
Texture2D<float4> text1 : register(t3);

float4 unreferenced_fn() {
  return text1.Sample(samp_unused, 0) + g2;
}

float4 main(float2 a : A) : SV_Target {
  return text1.Sample(samp1, a) + g2 + used_part;
}
