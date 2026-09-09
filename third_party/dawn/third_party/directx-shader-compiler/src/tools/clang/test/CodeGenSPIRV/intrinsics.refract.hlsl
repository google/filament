// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// CHECK:  [[glsl:%[0-9]+]] = OpExtInstImport "GLSL.std.450"

void main() {
  float4 i, n;
  float eta;

// CHECK:        [[i:%[0-9]+]] = OpLoad %v4float %i
// CHECK-NEXT:   [[n:%[0-9]+]] = OpLoad %v4float %n
// CHECK-NEXT: [[eta:%[0-9]+]] = OpLoad %float %eta
// CHECK-NEXT:     {{%[0-9]+}} = OpExtInst %v4float [[glsl]] Refract [[i]] [[n]] [[eta]]
  float4 r = refract(i, n, eta);
}
