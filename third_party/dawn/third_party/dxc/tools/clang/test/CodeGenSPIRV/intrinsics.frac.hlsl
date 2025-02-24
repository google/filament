// RUN: %dxc -T vs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// CHECK: [[glsl:%[0-9]+]] = OpExtInstImport "GLSL.std.450"

void main() {
  float    a, frac_a;
  float4   b, frac_b;
  float2x3 c, frac_c;

// CHECK:      [[a:%[0-9]+]] = OpLoad %float %a
// CHECK-NEXT:   {{%[0-9]+}} = OpExtInst %float [[glsl]] Fract [[a]]
  frac_a = frac(a);

// CHECK:      [[b:%[0-9]+]] = OpLoad %v4float %b
// CHECK-NEXT:   {{%[0-9]+}} = OpExtInst %v4float [[glsl]] Fract [[b]]
  frac_b = frac(b);

// CHECK:               [[c:%[0-9]+]] = OpLoad %mat2v3float %c
// CHECK-NEXT:     [[c_row0:%[0-9]+]] = OpCompositeExtract %v3float [[c]] 0
// CHECK-NEXT:[[frac_c_row0:%[0-9]+]] = OpExtInst %v3float [[glsl]] Fract [[c_row0]]
// CHECK-NEXT:     [[c_row1:%[0-9]+]] = OpCompositeExtract %v3float [[c]] 1
// CHECK-NEXT:[[frac_c_row1:%[0-9]+]] = OpExtInst %v3float [[glsl]] Fract [[c_row1]]
// CHECK-NEXT:            {{%[0-9]+}} = OpCompositeConstruct %mat2v3float [[frac_c_row0]] [[frac_c_row1]]
  frac_c = frac(c);
}
