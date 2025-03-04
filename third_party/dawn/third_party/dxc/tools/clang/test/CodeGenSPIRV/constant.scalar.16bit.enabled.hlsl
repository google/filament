// RUN: %dxc -T ps_6_2 -E main -enable-16bit-types -fcgl  %s -spirv | FileCheck %s

// Handling of 16-bit integers and 16-bit floats.
// Note that this test runs utilizes "-enable-16bit-types" option above.

// When this option is enabled, 16-bit types are use as outlined below:
// min10float: float16_t(warning)
// min16float: float16_t(warning)
// half:       float16_t
// float16_t:  float16_t
// min12int:   int16_t(warning)
// min16int:   int16_t(warning)
// int16_t:    int16_t
// min12uint:  uint16_t(warning)
// min16uint:  uint16_t(warning)
// uint16_t:   uint16_t

// CHECK: OpCapability Float16
// CHECK: OpCapability Int16

// CHECK-NOT: OpDecorate %c_half RelaxedPrecision
// CHECK-NOT: OpDecorate %c_min10float RelaxedPrecision
// CHECK-NOT: OpDecorate %c_min16float RelaxedPrecision
// CHECK-NOT: OpDecorate %c_float16t RelaxedPrecision
// CHECK-NOT: OpDecorate %c_min16int_n3 RelaxedPrecision
// CHECK-NOT: OpDecorate %c_min16int_3 RelaxedPrecision
// CHECK-NOT: OpDecorate %c_min16uint_5 RelaxedPrecision
// CHECK-NOT: OpDecorate %c_min12int_n9 RelaxedPrecision
// CHECK-NOT: OpDecorate %c_min12int_9 RelaxedPrecision
// XXXXX-NOT: OpDecorate %c_min12uint RelaxedPrecision
// CHECK-NOT: OpDecorate %c_uint16_16 RelaxedPrecision
// CHECK-NOT: OpDecorate %c_int16_n16 RelaxedPrecision
// CHECK-NOT: OpDecorate %c_int16_16 RelaxedPrecision

void main() {
// CHECK: %half = OpTypeFloat 16

// CHECK: %half_0x1_2p_0 = OpConstant %half 0x1.2p+0
  half       c_half = 1.125;
// CHECK: %half_0x1_ep_3 = OpConstant %half 0x1.ep+3
  min10float c_min10float = 15.0;
// CHECK: %half_n0x1p_0 = OpConstant %half -0x1p+0
  min16float c_min16float = -1.0;
// CHECK: %half_0x1_8p_0 = OpConstant %half 0x1.8p+0
  float16_t  c_float16t = 1.5;

// CHECK: %short = OpTypeInt 16 1

// CHECK: %short_n3 = OpConstant %short -3
  min16int   c_min16int_n3 = -3;
// CHECK: %short_3 = OpConstant %short 3
  min16int   c_min16int_3 = 3;

// CHECK: %ushort = OpTypeInt 16 0

// CHECK: %ushort_5 = OpConstant %ushort 5
  min16uint  c_min16uint_5 = 5;

// CHECK: %short_n9 = OpConstant %short -9
  min12int   c_min12int_n9 = -9;
// CHECK: %short_9 = OpConstant %short 9
  min12int   c_min12int_9 = 9;
// It seems that min12uint is still not supported by the front-end.
// XXXXX: %short_12 = OpConstant %short 12
//  min12uint   c_min12uint = 12;

// CHECK: %ushort_16 = OpConstant %ushort 16
  uint16_t  c_uint16_16 = 16;
// CHECK: %short_n16 = OpConstant %short -16
  int16_t   c_int16_n16 = -16;
// CHECK: %short_16 = OpConstant %short 16
  int16_t   c_int16_16 = 16;
}
