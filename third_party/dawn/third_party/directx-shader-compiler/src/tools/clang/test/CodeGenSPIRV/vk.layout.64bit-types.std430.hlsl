// RUN: %dxc -T vs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// CHECK: OpDecorate %_arr_double_uint_3 ArrayStride 8
// CHECK: OpDecorate %_arr_mat2v3double_uint_2 ArrayStride 64
// CHECK: OpDecorate %_arr_v2long_uint_1 ArrayStride 16

// CHECK: OpMemberDecorate %S 0 Offset 0
// CHECK: OpMemberDecorate %S 1 Offset 8
// CHECK: OpMemberDecorate %S 2 Offset 16
// CHECK: OpMemberDecorate %S 3 Offset 40
// CHECK: OpMemberDecorate %S 4 Offset 64
// CHECK: OpMemberDecorate %S 5 Offset 96
// CHECK: OpMemberDecorate %S 5 MatrixStride 32
// CHECK: OpMemberDecorate %S 5 RowMajor
// CHECK: OpMemberDecorate %S 6 Offset 160
// CHECK: OpMemberDecorate %S 7 Offset 176
// CHECK: OpMemberDecorate %S 8 Offset 192
// CHECK: OpMemberDecorate %S 8 MatrixStride 32
// CHECK: OpMemberDecorate %S 8 ColMajor
// CHECK: OpMemberDecorate %S 9 Offset 320

// CHECK: OpDecorate %_runtimearr_S ArrayStride 352

struct S {                       // Alignment | Offset + Size       = Next
              float     f1;      // 0         | 0        4            4
              uint64_t  f2;      // 8         | 8        8            16
              double    f3[3];   // 8         | 16       8 * 3        40
              float     f4;      // 4         | 40       4            44
              int64_t3  f5;      // 32        | 64       8 * 3        88
              double3x2 f6;      // 32        | 96       32 * 2       160    // SPIR-V RowMajor
              double2x1 f7;      // 16        | 160      16           176
              float     f8;      // 4         | 176      4            180
    row_major double2x3 f9[2];   // 32        | 192      32 * 4       320    // SPIR-V ColMajor
              int64_t2  f10[1];  // 16        | 320      16           336
};                               // 32 (max)                          352

StructuredBuffer<S> MySBuffer;

void main() { }
