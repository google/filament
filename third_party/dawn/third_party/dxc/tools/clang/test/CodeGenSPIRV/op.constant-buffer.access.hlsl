// RUN: %dxc -T vs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

struct S {
    float  f;
};

struct T {
    float    a;
    float2   b;
    float3x4 c;
    S        s;
    float    t[4];
};


ConstantBuffer<T> MyCbuffer : register(b1);
ConstantBuffer<T> MyCbufferArray[5] : register(b2);

float main() : A{
  // CHECK:      [[a:%[0-9]+]] = OpAccessChain %_ptr_Uniform_float %MyCbuffer %int_0
  // CHECK-NEXT: {{%[0-9]+}} = OpLoad %float [[a]]

  // CHECK:      [[b:%[0-9]+]] = OpAccessChain %_ptr_Uniform_v2float %MyCbuffer %int_1
  // CHECK-NEXT: [[b0:%[0-9]+]] = OpAccessChain %_ptr_Uniform_float [[b]] %int_0
  // CHECK-NEXT: {{%[0-9]+}} = OpLoad %float [[b0]]

  // CHECK:      [[c12:%[0-9]+]] = OpAccessChain %_ptr_Uniform_float %MyCbuffer %int_2 %uint_1 %uint_2
  // CHECK-NEXT: {{%[0-9]+}} = OpLoad %float [[c12]]

  // CHECK:      [[s:%[0-9]+]] = OpAccessChain %_ptr_Uniform_float %MyCbuffer %int_3 %int_0
  // CHECK-NEXT: {{%[0-9]+}} = OpLoad %float [[s]]

  // CHECK:      [[base:%[0-9]+]] = OpAccessChain %_ptr_Uniform__arr_float_uint_4 %MyCbuffer %int_4
  // CHECK:      [[t:%[0-9]+]] = OpAccessChain %_ptr_Uniform_float [[base]] %int_3
  // CHECK-NEXT: {{%[0-9]+}} = OpLoad %float [[t]]
  return MyCbuffer.a + MyCbuffer.b.x + MyCbuffer.c[1][2] + MyCbuffer.s.f + MyCbuffer.t[3] +
  // CHECK:      [[a_0:%[0-9]+]] = OpAccessChain %_ptr_Uniform_float %MyCbufferArray %int_4 %int_0
  // CHECK-NEXT: {{%[0-9]+}} = OpLoad %float [[a_0]]

  // CHECK:      [[b_0:%[0-9]+]] = OpAccessChain %_ptr_Uniform_v2float %MyCbufferArray %int_3 %int_1
  // CHECK-NEXT: [[b0_0:%[0-9]+]] = OpAccessChain %_ptr_Uniform_float [[b_0]] %int_0
  // CHECK-NEXT: {{%[0-9]+}} = OpLoad %float [[b0_0]]

  // CHECK:      [[c12_0:%[0-9]+]] = OpAccessChain %_ptr_Uniform_float %MyCbufferArray %int_2 %int_2 %uint_1 %uint_2
  // CHECK-NEXT: {{%[0-9]+}} = OpLoad %float [[c12_0]]

  // CHECK:      [[s_0:%[0-9]+]] = OpAccessChain %_ptr_Uniform_float %MyCbufferArray %int_1 %int_3 %int_0
  // CHECK-NEXT: {{%[0-9]+}} = OpLoad %float [[s_0]]

  // CHECK:      [[base:%[0-9]+]] = OpAccessChain %_ptr_Uniform__arr_float_uint_4 %MyCbufferArray %int_0 %int_4
  // CHECK:      [[t_0:%[0-9]+]] = OpAccessChain %_ptr_Uniform_float [[base]] %int_3
  // CHECK-NEXT: {{%[0-9]+}} = OpLoad %float [[t_0]]
         MyCbufferArray[4].a + MyCbufferArray[3].b.x + MyCbufferArray[2].c[1][2] + MyCbufferArray[1].s.f + MyCbufferArray[0].t[3];

}
