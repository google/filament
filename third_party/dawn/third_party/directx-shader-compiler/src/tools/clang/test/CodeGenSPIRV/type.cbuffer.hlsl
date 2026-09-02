// RUN: %dxc -T vs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// CHECK:      OpName %type_MyCbuffer "type.MyCbuffer"
// CHECK-NEXT: OpMemberName %type_MyCbuffer 0 "a"
// CHECK-NEXT: OpMemberName %type_MyCbuffer 1 "b"
// CHECK-NEXT: OpMemberName %type_MyCbuffer 2 "c"
// CHECK-NEXT: OpMemberName %type_MyCbuffer 3 "d"
// CHECK-NEXT: OpMemberName %type_MyCbuffer 4 "s"
// CHECK-NEXT: OpMemberName %type_MyCbuffer 5 "t"

// CHECK:      OpName %MyCbuffer "MyCbuffer"

// CHECK:      OpName %type_AnotherCBuffer "type.AnotherCBuffer"
// CHECK-NEXT: OpMemberName %type_AnotherCBuffer 0 "m"
// CHECK-NEXT: OpMemberName %type_AnotherCBuffer 1 "n"

// CHECK:      OpName %AnotherCBuffer "AnotherCBuffer"

struct S {
    float  f1;
    float3 f2;
};

cbuffer MyCbuffer : register(b1) {
    bool     a;
    int      b;
    uint2    c;
    float3x4 d;
    S        s;
    float    t[4];
};

cbuffer AnotherCBuffer : register(b2) {
    float3 m;
    float4 n;
}

// CHECK: %type_MyCbuffer = OpTypeStruct %uint %int %v2uint %mat3v4float %S %_arr_float_uint_4

// CHECK: %type_AnotherCBuffer = OpTypeStruct %v3float %v4float

// CHECK: %MyCbuffer = OpVariable %_ptr_Uniform_type_MyCbuffer Uniform
// CHECK: %AnotherCBuffer = OpVariable %_ptr_Uniform_type_AnotherCBuffer Uniform

float  main() : A {
  return t[0] + m[0];
}
