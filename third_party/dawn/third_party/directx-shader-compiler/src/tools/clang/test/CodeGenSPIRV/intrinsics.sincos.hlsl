// RUN: %dxc -T vs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// CHECK:      [[glsl:%[0-9]+]] = OpExtInstImport "GLSL.std.450"

void main() {
  float    a, sina, cosa;
  float4   b, sinb, cosb;
  float2x3 c, sinc, cosc;

// CHECK:        [[a0:%[0-9]+]] = OpLoad %float %a
// CHECK-NEXT: [[sina:%[0-9]+]] = OpExtInst %float [[glsl]] Sin [[a0]]
// CHECK-NEXT:                 OpStore %sina [[sina]]
// CHECK-NEXT:   [[a1:%[0-9]+]] = OpLoad %float %a
// CHECK-NEXT: [[cosa:%[0-9]+]] = OpExtInst %float [[glsl]] Cos [[a1]]
// CHECK-NEXT:                 OpStore %cosa [[cosa]]
  sincos(a, sina, cosa);

// CHECK:        [[b0:%[0-9]+]] = OpLoad %v4float %b
// CHECK-NEXT: [[sinb:%[0-9]+]] = OpExtInst %v4float [[glsl]] Sin [[b0]]
// CHECK-NEXT:                 OpStore %sinb [[sinb]]
// CHECK-NEXT:   [[b1:%[0-9]+]] = OpLoad %v4float %b
// CHECK-NEXT: [[cosb:%[0-9]+]] = OpExtInst %v4float [[glsl]] Cos [[b1]]
// CHECK-NEXT:                 OpStore %cosb [[cosb]]
  sincos(b, sinb, cosb);

// CHECK:             [[c0:%[0-9]+]] = OpLoad %mat2v3float %c
// CHECK-NEXT:   [[c0_row0:%[0-9]+]] = OpCompositeExtract %v3float [[c0]] 0
// CHECK-NEXT: [[sinc_row0:%[0-9]+]] = OpExtInst %v3float [[glsl]] Sin [[c0_row0]]
// CHECK-NEXT:   [[c0_row1:%[0-9]+]] = OpCompositeExtract %v3float [[c0]] 1
// CHECK-NEXT: [[sinc_row1:%[0-9]+]] = OpExtInst %v3float [[glsl]] Sin [[c0_row1]]
// CHECK-NEXT:      [[sinc:%[0-9]+]] = OpCompositeConstruct %mat2v3float [[sinc_row0]] [[sinc_row1]]
// CHECK-NEXT:                      OpStore %sinc [[sinc]]
// CHECK-NEXT:        [[c1:%[0-9]+]] = OpLoad %mat2v3float %c
// CHECK-NEXT:   [[c1_row0:%[0-9]+]] = OpCompositeExtract %v3float [[c1]] 0
// CHECK-NEXT: [[cosc_row0:%[0-9]+]] = OpExtInst %v3float [[glsl]] Cos [[c1_row0]]
// CHECK-NEXT:   [[c1_row1:%[0-9]+]] = OpCompositeExtract %v3float [[c1]] 1
// CHECK-NEXT: [[cosc_row1:%[0-9]+]] = OpExtInst %v3float [[glsl]] Cos [[c1_row1]]
// CHECK-NEXT:      [[cosc:%[0-9]+]] = OpCompositeConstruct %mat2v3float [[cosc_row0]] [[cosc_row1]]
// CHECK-NEXT:                      OpStore %cosc [[cosc]]
  sincos(c, sinc, cosc);
}
