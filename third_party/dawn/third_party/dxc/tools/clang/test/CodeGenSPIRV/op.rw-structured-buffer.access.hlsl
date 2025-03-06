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


RWStructuredBuffer<T> MySbuffer;

void main(uint index: A) {
// CHECK:      [[c12:%[0-9]+]] = OpAccessChain %_ptr_Uniform_float %MySbuffer %int_0 %uint_2 %int_2 %int_2 %uint_1 %uint_2
// CHECK-NEXT: {{%[0-9]+}} = OpLoad %float [[c12]]

// CHECK:      [[s:%[0-9]+]] = OpAccessChain %_ptr_Uniform_float %MySbuffer %int_0 %uint_3 %int_3 %int_0 %int_0
// CHECK-NEXT: {{%[0-9]+}} = OpLoad %float [[s]]
    float val = MySbuffer[2].c[2][1][2] + MySbuffer[3].s[0].f;

// CHECK:       [[val:%[0-9]+]] = OpLoad %float %val
// CHECK-NEXT:  [[index:%[0-9]+]] = OpLoad %uint %index

// CHECK-NEXT:  [[base:%[0-9]+]] = OpAccessChain %_ptr_Uniform__arr_float_uint_4 %MySbuffer %int_0 [[index]] %int_4
// CHECK-NEXT:  [[t3:%[0-9]+]] = OpAccessChain %_ptr_Uniform_float [[base]] %int_3
// CHECK-NEXT:  OpStore [[t3]] [[val]]

// CHECK:       [[f:%[0-9]+]] = OpAccessChain %_ptr_Uniform_float %MySbuffer %int_0 %uint_3 %int_3 %int_0 %int_0
// CHECK-NEXT:  OpStore [[f]] [[val]]

// CHECK-NEXT:  [[c212:%[0-9]+]] = OpAccessChain %_ptr_Uniform_float %MySbuffer %int_0 %uint_2 %int_2 %int_2 %uint_1 %uint_2
// CHECK-NEXT:  OpStore [[c212]] [[val]]

// CHECK-NEXT:  [[base:%[0-9]+]] = OpAccessChain %_ptr_Uniform__arr_v2float_uint_2 %MySbuffer %int_0 %uint_1 %int_1
// CHECK-NEXT:  [[b1:%[0-9]+]] = OpAccessChain %_ptr_Uniform_v2float [[base]] %int_1
// CHECK-NEXT:  [[x:%[0-9]+]] = OpAccessChain %_ptr_Uniform_float [[b1]] %int_0
// CHECK-NEXT:  OpStore [[x]] [[val]]

// CHECK-NEXT:  [[a:%[0-9]+]] = OpAccessChain %_ptr_Uniform_float %MySbuffer %int_0 %uint_0 %int_0
// CHECK-NEXT:  OpStore [[a]] [[val]]
    MySbuffer[0].a = MySbuffer[1].b[1].x = MySbuffer[2].c[2][1][2] =
    MySbuffer[3].s[0].f = MySbuffer[index].t[3] = val;
}
