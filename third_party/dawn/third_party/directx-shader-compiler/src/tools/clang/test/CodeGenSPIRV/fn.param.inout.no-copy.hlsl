// RUN: %dxc -T vs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

struct S {
    float4 val;
};

void foo(
    out   int      a,
    inout uint2    b,
    out   float2x3 c,
    inout S        d,
    out   float    e[4]
) {
    a = 0;
    b = 1;
    c = 2.0;
    d.val = 3.0;
    e[0] = 4.0;
}

void main() {
    int      a;
    uint2    b;
    float2x3 c;
    S        d;
    float    e[4];

// CHECK: %a = OpVariable %_ptr_Function_int Function
// CHECK: %b = OpVariable %_ptr_Function_v2uint Function
// CHECK: %c = OpVariable %_ptr_Function_mat2v3float Function
// CHECK: %d = OpVariable %_ptr_Function_S Function
// CHECK: %e = OpVariable %_ptr_Function__arr_float_uint_4 Function

// CHECK:      OpFunctionCall %void %foo %a %b %c %d %e

    foo(a, b, c, d, e);
}
