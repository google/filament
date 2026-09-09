// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// Regression test for a validation error, where parameter SROA
// would generate GEPs before the indices it uses

// CHECK: @main

Texture2D tex0[10] : register(t0);

float4 f(Texture2D textures[], unsigned int idx) {
  return textures[idx].Load(0);
}

[ RootSignature("DescriptorTable(SRV(t0, numDescriptors=10))") ]
float4 main() : SV_Target {
  return f(tex0, 1);
}
