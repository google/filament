// RUN: %dxc -T vs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// According to HLSL reference:
// The 'frexp' function can only operate on float, vector of float, and matrix of floats.

// CHECK: [[glsl:%[0-9]+]] = OpExtInstImport "GLSL.std.450"

// CHECK: OpName %FrexpStructType "FrexpStructType"
// CHECK: OpMemberName %FrexpStructType 0 "mantissa"
// CHECK: OpMemberName %FrexpStructType 1 "exponent"

// CHECK: OpName %FrexpStructType_0 "FrexpStructType"
// CHECK: OpMemberName %FrexpStructType_0 0 "mantissa"
// CHECK: OpMemberName %FrexpStructType_0 1 "exponent"

// CHECK: OpName %FrexpStructType_1 "FrexpStructType"
// CHECK: OpMemberName %FrexpStructType_1 0 "mantissa"
// CHECK: OpMemberName %FrexpStructType_1 1 "exponent"

// CHECK:   %FrexpStructType = OpTypeStruct %float %int
// CHECK: %FrexpStructType_0 = OpTypeStruct %v4float %v4int
// CHECK: %FrexpStructType_1 = OpTypeStruct %v3float %v3int

void main() {
  float    a, exp_a, mantissa_a;
  float4   b, exp_b, mantissa_b;
  float2x3 c, exp_c, mantissa_c;

// CHECK:                [[a:%[0-9]+]] = OpLoad %float %a
// CHECK-NEXT:     [[frexp_a:%[0-9]+]] = OpExtInst %FrexpStructType [[glsl]] FrexpStruct [[a]]
// CHECK-NEXT:   [[int_exp_a:%[0-9]+]] = OpCompositeExtract %int [[frexp_a]] 1
// CHECK-NEXT: [[float_exp_a:%[0-9]+]] = OpConvertSToF %float [[int_exp_a]]
// CHECK-NEXT:                        OpStore %exp_a [[float_exp_a]]
// CHECK-NEXT:  [[mantissa_a:%[0-9]+]] = OpCompositeExtract %float [[frexp_a]] 0
// CHECK-NEXT:                        OpStore %mantissa_a [[mantissa_a]]
  mantissa_a = frexp(a, exp_a);

// CHECK:                [[b:%[0-9]+]] = OpLoad %v4float %b
// CHECK-NEXT:     [[frexp_b:%[0-9]+]] = OpExtInst %FrexpStructType_0 [[glsl]] FrexpStruct [[b]]
// CHECK-NEXT:   [[int_exp_b:%[0-9]+]] = OpCompositeExtract %v4int [[frexp_b]] 1
// CHECK-NEXT: [[float_exp_b:%[0-9]+]] = OpConvertSToF %v4float [[int_exp_b]]
// CHECK-NEXT:                        OpStore %exp_b [[float_exp_b]]
// CHECK-NEXT:  [[mantissa_b:%[0-9]+]] = OpCompositeExtract %v4float [[frexp_b]] 0
// CHECK-NEXT:                        OpStore %mantissa_b [[mantissa_b]]
  mantissa_b = frexp(b, exp_b);

// CHECK:                     [[c:%[0-9]+]] = OpLoad %mat2v3float %c
// CHECK-NEXT:           [[c_row0:%[0-9]+]] = OpCompositeExtract %v3float [[c]] 0
// CHECK-NEXT:     [[c_frexp_row0:%[0-9]+]] = OpExtInst %FrexpStructType_1 [[glsl]] FrexpStruct [[c_row0]]
// CHECK-NEXT:   [[int_exp_c_row0:%[0-9]+]] = OpCompositeExtract %v3int [[c_frexp_row0]] 1
// CHECK-NEXT: [[float_exp_c_row0:%[0-9]+]] = OpConvertSToF %v3float [[int_exp_c_row0]]
// CHECK-NEXT:  [[mantissa_c_row0:%[0-9]+]] = OpCompositeExtract %v3float [[c_frexp_row0]] 0
// CHECK-NEXT:           [[c_row1:%[0-9]+]] = OpCompositeExtract %v3float [[c]] 1
// CHECK-NEXT:     [[c_frexp_row1:%[0-9]+]] = OpExtInst %FrexpStructType_1 [[glsl]] FrexpStruct [[c_row1]]
// CHECK-NEXT:   [[int_exp_c_row1:%[0-9]+]] = OpCompositeExtract %v3int [[c_frexp_row1]] 1
// CHECK-NEXT: [[float_exp_c_row1:%[0-9]+]] = OpConvertSToF %v3float [[int_exp_c_row1]]
// CHECK-NEXT:  [[mantissa_c_row1:%[0-9]+]] = OpCompositeExtract %v3float [[c_frexp_row1]] 0
// CHECK-NEXT:      [[float_exp_c:%[0-9]+]] = OpCompositeConstruct %mat2v3float [[float_exp_c_row0]] [[float_exp_c_row1]]
// CHECK-NEXT:                             OpStore %exp_c [[float_exp_c]]
// CHECK-NEXT:       [[mantissa_c:%[0-9]+]] = OpCompositeConstruct %mat2v3float [[mantissa_c_row0]] [[mantissa_c_row1]]
// CHECK-NEXT:                             OpStore %mantissa_c [[mantissa_c]]
  mantissa_c = frexp(c, exp_c);
}
