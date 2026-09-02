// RUN: %dxc -T vs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

struct S {
    float4 f1;
    float3 f2;
};




// CHECK: OpMemberDecorate %type_MyCbuffer 0 Offset 0
// CHECK: OpMemberDecorate %type_MyCbuffer 1 Offset 16
// CHECK: OpMemberDecorate %type_MyCbuffer 2 Offset 48
// CHECK: OpMemberDecorate %type_MyCbuffer 3 Offset 52
// CHECK: OpMemberDecorate %type_MyCbuffer 4 Offset 64
// CHECK: OpMemberDecorate %type_MyCbuffer 5 Offset 128

cbuffer MyCbuffer : register(b1) {
    bool     CB_a;
    S        CB_s;
    int      CB_b;
    uint2    CB_c;
    float3x4 CB_d;
    float    CB_t[4];
};



// CHECK: OpMemberDecorate %type_MyTbuffer 0 Offset 0
// CHECK: OpMemberDecorate %type_MyTbuffer 1 Offset 16
// CHECK: OpMemberDecorate %type_MyTbuffer 2 Offset 48
// CHECK: OpMemberDecorate %type_MyTbuffer 3 Offset 52
// CHECK: OpMemberDecorate %type_MyTbuffer 4 Offset 64
// CHECK: OpMemberDecorate %type_MyTbuffer 5 Offset 128

tbuffer MyTbuffer : register(t1) {
    bool     TB_a;
    S        TB_s;
    int      TB_b;
    uint2    TB_c;
    float3x4 TB_d;
    float    TB_t[4];
};


float4 main() : A {
  return CB_t[0] + TB_t[0];
}
