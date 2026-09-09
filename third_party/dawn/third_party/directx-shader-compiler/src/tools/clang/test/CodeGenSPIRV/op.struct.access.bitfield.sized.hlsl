// RUN: %dxc -T cs_6_2 -E main -spirv -fcgl -enable-16bit-types %s | FileCheck %s

struct S1 {
  uint64_t f1 : 32;
  uint64_t f2 : 1;
};
// CHECK-DAG: %S1 = OpTypeStruct %ulong

struct S2 {
  uint16_t f1 : 4;
  uint16_t f2 : 5;
};
// CHECK-DAG: %S2 = OpTypeStruct %ushort

struct S3 {
  uint64_t f1 : 45;
  uint64_t f2 : 10;
  uint16_t f3 : 7;
  uint32_t f4 : 5;
};
// CHECK-DAG: %S3 = OpTypeStruct %ulong %ushort %uint

struct S4 {
  int64_t f1 : 32;
  int64_t f2 : 1;
};
// CHECK-DAG: %S4 = OpTypeStruct %long

[numthreads(1, 1, 1)]
void main() {
  S1 s1;
  s1.f1 = 3;
// CHECK:   [[ptr:%[0-9]+]] = OpAccessChain %_ptr_Function_ulong %s1 %int_0
// CHECK:   [[tmp:%[0-9]+]] = OpLoad %ulong [[ptr]]
//                                                        0xffffffff00000000
// CHECK:   [[dst:%[0-9]+]] = OpBitwiseAnd %ulong [[tmp]] %ulong_18446744069414584320
// CHECK:   [[val:%[0-9]+]] = OpBitcast %ulong %ulong_3
// CHECK:   [[tmp:%[0-9]+]] = OpShiftLeftLogical %ulong [[val]] %uint_0
//                                                        0x00000000ffffffff
// CHECK:   [[src:%[0-9]+]] = OpBitwiseAnd %ulong [[tmp]] %ulong_4294967295
// CHECK:   [[mix:%[0-9]+]] = OpBitwiseOr %ulong [[dst]] [[src]]
// CHECK:                     OpStore [[ptr]] [[mix]]

  s1.f2 = 1;
// CHECK:   [[ptr:%[0-9]+]] = OpAccessChain %_ptr_Function_ulong %s1 %int_0
// CHECK:   [[tmp:%[0-9]+]] = OpLoad %ulong [[ptr]]
//                                                        0xfffffffeffffffff
// CHECK:   [[dst:%[0-9]+]] = OpBitwiseAnd %ulong [[tmp]] %ulong_18446744069414584319
// CHECK:   [[val:%[0-9]+]] = OpBitcast %ulong %ulong_1
// CHECK:   [[tmp:%[0-9]+]] = OpShiftLeftLogical %ulong [[val]] %uint_32
//                                                        0x0000000100000000
// CHECK:   [[src:%[0-9]+]] = OpBitwiseAnd %ulong [[tmp]] %ulong_4294967296
// CHECK:   [[mix:%[0-9]+]] = OpBitwiseOr %ulong [[dst]] [[src]]
// CHECK:                     OpStore [[ptr]] [[mix]]

  S2 s2;
  s2.f1 = 2;
// CHECK:   [[ptr:%[0-9]+]] = OpAccessChain %_ptr_Function_ushort %s2 %int_0
// CHECK:   [[tmp:%[0-9]+]] = OpLoad %ushort [[ptr]]
//                                                         0xfff0
// CHECK:   [[dst:%[0-9]+]] = OpBitwiseAnd %ushort [[tmp]] %ushort_65520
// CHECK:   [[val:%[0-9]+]] = OpBitcast %ushort %ushort_2
// CHECK:   [[tmp:%[0-9]+]] = OpShiftLeftLogical %ushort [[val]] %uint_0
//                                                         0x000f
// CHECK:   [[src:%[0-9]+]] = OpBitwiseAnd %ushort [[tmp]] %ushort_15
// CHECK:   [[mix:%[0-9]+]] = OpBitwiseOr %ushort [[dst]] [[src]]
// CHECK:                     OpStore [[ptr]] [[mix]]

  s2.f2 = 3;
// CHECK:   [[ptr:%[0-9]+]] = OpAccessChain %_ptr_Function_ushort %s2 %int_0
// CHECK:   [[tmp:%[0-9]+]] = OpLoad %ushort [[ptr]]
//                                                         0xfe0f
// CHECK:   [[dst:%[0-9]+]] = OpBitwiseAnd %ushort [[tmp]] %ushort_65039
// CHECK:   [[val:%[0-9]+]] = OpBitcast %ushort %ushort_3
// CHECK:   [[tmp:%[0-9]+]] = OpShiftLeftLogical %ushort [[val]] %uint_4
//                                                         0x01f0
// CHECK:   [[src:%[0-9]+]] = OpBitwiseAnd %ushort [[tmp]] %ushort_496
// CHECK:   [[mix:%[0-9]+]] = OpBitwiseOr %ushort [[dst]] [[src]]
// CHECK:                     OpStore [[ptr]] [[mix]]

  S3 s3;
  s3.f1 = 5;
// CHECK:   [[ptr:%[0-9]+]] = OpAccessChain %_ptr_Function_ulong %s3 %int_0
// CHECK:   [[tmp:%[0-9]+]] = OpLoad %ulong [[ptr]]
//                                                        0xffffe00000000000
// CHECK:   [[dst:%[0-9]+]] = OpBitwiseAnd %ulong [[tmp]] %ulong_18446708889337462784
// CHECK:   [[val:%[0-9]+]] = OpBitcast %ulong %ulong_5
// CHECK:   [[tmp:%[0-9]+]] = OpShiftLeftLogical %ulong [[val]] %uint_0
//                                                        0x00001fffffffffff
// CHECK:   [[src:%[0-9]+]] = OpBitwiseAnd %ulong [[tmp]] %ulong_35184372088831
// CHECK:   [[mix:%[0-9]+]] = OpBitwiseOr %ulong [[dst]] [[src]]
// CHECK:                     OpStore [[ptr]] [[mix]]

  s3.f2 = 6;
// CHECK:   [[ptr:%[0-9]+]] = OpAccessChain %_ptr_Function_ulong %s3 %int_0
// CHECK:   [[tmp:%[0-9]+]] = OpLoad %ulong [[ptr]]
//                                                        0xff801fffffffffff
// CHECK:   [[dst:%[0-9]+]] = OpBitwiseAnd %ulong [[tmp]] %ulong_18410750461062676479
// CHECK:   [[val:%[0-9]+]] = OpBitcast %ulong %ulong_6
// CHECK:   [[tmp:%[0-9]+]] = OpShiftLeftLogical %ulong [[val]] %uint_45
//                                                        0x007fe00000000000
// CHECK:   [[src:%[0-9]+]] = OpBitwiseAnd %ulong [[tmp]] %ulong_35993612646875136
// CHECK:   [[mix:%[0-9]+]] = OpBitwiseOr %ulong [[dst]] [[src]]
// CHECK:                     OpStore [[ptr]] [[mix]]

  s3.f3 = 7;
// CHECK:   [[ptr:%[0-9]+]] = OpAccessChain %_ptr_Function_ushort %s3 %int_1
// CHECK:   [[tmp:%[0-9]+]] = OpLoad %ushort [[ptr]]
//                                                         0xff80
// CHECK:   [[dst:%[0-9]+]] = OpBitwiseAnd %ushort [[tmp]] %ushort_65408
// CHECK:   [[val:%[0-9]+]] = OpBitcast %ushort %ushort_7
// CHECK:   [[tmp:%[0-9]+]] = OpShiftLeftLogical %ushort [[val]] %uint_0
//                                                         0x007f
// CHECK:   [[src:%[0-9]+]] = OpBitwiseAnd %ushort [[tmp]] %ushort_127
// CHECK:   [[mix:%[0-9]+]] = OpBitwiseOr %ushort [[dst]] [[src]]
// CHECK:                     OpStore [[ptr]] [[mix]]

  s3.f4 = 8;
// CHECK:   [[ptr:%[0-9]+]] = OpAccessChain %_ptr_Function_uint %s3 %int_2
// CHECK:   [[val:%[0-9]+]] = OpLoad %uint [[ptr]]
// CHECK:   [[tmp:%[0-9]+]] = OpBitFieldInsert %uint [[val]] %uint_8 %uint_0 %uint_5
// CHECK:                     OpStore [[ptr]] [[tmp]]

  S4 s4;
  s4.f1 = 3;
// CHECK:   [[ptr:%[0-9]+]] = OpAccessChain %_ptr_Function_long %s4 %int_0
// CHECK:   [[tmp:%[0-9]+]] = OpLoad %long [[ptr]]
//                                                       0xffffffff00000000
// CHECK:   [[dst:%[0-9]+]] = OpBitwiseAnd %long [[tmp]] %ulong_18446744069414584320
// CHECK:   [[val:%[0-9]+]] = OpBitcast %long %long_3
// CHECK:   [[tmp:%[0-9]+]] = OpShiftLeftLogical %long [[val]] %uint_0
//                                                       0x00000000ffffffff
// CHECK:   [[src:%[0-9]+]] = OpBitwiseAnd %long [[tmp]] %ulong_4294967295
// CHECK:   [[mix:%[0-9]+]] = OpBitwiseOr %long [[dst]] [[src]]
// CHECK:                     OpStore [[ptr]] [[mix]]

  s4.f2 = 1;
// CHECK:   [[ptr:%[0-9]+]] = OpAccessChain %_ptr_Function_long %s4 %int_0
// CHECK:   [[tmp:%[0-9]+]] = OpLoad %long [[ptr]]
//                                                       0xfffffffeffffffff
// CHECK:   [[dst:%[0-9]+]] = OpBitwiseAnd %long [[tmp]] %ulong_18446744069414584319
// CHECK:   [[val:%[0-9]+]] = OpBitcast %long %long_1
// CHECK:   [[tmp:%[0-9]+]] = OpShiftLeftLogical %long [[val]] %uint_32
//                                                       0x0000000100000000
// CHECK:   [[src:%[0-9]+]] = OpBitwiseAnd %long [[tmp]] %ulong_4294967296
// CHECK:   [[mix:%[0-9]+]] = OpBitwiseOr %long [[dst]] [[src]]
// CHECK:                     OpStore [[ptr]] [[mix]]
}
