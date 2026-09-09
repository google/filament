// RUN: %dxc -T ps_6_5 %s | %FileCheck %s
// RUN: %dxc -T ps_6_6 %s | %FileCheck %s -check-prefix=FAIL

// Make sure that global heap variable redeclaration fails
// on 6.6 and passes on previous shader models

// FAIL: error: redefinition of 'ResourceDescriptorHeap' with a different type: 'Texture2D<float> []' vs '.Resource'
// FAIL: error: redefinition of 'SamplerDescriptorHeap' with a different type: 'SamplerState []' vs '.Sampler'

// Verify that feature bits aren't set and the shader produce roughly reasonable code
// CHECK-NOT: Resource descriptor heap indexing
// CHECK-NOT: Sampler descriptor heap indexing
// CHECK: @main
// CHECK: @dx.op.sample.f32

Texture2D<float> ResourceDescriptorHeap[] : register(t0);
SamplerState SamplerDescriptorHeap[] : register(s0);

uint ID;
float main(float4 pos: SV_Position): SV_Target {
  Texture2D<float> tex = ResourceDescriptorHeap[ID];
  SamplerState samp = SamplerDescriptorHeap[ID];
  return tex.Sample(samp, pos.xy);
}

