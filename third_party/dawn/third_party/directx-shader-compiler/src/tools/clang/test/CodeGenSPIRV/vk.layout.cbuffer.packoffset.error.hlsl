// RUN: not %dxc -T vs_6_0 -E main -fcgl  %s -spirv  2>&1 | FileCheck %s

cbuffer MyCBuffer {
    float4 data1 : packoffset(c0);
    float4 data2 : packoffset(c2);
    float  data3 : packoffset(c0);    // error: overlap
    float  data4 : packoffset(c10.z);
    float  data5 : packoffset(c10.z); // error: overlap
}

float4 main() : A {
    return data1;
}

// CHECK: :6:20: error: packoffset caused overlap with previous members
// CHECK: :8:20: error: packoffset caused overlap with previous members
