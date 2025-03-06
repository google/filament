// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s

struct S {
    float  f;
};

struct T {
    float    a;
    float2   b[2];
    float3x4 c[3];
    S        s[2];
    float    t[4];
};


StructuredBuffer<T> MySbuffer;

float4 main(uint index: A) : SV_Target {
// CHECK:      [[a:%[0-9]+]] = OpAccessChain %_ptr_Uniform_float %MySbuffer %int_0 %uint_0 %int_0
// CHECK-NEXT: {{%[0-9]+}} = OpLoad %float [[a]]

// CHECK:      [[base:%[0-9]+]] = OpAccessChain %_ptr_Uniform__arr_v2float_uint_2 %MySbuffer %int_0 %uint_1 %int_1
// CHECK:      [[b1:%[0-9]+]] = OpAccessChain %_ptr_Uniform_v2float [[base]] %int_1
// CHECK-NEXT: [[x:%[0-9]+]] = OpAccessChain %_ptr_Uniform_float [[b1]] %int_0
// CHECK-NEXT: {{%[0-9]+}} = OpLoad %float [[x]]

// CHECK:      [[c12:%[0-9]+]] = OpAccessChain %_ptr_Uniform_float %MySbuffer %int_0 %uint_2 %int_2 %int_2 %uint_1 %uint_2
// CHECK-NEXT: {{%[0-9]+}} = OpLoad %float [[c12]]

// CHECK:      [[s:%[0-9]+]] = OpAccessChain %_ptr_Uniform_float %MySbuffer %int_0 %uint_3 %int_3 %int_0 %int_0
// CHECK-NEXT: {{%[0-9]+}} = OpLoad %float [[s]]

// CHECK:      [[index:%[0-9]+]] = OpLoad %uint %index
// CHECK-NEXT: [[base:%[0-9]+]] = OpAccessChain %_ptr_Uniform__arr_float_uint_4 %MySbuffer %int_0 [[index]] %int_4
// CHECK-NEXT: [[t:%[0-9]+]] = OpAccessChain %_ptr_Uniform_float [[base]] %int_3
// CHECK-NEXT: {{%[0-9]+}} = OpLoad %float [[t]]
    return MySbuffer[0].a + MySbuffer[1].b[1].x + MySbuffer[2].c[2][1][2] +
           MySbuffer[3].s[0].f + MySbuffer[index].t[3];
}
