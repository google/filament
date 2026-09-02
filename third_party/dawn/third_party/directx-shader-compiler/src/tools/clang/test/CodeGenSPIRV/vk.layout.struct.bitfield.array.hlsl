// RUN: %dxc -T cs_6_0 -E main -fcgl -spirv %s | FileCheck %s

struct S1 {
  uint f1 : 1;
  uint f2 : 1;
};
RWStructuredBuffer<S1> b1;
// CHECK-DAG:                   OpDecorate %_runtimearr_S1 ArrayStride 4
// CHECK-DAG: %_runtimearr_S1 = OpTypeRuntimeArray %S1
// CHECK-DAG:             %S1 = OpTypeStruct %uint
// CHECK-DAG:                   OpMemberDecorate %S1 0 Offset 0
// CHECK-NOT-DAG:               OpMemberDecorate %S1 1

struct S2 {
  uint f1 : 31;
  uint f2 : 1;
};
RWStructuredBuffer<S2> b2;
// CHECK-DAG:                   OpDecorate %_runtimearr_S2 ArrayStride 4
// CHECK-DAG: %_runtimearr_S2 = OpTypeRuntimeArray %S2
// CHECK-DAG:             %S2 = OpTypeStruct %uint
// CHECK-DAG:                   OpMemberDecorate %S2 0 Offset 0
// CHECK-NOT-DAG:               OpMemberDecorate %S2 1

struct S3 {
  uint f1 : 1;
  uint f2 : 31;
};
RWStructuredBuffer<S3> b3;
// CHECK-DAG:                   OpDecorate %_runtimearr_S3 ArrayStride 4
// CHECK-DAG: %_runtimearr_S3 = OpTypeRuntimeArray %S3
// CHECK-DAG:             %S3 = OpTypeStruct %uint
// CHECK-DAG:                   OpMemberDecorate %S3 0 Offset 0
// CHECK-NOT-DAG:               OpMemberDecorate %S3 1

struct S4 {
  uint f1 : 1;
  uint f2 : 32;
};
RWStructuredBuffer<S4> b4;
// CHECK-DAG:                   OpDecorate %_runtimearr_S4 ArrayStride 8
// CHECK-DAG: %_runtimearr_S4 = OpTypeRuntimeArray %S4
// CHECK-DAG:             %S4 = OpTypeStruct %uint %uint
// CHECK-DAG:                   OpMemberDecorate %S4 0 Offset 0
// CHECK-DAG:                   OpMemberDecorate %S4 1 Offset 4
// CHECK-NOT-DAG:               OpMemberDecorate %S4 2

struct S5 {
  uint f1 : 1;
  int  f2 : 1;
  uint f3 : 1;
};
RWStructuredBuffer<S5> b5;
// CHECK-DAG:                   OpDecorate %_runtimearr_S5 ArrayStride 12
// CHECK-DAG: %_runtimearr_S5 = OpTypeRuntimeArray %S5
// CHECK-DAG:             %S5 = OpTypeStruct %uint %int %uint
// CHECK-DAG:                   OpMemberDecorate %S5 0 Offset 0
// CHECK-DAG:                   OpMemberDecorate %S5 1 Offset 4
// CHECK-DAG:                   OpMemberDecorate %S5 2 Offset 8
// CHECK-NOT-DAG:               OpMemberDecorate %S5 3

struct S6 {
  uint f1 : 1;
  int  f2 : 1;
  uint f3 : 16;
  uint f4 : 16;
};
RWStructuredBuffer<S6> b6;
// CHECK-DAG:                   OpDecorate %_runtimearr_S6 ArrayStride 12
// CHECK-DAG: %_runtimearr_S6 = OpTypeRuntimeArray %S6
// CHECK-DAG:             %S6 = OpTypeStruct %uint %int %uint
// CHECK-DAG:                   OpMemberDecorate %S6 0 Offset 0
// CHECK-DAG:                   OpMemberDecorate %S6 1 Offset 4
// CHECK-DAG:                   OpMemberDecorate %S6 2 Offset 8
// CHECK-NOT-DAG:               OpMemberDecorate %S6 3

struct S7 {
  uint  f1 : 1;
  float f2;
  uint  f3 : 1;
};
RWStructuredBuffer<S7> b7;
// CHECK-DAG:                   OpDecorate %_runtimearr_S7 ArrayStride 12
// CHECK-DAG: %_runtimearr_S7 = OpTypeRuntimeArray %S7
// CHECK-DAG:             %S7 = OpTypeStruct %uint %float %uint
// CHECK-DAG:                   OpMemberDecorate %S7 0 Offset 0
// CHECK-DAG:                   OpMemberDecorate %S7 1 Offset 4
// CHECK-DAG:                   OpMemberDecorate %S7 2 Offset 8
// CHECK-NOT-DAG:               OpMemberDecorate %S7 3

struct S8 {
  uint  f1 : 1;
  S1    f2;
};
RWStructuredBuffer<S8> b8;
// CHECK-DAG:                   OpDecorate %_runtimearr_S8 ArrayStride 8
// CHECK-DAG: %_runtimearr_S8 = OpTypeRuntimeArray %S8
// CHECK-DAG:             %S8 = OpTypeStruct %uint %S1
// CHECK-DAG:                   OpMemberDecorate %S8 0 Offset 0
// CHECK-DAG:                   OpMemberDecorate %S8 1 Offset 4
// CHECK-NOT-DAG:               OpMemberDecorate %S8 2

struct S9 {
  uint  f1 : 1;
  uint  f2 : 1;
  S1    f3[10];
  uint  f4 : 1;
  uint  f5 : 1;
};
RWStructuredBuffer<S9> b9;
// CHECK-DAG:                   OpDecorate %_runtimearr_S9 ArrayStride 48
// CHECK-DAG: %_runtimearr_S9 = OpTypeRuntimeArray %S9
// CHECK-DAG:             %S9 = OpTypeStruct %uint %_arr_S1_uint_10 %uint
// CHECK-DAG:                   OpMemberDecorate %S9 0 Offset 0
// CHECK-DAG:                   OpMemberDecorate %S9 1 Offset 4
// CHECK-DAG:                   OpMemberDecorate %S9 2 Offset 44
// CHECK-NOT-DAG:               OpMemberDecorate %S9 3

struct S10 : S1 {
  uint f1 : 1;
  uint f2 : 1;
};
RWStructuredBuffer<S10> b10;
// CHECK-DAG:                    OpDecorate %_runtimearr_S10 ArrayStride 8
// CHECK-DAG: %_runtimearr_S10 = OpTypeRuntimeArray %S10
// CHECK-DAG:             %S10 = OpTypeStruct %S1 %uint
// CHECK-DAG:                    OpMemberDecorate %S10 0 Offset 0
// CHECK-DAG:                    OpMemberDecorate %S10 1 Offset 4
// CHECK-NOT-DAG:                OpMemberDecorate %S10 2

struct S11 : S10 {
  uint f1 : 1;
};
RWStructuredBuffer<S11> b11;
// CHECK-DAG:                    OpDecorate %_runtimearr_S11 ArrayStride 12
// CHECK-DAG: %_runtimearr_S11 = OpTypeRuntimeArray %S11
// CHECK-DAG:             %S11 = OpTypeStruct %S10 %uint
// CHECK-DAG:                    OpMemberDecorate %S11 0 Offset 0
// CHECK-DAG:                    OpMemberDecorate %S11 1 Offset 8
// CHECK-NOT-DAG:                OpMemberDecorate %S11 2

struct S12 : S8 {
  uint f1 : 1;
};
RWStructuredBuffer<S12> b12;
// CHECK-DAG:                    OpDecorate %_runtimearr_S12 ArrayStride 12
// CHECK-DAG: %_runtimearr_S12 = OpTypeRuntimeArray %S12
// CHECK-DAG:             %S12 = OpTypeStruct %S8 %uint
// CHECK-DAG:                    OpMemberDecorate %S12 0 Offset 0
// CHECK-DAG:                    OpMemberDecorate %S12 1 Offset 8
// CHECK-NOT-DAG:                OpMemberDecorate %S12 2

[numthreads(1, 1, 1)]
void main() { }
