// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// The derived struct methods do not override the base struct methods.

struct A {
  float4 base;
  void SetBase(float4 v) { base = v; }
  float4 GetBase() { return base; }
};

struct B : A {
  float4 derived;
  float4 GetDerived() { return derived; }
  void SetDerived(float4 v) { derived = v; }
};

struct C : B {
  float4 c_value;
  float4 GetCValue() { return c_value; }
  void SetValue(float4 v) { c_value = v; }
};

float4 main() : SV_Target {
  C c;

// CHECK:  [[v4f0:%[0-9]+]] = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
// CHECK: [[A_ptr:%[0-9]+]] = OpAccessChain %_ptr_Function_A %c %uint_0 %uint_0
// CHECK:                  OpStore %param_var_v [[v4f0]]
// CHECK:                  OpFunctionCall %void %A_SetBase [[A_ptr]] %param_var_v
  c.SetBase(float4(0, 0, 0, 0));

// CHECK: [[B_ptr:%[0-9]+]] = OpAccessChain %_ptr_Function_B %c %uint_0
// CHECK:                  OpStore %param_var_v_0 [[v4f0]]
// CHECK:                  OpFunctionCall %void %B_SetDerived [[B_ptr]] %param_var_v_0
  c.SetDerived(float4(0, 0, 0, 0));

// CHECK:                  OpStore %param_var_v_1 [[v4f0]]
// CHECK:                  OpFunctionCall %void %C_SetValue %c %param_var_v_1
  c.SetValue(float4(0, 0, 0, 0));

  return
// CHECK: [[A_ptr_0:%[0-9]+]] = OpAccessChain %_ptr_Function_A %c %uint_0 %uint_0
// CHECK:       {{%[0-9]+}} = OpFunctionCall %v4float %A_GetBase [[A_ptr_0]]
    c.GetBase() +
// CHECK: [[B_ptr_0:%[0-9]+]] = OpAccessChain %_ptr_Function_B %c %uint_0
// CHECK:       {{%[0-9]+}} = OpFunctionCall %v4float %B_GetDerived [[B_ptr_0]]
    c.GetDerived() +
// CHECK:       {{%[0-9]+}} = OpFunctionCall %v4float %C_GetCValue %c
    c.GetCValue();
}

// Definition for: void A::SetBase(float4 v) { base = v;}
//
// CHECK:        %A_SetBase = OpFunction
// CHECK:       %param_this = OpFunctionParameter %_ptr_Function_A
// CHECK:                %v = OpFunctionParameter %_ptr_Function_v4float
// CHECK:        [[v:%[0-9]+]] = OpLoad %v4float %v
// CHECK: [[base_ptr:%[0-9]+]] = OpAccessChain %_ptr_Function_v4float %param_this %int_0
// CHECK:                     OpStore [[base_ptr]] [[v]]

// Definition for: void B::SetDerived(float4 v) { derived = v; }
//
// CHECK:        %B_SetDerived = OpFunction
// CHECK:        %param_this_0 = OpFunctionParameter %_ptr_Function_B
// CHECK:                 %v_0 = OpFunctionParameter %_ptr_Function_v4float
// CHECK:           [[v_0:%[0-9]+]] = OpLoad %v4float %v_0
// CHECK: [[derived_ptr:%[0-9]+]] = OpAccessChain %_ptr_Function_v4float %param_this_0 %int_1
// CHECK:                        OpStore [[derived_ptr]] [[v_0]]

// Definition for: void C::SetValue(float4 v) { c_value = v; }
//
// CHECK:        %C_SetValue = OpFunction
// CHECK:      %param_this_1 = OpFunctionParameter %_ptr_Function_C
// CHECK:               %v_1 = OpFunctionParameter %_ptr_Function_v4float
// CHECK:         [[v_1:%[0-9]+]] = OpLoad %v4float %v_1
// CHECK: [[c_val_ptr:%[0-9]+]] = OpAccessChain %_ptr_Function_v4float %param_this_1 %int_1
// CHECK:                      OpStore [[c_val_ptr]] [[v_1]]

// Definition for A::float4 GetBase() { return base; }
//
// CHECK:        %A_GetBase = OpFunction
// CHECK:     %param_this_2 = OpFunctionParameter %_ptr_Function_A
// CHECK: [[base_ptr_0:%[0-9]+]] = OpAccessChain %_ptr_Function_v4float %param_this_2 %int_0
// CHECK:                     OpLoad %v4float [[base_ptr_0]]


// Definition for B::float4 GetDerived() { return derived; }
//
// CHECK:        %B_GetDerived = OpFunction
// CHECK:        %param_this_3 = OpFunctionParameter %_ptr_Function_B
// CHECK: [[derived_ptr_0:%[0-9]+]] = OpAccessChain %_ptr_Function_v4float %param_this_3 %int_1
// CHECK:                        OpLoad %v4float [[derived_ptr_0]]

// Definition for C::float4 GetCValue() { return c_value; }
//
// CHECK:       %C_GetCValue = OpFunction
// CHECK:      %param_this_4 = OpFunctionParameter %_ptr_Function_C
// CHECK: [[c_val_ptr_0:%[0-9]+]] = OpAccessChain %_ptr_Function_v4float %param_this_4 %int_1
// CHECK:                      OpLoad %v4float [[c_val_ptr_0]]
