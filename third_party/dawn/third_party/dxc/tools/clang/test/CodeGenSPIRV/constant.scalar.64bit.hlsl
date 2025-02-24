// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s

void main() {

// CHECK: %double_0 = OpConstant %double 0
  double c_double_0 = 0.;
// CHECK: %double_n0 = OpConstant %double -0
  float64_t c_double_n0 = -0.;
// CHECK: %double_4_5 = OpConstant %double 4.5
  float64_t c_double_4_5 = 4.5;
// CHECK: %double_n8_5 = OpConstant %double -8.5
  double c_double_n8_5 = -8.5;
// CHECK: %double_1234567898765_3201 = OpConstant %double 1234567898765.3201
  double c_large  =  1234567898765.32;
// CHECK: %double_n1234567898765_3201 = OpConstant %double -1234567898765.3201
  float64_t c_nlarge = -1234567898765.32;

// CHECK: %long_1 = OpConstant %long 1
  int64_t  c_int64_small_1  = 1;
// CHECK: %long_n1 = OpConstant %long -1
  int64_t  c_int64_small_n1  = -1;
// CHECK: %long_2147483648 = OpConstant %long 2147483648
  int64_t  c_int64_large  = 2147483648;

// CHECK: %ulong_2 = OpConstant %ulong 2
  uint64_t c_uint64_small_2 = 2;
// CHECK: %ulong_4294967296 = OpConstant %ulong 4294967296
  uint64_t c_uint64_large = 4294967296;
}
