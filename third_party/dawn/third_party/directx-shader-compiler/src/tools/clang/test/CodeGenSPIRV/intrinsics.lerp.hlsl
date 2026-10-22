// RUN: %dxc -T vs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// According to HLSL reference:
// The 'lerp' function can only operate on float, vector of float, and matrix of floats.

// CHECK:      [[glsl:%[0-9]+]] = OpExtInstImport "GLSL.std.450"

void main() {
  float    a1, a2, a3, lerp_a;
  float4   b1, b2, b3, lerp_b;
  float2x3 c1, c2, c3, lerp_c;
  
// CHECK:          [[a1:%[0-9]+]] = OpLoad %float %a1
// CHECK-NEXT:     [[a2:%[0-9]+]] = OpLoad %float %a2
// CHECK-NEXT:     [[a3:%[0-9]+]] = OpLoad %float %a3
// CHECK-NEXT: [[lerp_a:%[0-9]+]] = OpExtInst %float [[glsl]] FMix [[a1]] [[a2]] [[a3]]
// CHECK-NEXT:                   OpStore %lerp_a [[lerp_a]]
  lerp_a = lerp(a1, a2, a3);

// CHECK:          [[b1:%[0-9]+]] = OpLoad %v4float %b1
// CHECK-NEXT:     [[b2:%[0-9]+]] = OpLoad %v4float %b2
// CHECK-NEXT:     [[b3:%[0-9]+]] = OpLoad %v4float %b3
// CHECK-NEXT: [[lerp_b:%[0-9]+]] = OpExtInst %v4float [[glsl]] FMix [[b1]] [[b2]] [[b3]]
// CHECK-NEXT:                   OpStore %lerp_b [[lerp_b]]
  lerp_b = lerp(b1, b2, b3);

// CHECK:               [[c1:%[0-9]+]] = OpLoad %mat2v3float %c1
// CHECK-NEXT:          [[c2:%[0-9]+]] = OpLoad %mat2v3float %c2
// CHECK-NEXT:          [[c3:%[0-9]+]] = OpLoad %mat2v3float %c3
// CHECK-NEXT:     [[c1_row0:%[0-9]+]] = OpCompositeExtract %v3float [[c1]] 0
// CHECK-NEXT:     [[c2_row0:%[0-9]+]] = OpCompositeExtract %v3float [[c2]] 0
// CHECK-NEXT:     [[c3_row0:%[0-9]+]] = OpCompositeExtract %v3float [[c3]] 0
// CHECK-NEXT: [[lerp_c_row0:%[0-9]+]] = OpExtInst %v3float [[glsl]] FMix [[c1_row0]] [[c2_row0]] [[c3_row0]]
// CHECK-NEXT:     [[c1_row1:%[0-9]+]] = OpCompositeExtract %v3float [[c1]] 1
// CHECK-NEXT:     [[c2_row1:%[0-9]+]] = OpCompositeExtract %v3float [[c2]] 1
// CHECK-NEXT:     [[c3_row1:%[0-9]+]] = OpCompositeExtract %v3float [[c3]] 1
// CHECK-NEXT: [[lerp_c_row1:%[0-9]+]] = OpExtInst %v3float [[glsl]] FMix [[c1_row1]] [[c2_row1]] [[c3_row1]]
// CHECK-NEXT:      [[lerp_c:%[0-9]+]] = OpCompositeConstruct %mat2v3float [[lerp_c_row0]] [[lerp_c_row1]]
// CHECK-NEXT:                        OpStore %lerp_c [[lerp_c]]
  lerp_c = lerp(c1, c2, c3);
}
