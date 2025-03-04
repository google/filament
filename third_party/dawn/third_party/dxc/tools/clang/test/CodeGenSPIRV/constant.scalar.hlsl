// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// TODO
// float: denormalized numbers, Inf, NaN

void main() {
  // Boolean constants
// CHECK: %true = OpConstantTrue %bool
  bool c_bool_t = true;
// CHECK: %false = OpConstantFalse %bool
  bool c_bool_f = false;

  // Signed integer constants
// CHECK: %int_0 = OpConstant %int 0
  int c_int_0 = 0;
// CHECK: %int_1 = OpConstant %int 1
  int c_int_1 = 1;
// CHECK: %int_n1 = OpConstant %int -1
  int c_int_n1 = -1;
// CHECK: %int_42 = OpConstant %int 42
  int c_int_42 = 42;
// CHECK: %int_n42 = OpConstant %int -42
  int c_int_n42 = -42;
// CHECK: %int_2147483647 = OpConstant %int 2147483647
  int c_int_max = 2147483647;
// CHECK: %int_n2147483648 = OpConstant %int -2147483648
  int c_int_min = -2147483648;

  // Unsigned integer constants
// CHECK: %uint_0 = OpConstant %uint 0
  uint c_uint_0 = 0;
// CHECK: %uint_1 = OpConstant %uint 1
  uint c_uint_1 = 1;
// CHECK: %uint_38 = OpConstant %uint 38
  uint c_uint_38 = 38;
// CHECK: %uint_4294967295 = OpConstant %uint 4294967295
  uint c_uint_max = 4294967295;

  // Float constants
// CHECK: %float_0 = OpConstant %float 0
  float c_float_0 = 0.;
// CHECK: %float_n0 = OpConstant %float -0
  float c_float_n0 = -0.;
// CHECK: %float_4_25 = OpConstant %float 4.25
  float c_float_4_25 = 4.25;
// CHECK: %float_n4_19999981 = OpConstant %float -4.19999981
  float c_float_n4_2 = -4.2;
}
