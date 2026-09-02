// RUN: %dxc -T cs_6_2 -E main -spirv -fcgl -enable-16bit-types %s | FileCheck %s

struct S1 {
  uint64_t f1 : 32;
  uint64_t f2 : 1;
};
// CHECK-DAG: %S1 = OpTypeStruct %ulong

struct S2 {
  int64_t f1 : 32;
  int64_t f2 : 1;
};
// CHECK-DAG: %S2 = OpTypeStruct %long

struct S3 {
  uint64_t f1 : 45;
  uint64_t f2 : 10;
  uint16_t f3 : 7;
  uint32_t f4 : 5;
};
// CHECK-DAG: %S3 = OpTypeStruct %ulong %ushort %uint

[numthreads(1, 1, 1)]
void main() {
  uint64_t vulong;
  uint32_t vuint;
  uint16_t vushort;
  int64_t vlong;

  S1 s1;
  vulong = s1.f1;
// CHECK: [[ptr:%[0-9]+]] = OpAccessChain %_ptr_Function_ulong %s1 %int_0
// CHECK: [[raw:%[0-9]+]] = OpLoad %ulong [[ptr]]
// CHECK: [[tmp:%[0-9]+]] = OpShiftLeftLogical %ulong [[raw]] %uint_32
// CHECK: [[out:%[0-9]+]] = OpShiftRightLogical %ulong [[tmp]] %uint_32
// CHECK:                   OpStore %vulong [[out]]
  vulong = s1.f2;
// CHECK: [[ptr:%[0-9]+]] = OpAccessChain %_ptr_Function_ulong %s1 %int_0
// CHECK: [[raw:%[0-9]+]] = OpLoad %ulong [[ptr]]
// CHECK: [[tmp:%[0-9]+]] = OpShiftLeftLogical %ulong [[raw]] %uint_31
// CHECK: [[out:%[0-9]+]] = OpShiftRightLogical %ulong [[tmp]] %uint_63
// CHECK:                   OpStore %vulong [[out]]

  S2 s2;
  vlong = s2.f1;
// CHECK: [[ptr:%[0-9]+]] = OpAccessChain %_ptr_Function_long %s2 %int_0
// CHECK: [[raw:%[0-9]+]] = OpLoad %long [[ptr]]
// CHECK: [[tmp:%[0-9]+]] = OpShiftLeftLogical %long [[raw]] %uint_32
// CHECK: [[out:%[0-9]+]] = OpShiftRightArithmetic %long [[tmp]] %uint_32
// CHECK:                   OpStore %vlong [[out]]
  vlong = s2.f2;
// CHECK: [[ptr:%[0-9]+]] = OpAccessChain %_ptr_Function_long %s2 %int_0
// CHECK: [[raw:%[0-9]+]] = OpLoad %long [[ptr]]
// CHECK: [[tmp:%[0-9]+]] = OpShiftLeftLogical %long [[raw]] %uint_31
// CHECK: [[out:%[0-9]+]] = OpShiftRightArithmetic %long [[tmp]] %uint_63
// CHECK:                   OpStore %vlong [[out]]

  S3 s3;
  vulong = s3.f1;
// CHECK: [[ptr:%[0-9]+]] = OpAccessChain %_ptr_Function_ulong %s3 %int_0
// CHECK: [[raw:%[0-9]+]] = OpLoad %ulong [[ptr]]
// CHECK: [[tmp:%[0-9]+]] = OpShiftLeftLogical %ulong [[raw]] %uint_19
// CHECK: [[out:%[0-9]+]] = OpShiftRightLogical %ulong [[tmp]] %uint_19
// CHECK:                   OpStore %vulong [[out]]
  vulong = s3.f2;
// CHECK: [[ptr:%[0-9]+]] = OpAccessChain %_ptr_Function_ulong %s3 %int_0
// CHECK: [[raw:%[0-9]+]] = OpLoad %ulong [[ptr]]
// CHECK: [[tmp:%[0-9]+]] = OpShiftLeftLogical %ulong [[raw]] %uint_9
// CHECK: [[out:%[0-9]+]] = OpShiftRightLogical %ulong [[tmp]] %uint_54
// CHECK:                   OpStore %vulong [[out]]

  vushort = s3.f3;
// CHECK: [[ptr:%[0-9]+]] = OpAccessChain %_ptr_Function_ushort %s3 %int_1
// CHECK: [[raw:%[0-9]+]] = OpLoad %ushort [[ptr]]
// CHECK: [[tmp:%[0-9]+]] = OpShiftLeftLogical %ushort [[raw]] %uint_9
// CHECK: [[out:%[0-9]+]] = OpShiftRightLogical %ushort [[tmp]] %uint_9
// CHECK:                   OpStore %vushort [[out]]

  vuint = s3.f4;
// CHECK: [[ptr:%[0-9]+]] = OpAccessChain %_ptr_Function_uint %s3 %int_2
// CHECK: [[raw:%[0-9]+]] = OpLoad %uint [[ptr]]
// CHECK: [[tmp:%[0-9]+]] = OpBitFieldUExtract %uint [[raw]] %uint_0 %uint_5
// CHECK:                   OpStore %vuint [[tmp]]
}
