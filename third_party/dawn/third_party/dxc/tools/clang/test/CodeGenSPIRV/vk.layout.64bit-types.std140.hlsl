// RUN: %dxc -T vs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// CHECK: OpDecorate %_arr_double_uint_3 ArrayStride 16
// CHECK: OpDecorate %_arr_mat2v3double_uint_2 ArrayStride 64
// CHECK: OpDecorate %_arr_v2long_uint_1 ArrayStride 16

// CHECK: OpMemberDecorate %type_MyCBuffer 0 Offset 0
// CHECK: OpMemberDecorate %type_MyCBuffer 1 Offset 8
// CHECK: OpMemberDecorate %type_MyCBuffer 2 Offset 16
// CHECK: OpMemberDecorate %type_MyCBuffer 3 Offset 64
// CHECK: OpMemberDecorate %type_MyCBuffer 4 Offset 96
// CHECK: OpMemberDecorate %type_MyCBuffer 5 Offset 128
// CHECK: OpMemberDecorate %type_MyCBuffer 5 MatrixStride 32
// CHECK: OpMemberDecorate %type_MyCBuffer 5 RowMajor
// CHECK: OpMemberDecorate %type_MyCBuffer 6 Offset 192
// CHECK: OpMemberDecorate %type_MyCBuffer 7 Offset 208
// CHECK: OpMemberDecorate %type_MyCBuffer 8 Offset 224
// CHECK: OpMemberDecorate %type_MyCBuffer 8 MatrixStride 32
// CHECK: OpMemberDecorate %type_MyCBuffer 8 ColMajor
// CHECK: OpMemberDecorate %type_MyCBuffer 9 Offset 352


cbuffer MyCBuffer{               // Alignment | Offset + Size                 = Next
              float     f1;      // 0         | 0        4                      4
              uint64_t  f2;      // 8         | 8        8                      16
              double    f3[3];   // 16        | 16       16 (stride) * 3        64
              float     f4;      // 4         | 64       4                      68
              int64_t3  f5;      // 32        | 96       8 * 3                  120
              double3x2 f6;      // 32        | 128      32 * 2                 192    // SPIR-V RowMajor
              double2x1 f7;      // 16        | 192      16                     208
              float     f8;      // 4         | 208      4                      212
    row_major double2x3 f9[2];   // 32        | 224      32 * 4                 352    // SPIR-V ColMajor
              int64_t2  f10[1];  // 16        | 352      16 (stride)            368
};                               // 32 (max)                                    384

void main() { }
