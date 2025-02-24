// RUN: %dxc -T vs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

struct S {
    float  f;
};

tbuffer MyTbuffer : register(t0) {
    float    a;
    float2   b;
    float3x4 c;
    S        s;
    float    t[4];
};

float main() : A {
// CHECK:      [[a:%[0-9]+]] = OpAccessChain %_ptr_Uniform_float %MyTbuffer %int_0
// CHECK-NEXT: {{%[0-9]+}} = OpLoad %float [[a]]

// CHECK:      [[b:%[0-9]+]] = OpAccessChain %_ptr_Uniform_v2float %MyTbuffer %int_1
// CHECK-NEXT: [[b0:%[0-9]+]] = OpAccessChain %_ptr_Uniform_float [[b]] %int_0
// CHECK-NEXT: {{%[0-9]+}} = OpLoad %float [[b0]]

// CHECK:      [[c:%[0-9]+]] = OpAccessChain %_ptr_Uniform_mat3v4float %MyTbuffer %int_2
// CHECK-NEXT: [[c12:%[0-9]+]] = OpAccessChain %_ptr_Uniform_float [[c]] %uint_1 %uint_2
// CHECK-NEXT: {{%[0-9]+}} = OpLoad %float [[c12]]

// CHECK:      [[s:%[0-9]+]] = OpAccessChain %_ptr_Uniform_S %MyTbuffer %int_3
// CHECK-NEXT: [[s0:%[0-9]+]] = OpAccessChain %_ptr_Uniform_float [[s]] %int_0
// CHECK-NEXT: {{%[0-9]+}} = OpLoad %float [[s0]]

// CHECK:      [[t:%[0-9]+]] = OpAccessChain %_ptr_Uniform__arr_float_uint_4 %MyTbuffer %int_4
// CHECK-NEXT: [[t3:%[0-9]+]] = OpAccessChain %_ptr_Uniform_float [[t]] %int_3
// CHECK-NEXT: {{%[0-9]+}} = OpLoad %float [[t3]]
    return a + b.x + c[1][2] + s.f + t[3];
}
