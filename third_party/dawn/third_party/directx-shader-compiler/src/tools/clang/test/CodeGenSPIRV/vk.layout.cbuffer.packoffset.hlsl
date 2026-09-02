// RUN: %dxc -T vs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// CHECK: OpMemberDecorate %type_MyCBuffer 0 Offset 0
// CHECK: OpMemberDecorate %type_MyCBuffer 1 Offset 32
// CHECK: OpMemberDecorate %type_MyCBuffer 2 Offset 52
// CHECK: OpMemberDecorate %type_MyCBuffer 3 Offset 56
// CHECK: OpMemberDecorate %type_MyCBuffer 4 Offset 60
// CHECK: OpMemberDecorate %type_MyCBuffer 5 Offset 1600
// CHECK: OpMemberDecorate %type_MyCBuffer 6 Offset 1760
// CHECK: OpMemberDecorate %type_MyCBuffer 7 Offset 2400

struct S {
    float4 f;
};

cbuffer MyCBuffer {                    // Offset
    float4   data1 : packoffset(c0);   //                  0
    float4   data2 : packoffset(c2);   // 2 * 16         = 32
    float    data3 : packoffset(c3.y); // 3 * 16 + 1 * 4 = 52
    float    data4 : packoffset(c3.z); // 3 * 16 + 2 * 4 = 56
    float    data5 : packoffset(c3.w); //                  60
    float4   data6 : packoffset(c100); // 100 * 16       = 1600
    float2x3 data7 : packoffset(c110); // 110 * 16       = 1760
    S        data8 : packoffset(c150); // 150 * 16       = 2400
}

float4 main() : A {
    return data1;
}
