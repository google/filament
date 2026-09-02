// RUN: %dxc -T vs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// CHECK:      OpName %type_MyTbuffer "type.MyTbuffer"
// CHECK-NEXT: OpMemberName %type_MyTbuffer 0 "a"
// CHECK-NEXT: OpMemberName %type_MyTbuffer 1 "b"
// CHECK-NEXT: OpMemberName %type_MyTbuffer 2 "c"
// CHECK-NEXT: OpMemberName %type_MyTbuffer 3 "d"
// CHECK-NEXT: OpMemberName %type_MyTbuffer 4 "s"
// CHECK-NEXT: OpMemberName %type_MyTbuffer 5 "t"

// CHECK:      OpName %MyTbuffer "MyTbuffer"

// CHECK:      OpName %type_AnotherTbuffer "type.AnotherTbuffer"
// CHECK-NEXT: OpMemberName %type_AnotherTbuffer 0 "m"
// CHECK-NEXT: OpMemberName %type_AnotherTbuffer 1 "n"

// CHECK:      OpName %AnotherTbuffer "AnotherTbuffer"

struct S {
    float  f1;
    float3 f2;
};

tbuffer MyTbuffer : register(t1) {
    bool     a;
    int      b;
    uint2    c;
    float3x4 d;
    S        s;
    float    t[4];
};

tbuffer AnotherTbuffer : register(t2) {
    float3 m;
    float4 n;
}

// CHECK: %type_MyTbuffer = OpTypeStruct %uint %int %v2uint %mat3v4float %S %_arr_float_uint_4

// CHECK: %type_AnotherTbuffer = OpTypeStruct %v3float %v4float

// CHECK: %MyTbuffer = OpVariable %_ptr_Uniform_type_MyTbuffer Uniform
// CHECK: %AnotherTbuffer = OpVariable %_ptr_Uniform_type_AnotherTbuffer Uniform

void main() {
}
