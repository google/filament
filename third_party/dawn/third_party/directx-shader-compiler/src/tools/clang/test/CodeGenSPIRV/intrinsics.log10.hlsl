// RUN: %dxc -T vs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// According to HLSL reference:
// The 'log10' function can only operate on float, vector of float, and matrix of floats.

// CHECK:  [[glsl:%[0-9]+]] = OpExtInstImport "GLSL.std.450"
// CHECK: %float_0_30103001 = OpConstant %float 0.30103001

void main() {
  float    a, log10_a;
  float4   b, log10_b;
  float2x3 c, log10_c;

// CHECK:           [[a:%[0-9]+]] = OpLoad %float %a
// CHECK-NEXT: [[log2_a:%[0-9]+]] = OpExtInst %float [[glsl]] Log2 [[a]]
// CHECK-NEXT:[[log10_a:%[0-9]+]] = OpFMul %float [[log2_a]] %float_0_30103
// CHECK-NEXT:                   OpStore %log10_a [[log10_a]]
  log10_a = log10(a);

// CHECK:           [[b:%[0-9]+]] = OpLoad %v4float %b
// CHECK-NEXT: [[log2_b:%[0-9]+]] = OpExtInst %v4float [[glsl]] Log2 [[b]]
// CHECK-NEXT:[[log10_b:%[0-9]+]] = OpVectorTimesScalar %v4float [[log2_b]] %float_0_30103
// CHECK-NEXT:                   OpStore %log10_b [[log10_b]]
  log10_b = log10(b);

// CHECK:                [[c:%[0-9]+]] = OpLoad %mat2v3float %c
// CHECK-NEXT:      [[c_row0:%[0-9]+]] = OpCompositeExtract %v3float [[c]] 0
// CHECK-NEXT: [[log2_c_row0:%[0-9]+]] = OpExtInst %v3float [[glsl]] Log2 [[c_row0]]
// CHECK-NEXT:      [[c_row1:%[0-9]+]] = OpCompositeExtract %v3float [[c]] 1
// CHECK-NEXT: [[log2_c_row1:%[0-9]+]] = OpExtInst %v3float [[glsl]] Log2 [[c_row1]]
// CHECK-NEXT:      [[log2_c:%[0-9]+]] = OpCompositeConstruct %mat2v3float [[log2_c_row0]] [[log2_c_row1]]
// CHECK-NEXT:     [[log10_c:%[0-9]+]] = OpMatrixTimesScalar %mat2v3float [[log2_c]] %float_0_30103
// CHECK-NEXT:                        OpStore %log10_c [[log10_c]]
  log10_c = log10(c);
}
