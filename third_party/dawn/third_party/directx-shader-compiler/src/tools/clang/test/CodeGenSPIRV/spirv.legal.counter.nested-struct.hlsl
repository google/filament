// RUN: %dxc -T vs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// CHECK: %counter_var_rw = OpVariable %_ptr_Uniform_type_ACSBuffer_counter Uniform
// CHECK: %counter_var_t_1_0 = OpVariable %_ptr_Private__ptr_Uniform_type_ACSBuffer_counter Private
// CHECK: %counter_var_s_0 = OpVariable %_ptr_Private__ptr_Uniform_type_ACSBuffer_counter Private
  
RWStructuredBuffer<uint> rw : register(u0); 

struct S {
  RWStructuredBuffer<uint> rw; 
  uint u; 
}; 

struct T { 
  float a; 
  S s; 
};

void foo(S s) { s.rw[0] = 0; }

float4 main() : SV_POSITION { 
  T t;
// CHECK: OpStore %counter_var_t_1_0 %counter_var_rw
  t.s.rw = rw; 

// CHECK: [[var:%[0-9]+]] = OpLoad %_ptr_Uniform_type_ACSBuffer_counter %counter_var_t_1_0
// CHECK: OpStore %counter_var_s_0 [[var]]
// CHECK: OpFunctionCall
  foo(t.s);

  return float4(1, 1, 1, 1);
} 
