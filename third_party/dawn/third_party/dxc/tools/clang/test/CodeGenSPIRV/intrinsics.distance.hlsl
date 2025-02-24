// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// CHECK:  [[glsl:%[0-9]+]] = OpExtInstImport "GLSL.std.450"

void main() {
  float3 a, b;

// CHECK:        [[a:%[0-9]+]] = OpLoad %v3float %a
// CHECK-NEXT:   [[b:%[0-9]+]] = OpLoad %v3float %b
// CHECK-NEXT:     {{%[0-9]+}} = OpExtInst %float [[glsl]] Distance [[a]] [[b]]
  float d = distance(a, b);
}
