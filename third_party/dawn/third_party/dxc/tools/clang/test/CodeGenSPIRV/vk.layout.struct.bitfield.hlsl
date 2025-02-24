// RUN: %dxc -T vs_6_0 -E main -HV 2021 -fcgl  %s -spirv | FileCheck %s

// CHECK:     OpMemberDecorate %S1 0 Offset 0
// CHECK-NOT: OpMemberDecorate %S1 1 Offset {{.+}}

// CHECK:     OpMemberDecorate %S2 0 Offset 0
// CHECK-NOT: OpMemberDecorate %S2 1 Offset {{.+}}

// CHECK:     OpMemberDecorate %S3 0 Offset 0
// CHECK-NOT: OpMemberDecorate %S3 1 Offset {{.+}}

// CHECK:     OpMemberDecorate %S4 0 Offset 0
// CHECK-NOT: OpMemberDecorate %S4 1 Offset {{.+}}

// CHECK:     OpMemberDecorate %S5 0 Offset 0
// CHECK:     OpMemberDecorate %S5 1 Offset 4
// CHECK-NOT: OpMemberDecorate %S5 2 Offset {{.+}}

// CHECK:     OpMemberDecorate %S6 0 Offset 0
// CHECK:     OpMemberDecorate %S6 1 Offset 4
// CHECK-NOT: OpMemberDecorate %S6 2 Offset {{.+}}

// CHECK:     OpMemberDecorate %S7 0 Offset 0
// CHECK:     OpMemberDecorate %S7 1 Offset 4
// CHECK:     OpMemberDecorate %S7 2 Offset 8
// CHECK-NOT: OpMemberDecorate %S7 3 Offset {{.+}}

// CHECK:     OpMemberDecorate %S8 0 Offset 0
// CHECK:     OpMemberDecorate %S8 1 Offset 4
// CHECK-NOT: OpMemberDecorate %S8 2 Offset {{.+}}

// CHECK:     OpMemberDecorate %S9 0 Offset 0
// CHECK:     OpMemberDecorate %S9 1 Offset 4
// CHECK-NOT: OpMemberDecorate %S9 2 Offset {{.+}}

// CHECK:     OpMemberDecorate %S10 0 Offset 0
// CHECK:     OpMemberDecorate %S10 1 Offset 4
// CHECK:     OpMemberDecorate %S10 2 Offset 8
// CHECK:     OpMemberDecorate %S10 3 Offset 12
// CHECK-NOT: OpMemberDecorate %S10 4 Offset {{.+}}

// CHECK:     OpMemberDecorate %S11 0 Offset 0
// CHECK:     OpMemberDecorate %S11 1 Offset 4
// CHECK-NOT: OpMemberDecorate %S11 2 Offset {{.+}}

// CHECK:     OpMemberDecorate %S12 0 Offset 0
// CHECK:     OpMemberDecorate %S12 1 Offset 4
// CHECK-NOT: OpMemberDecorate %S12 2 Offset {{.+}}

// CHECK:     OpMemberDecorate %S13 0 Offset 0
// CHECK:     OpMemberDecorate %S13 1 Offset 16
// CHECK-NOT: OpMemberDecorate %S13 2 Offset {{.+}}

// CHECK: OpMemberDecorate %type_buff 0 Offset 0
// CHECK: OpMemberDecorate %type_buff 1 Offset 16
// CHECK: OpMemberDecorate %type_buff 2 Offset 32
// CHECK: OpMemberDecorate %type_buff 3 Offset 48
// CHECK: OpMemberDecorate %type_buff 4 Offset 64
// CHECK: OpMemberDecorate %type_buff 5 Offset 80
// CHECK: OpMemberDecorate %type_buff 6 Offset 96
// CHECK: OpMemberDecorate %type_buff 7 Offset 112
// CHECK: OpMemberDecorate %type_buff 8 Offset 128
// CHECK: OpMemberDecorate %type_buff 9 Offset 144
// CHECK: OpMemberDecorate %type_buff 10 Offset 160
// CHECK: OpMemberDecorate %type_buff 11 Offset 176
// CHECK: OpMemberDecorate %type_buff 12 Offset 192

// CHECK:  %S1 = OpTypeStruct %uint
// CHECK:  %S2 = OpTypeStruct %uint
// CHECK:  %S3 = OpTypeStruct %uint
// CHECK:  %S4 = OpTypeStruct %uint
// CHECK:  %S5 = OpTypeStruct %uint %uint
// CHECK:  %S6 = OpTypeStruct %uint %int
// CHECK:  %S7 = OpTypeStruct %uint %uint %uint
// CHECK:  %S8 = OpTypeStruct %uint %uint
// CHECK:  %S9 = OpTypeStruct %uint %uint
// CHECK: %S10 = OpTypeStruct %uint %int %uint %int
// CHECK: %S11 = OpTypeStruct %uint %uint
// CHECK: %S12 = OpTypeStruct %uint %uint
// CHECK: %S13 = OpTypeStruct %uint %v4float
// CHECK: %S14 = OpTypeStruct %uint

// Sanity check.
struct S1 {
    uint f1 : 1;
};

// Bitfield merging.
struct S2 {
    uint f2 : 1;
    uint f1 : 1;
};

// Bitfield merging: limit.
struct S3 {
    uint f2 : 1;
    uint f1 : 31;
};
struct S4 {
    uint f2 : 31;
    uint f1 : 1;
};

// Bitfield merging: overflow.
struct S5 {
    uint f1 : 30;
    uint f2 : 3;
};

// Bitfield merging: type.
struct S6 {
    uint f2 : 1;
     int f1 : 1;
};

// Bitfield merging: mix.
struct S7 {
    uint f1;
    uint f2 : 1;
    uint f3 : 1;
    uint f4;
};
struct S8 {
    uint f1 : 1;
    uint f2 : 1;
    uint f3;
};
struct S9 {
    uint f1;
    uint f2 : 1;
    uint f3 : 1;
};
struct S10 {
    uint f2 : 1;
     int f1 : 1;
    uint f3 : 1;
     int f4 : 1;
};

// alignment.
struct S11 {
    uint f1;
    uint f2 : 1;
};
struct S12 {
    uint f2 : 1;
    uint f1;
};
struct S13 {
    uint   f1 : 1;
    float4 f2;
};

struct S14 {
    uint f1 : 1;
    uint f2 : 3;
    uint f3 : 8;
};

cbuffer buff : register(b0) {
  S1 CB_s1;
  S2 CB_s2;
  S3 CB_s3;
  S4 CB_s4;
  S5 CB_s5;
  S6 CB_s6;
  S7 CB_s7;
  S8 CB_s8;
  S9 CB_s9;
  S10 CB_s10;
  S11 CB_s11;
  S12 CB_s12;
  S13 CB_s13;
  S14 CB_s14;
}

uint main() : A {
  return 0u;
}
