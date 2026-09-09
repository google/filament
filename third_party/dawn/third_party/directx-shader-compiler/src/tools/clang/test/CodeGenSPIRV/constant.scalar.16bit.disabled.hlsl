// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s

/////////////////////////////////////////////////////////////////////////////////
/// Types with fewer than 32 bits, used without '-enable-16bit-type' options  ///
/////////////////////////////////////////////////////////////////////////////////

// See https://github.com/Microsoft/DirectXShaderCompiler/wiki/16-Bit-Scalar-Types
// for details about these types.

// CHECK-NOT: OpDecorate %c_half_4_5 RelaxedPrecision
// CHECK-NOT: OpDecorate %c_half_n8_2 RelaxedPrecision
// CHECK: OpDecorate %c_min10float RelaxedPrecision
// CHECK: OpDecorate %c_min16float RelaxedPrecision
// CHECK: OpDecorate %c_min16int_n3 RelaxedPrecision
// CHECK: OpDecorate %c_min16uint_5 RelaxedPrecision
// CHECK: OpDecorate %c_min12int RelaxedPrecision

void main() {
// Note: in the absence of "-enable-16bit-types" option,
// 'half' is translated to float *without* RelaxedPrecision decoration.
// CHECK: %float_7_5 = OpConstant %float 7.5
  half c_half_7_5 = 7.5;
// CHECK: %float_n8_80000019 = OpConstant %float -8.80000019
  half c_half_n8_8 = -8.8;

// Note: in the absence of "-enable-16bit-type" option,
// 'min{10|16}float' are translated to
// 32-bit float in SPIR-V with RelaxedPrecision decoration (checked above).
// CHECK: %float_1_5 = OpConstant %float 1.5
  min10float c_min10float = 1.5;
// CHECK: %float_n1 = OpConstant %float -1
  min16float c_min16float = -1.0;

// Note: in the absence of "-enable-16bit-type" option,
// 'min12{uint|int}' and 'min16{uint|int}' are translated to
// 32-bit uint/int in SPIR-V with RelaxedPrecision decoration (checked above).
// CHECK: %int_n3 = OpConstant %int -3
  min16int c_min16int_n3 = -3;
// CHECK: %uint_5 = OpConstant %uint 5
  min16uint c_min16uint_5 = 5;
// CHECK: %int_n9 = OpConstant %int -9
  min12int c_min12int = -9;
// It seems that min12uint is still not supported by the front-end.
// XXXXX: %uint_12 = OpConstant %uint 12
//  min12uint c_min12uint = 12;
}
