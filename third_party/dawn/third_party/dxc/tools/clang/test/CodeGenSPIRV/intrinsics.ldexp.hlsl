// RUN: %dxc -T vs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// According to HLSL reference:
// The 'ldexp' function can only operate on float, vector of float, and matrix of floats.

// CHECK:      [[glsl:%[0-9]+]] = OpExtInstImport "GLSL.std.450"

void main() {
  float    a1, a2, ldexp_a;
  float4   b1, b2, ldexp_b;
  float2x3 c1, c2, ldexp_c;
  
// CHECK:          [[a1:%[0-9]+]] = OpLoad %float %a1
// CHECK-NEXT:     [[a2:%[0-9]+]] = OpLoad %float %a2
// CHECK-NEXT:    [[exp:%[0-9]+]] = OpExtInst %float [[glsl]] Exp2 [[a2]]
// CHECK-NEXT:    [[res:%[0-9]+]] = OpFMul %float [[a1]] [[exp]]
// CHECK-NEXT:                   OpStore %ldexp_a [[res]]
  ldexp_a = ldexp(a1, a2);

// CHECK:          [[b1:%[0-9]+]] = OpLoad %v4float %b1
// CHECK-NEXT:     [[b2:%[0-9]+]] = OpLoad %v4float %b2
// CHECK-NEXT:    [[exp_0:%[0-9]+]] = OpExtInst %v4float [[glsl]] Exp2 [[b2]]
// CHECK-NEXT:    [[res_0:%[0-9]+]] = OpFMul %v4float [[b1]] [[exp_0]]
// CHECK-NEXT:                   OpStore %ldexp_b [[res_0]]
  ldexp_b = ldexp(b1, b2);

// CHECK:               [[c1:%[0-9]+]] = OpLoad %mat2v3float %c1
// CHECK-NEXT:          [[c2:%[0-9]+]] = OpLoad %mat2v3float %c2
// CHECK-NEXT:     [[c1_row0:%[0-9]+]] = OpCompositeExtract %v3float [[c1]] 0
// CHECK-NEXT:     [[c2_row0:%[0-9]+]] = OpCompositeExtract %v3float [[c2]] 0
// CHECK-NEXT:         [[exp_1:%[0-9]+]] = OpExtInst %v3float [[glsl]] Exp2 [[c2_row0]]
// CHECK-NEXT:        [[res0:%[0-9]+]] = OpFMul %v3float [[c1_row0]] [[exp_1]]
// CHECK-NEXT:     [[c1_row1:%[0-9]+]] = OpCompositeExtract %v3float [[c1]] 1
// CHECK-NEXT:     [[c2_row1:%[0-9]+]] = OpCompositeExtract %v3float [[c2]] 1
// CHECK-NEXT:         [[exp_2:%[0-9]+]] = OpExtInst %v3float [[glsl]] Exp2 [[c2_row1]]
// CHECK-NEXT:        [[res1:%[0-9]+]] = OpFMul %v3float [[c1_row1]] [[exp_2]]
// CHECK-NEXT:     [[ldexp_c:%[0-9]+]] = OpCompositeConstruct %mat2v3float [[res0]] [[res1]]
// CHECK-NEXT:                        OpStore %ldexp_c [[ldexp_c]]
  ldexp_c = ldexp(c1, c2);
}
