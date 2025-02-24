// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// The derived methods override the base struct methods.

struct A {
  float4 base;
};

struct B : A {
  float4 derived;

  float4 GetBase() {return base;}
  float4 GetDerived() {return derived;}
  void SetBase(float4 v) { base = v;}
  void SetDerived(float4 v) { derived = v; }
};

struct C : B {
  float4 c_value;

  float4 GetBase() { return base; }
  float4 GetDerived() { return derived; }
  float4 GetCValue() { return c_value; }
  void SetBase(float4 v) { base = v; }
  void SetDerived(float4 v) { derived = v; }
  void SetValue(float4 v) { c_value = v; }
};

float4 main() : SV_Target {
  B b;
// CHECK:    [[A_ptr:%[0-9]+]] = OpAccessChain %_ptr_Function_A %b %uint_0
// CHECK: [[base_ptr:%[0-9]+]] = OpAccessChain %_ptr_Function_v4float [[A_ptr]] %int_0
// CHECK:                     OpStore [[base_ptr]] {{%[0-9]+}}
  b.base = float4(1, 1, 0, 1);
// CHECK: [[derived_ptr:%[0-9]+]] = OpAccessChain %_ptr_Function_v4float %b %int_1
// CHECK:                        OpStore [[derived_ptr]] {{%[0-9]+}}
  b.derived = float4(1, 0, 1, 1);

// CHECK: OpFunctionCall %void %B_SetBase %b %param_var_v
  b.SetBase(float4(1, 0, 1, 1));
// CHECK: OpFunctionCall %void %B_SetDerived %b %param_var_v_0
  b.SetDerived(float4(1, 0, 1, 1));

  C c;
// CHECK:       [[A_ptr_0:%[0-9]+]] = OpAccessChain %_ptr_Function_A %c %uint_0 %uint_0
// CHECK:    [[base_ptr_0:%[0-9]+]] = OpAccessChain %_ptr_Function_v4float [[A_ptr_0]] %int_0
// CHECK:                        OpStore [[base_ptr_0]] {{%[0-9]+}}
  c.base = float4(0,0,0,0);
// CHECK:       [[B_ptr:%[0-9]+]] = OpAccessChain %_ptr_Function_B %c %uint_0
// CHECK: [[derived_ptr_0:%[0-9]+]] = OpAccessChain %_ptr_Function_v4float [[B_ptr]] %int_1
// CHECK:                        OpStore [[derived_ptr_0]] {{%[0-9]+}}
  c.derived = float4(0,0,0,0);
// CHECK: [[c_value_ptr:%[0-9]+]] = OpAccessChain %_ptr_Function_v4float %c %int_1
// CHECK:                        OpStore [[c_value_ptr]] {{%[0-9]+}}
  c.c_value = float4(0,0,0,0);

// CHECK: OpFunctionCall %void %C_SetBase %c %param_var_v_1
  c.SetBase(float4(0, 0, 0, 0));
// CHECK: OpFunctionCall %void %C_SetDerived %c %param_var_v_2
  c.SetDerived(float4(0, 0, 0, 0));
// CHECK: OpFunctionCall %void %C_SetValue %c %param_var_v_3
  c.SetValue(float4(0, 0, 0, 0));

// CHECK: OpFunctionCall %v4float %B_GetBase %b
// CHECK: OpFunctionCall %v4float %B_GetDerived %b
// CHECK: OpFunctionCall %v4float %C_GetBase %c
// CHECK: OpFunctionCall %v4float %C_GetDerived %c
// CHECK: OpFunctionCall %v4float %C_GetCValue %c
  return b.GetBase() + b.GetDerived() + c.GetBase() + c.GetDerived() + c.GetCValue();
}


// Definition for: void B::SetBase(float4 v) { base = v;}
// CHECK:             %B_SetBase = OpFunction
// CHECK-NEXT:       %param_this = OpFunctionParameter %_ptr_Function_B
// CHECK-NEXT:                %v = OpFunctionParameter %_ptr_Function_v4float
// CHECK-NEXT:       %bb_entry_0 = OpLabel
// CHECK-NEXT:        [[v:%[0-9]+]] = OpLoad %v4float %v
// CHECK-NEXT:    [[A_ptr_1:%[0-9]+]] = OpAccessChain %_ptr_Function_A %param_this %uint_0
// CHECK-NEXT: [[base_ptr_1:%[0-9]+]] = OpAccessChain %_ptr_Function_v4float [[A_ptr_1]] %int_0
// CHECK-NEXT:                     OpStore [[base_ptr_1]] [[v]]
// CHECK-NEXT:                     OpReturn
// CHECK-NEXT:                     OpFunctionEnd

// Definition for: void B::SetDerived(float4 v) { derived = v; }
// CHECK:             %B_SetDerived = OpFunction
// CHECK-NEXT:        %param_this_0 = OpFunctionParameter %_ptr_Function_B
// CHECK-NEXT:                 %v_0 = OpFunctionParameter %_ptr_Function_v4float
// CHECK-NEXT:          %bb_entry_1 = OpLabel
// CHECK-NEXT:           [[v_0:%[0-9]+]] = OpLoad %v4float %v_0
// CHECK-NEXT: [[derived_ptr_1:%[0-9]+]] = OpAccessChain %_ptr_Function_v4float %param_this_0 %int_1
// CHECK-NEXT:                        OpStore [[derived_ptr_1]] [[v_0]]
// CHECK-NEXT:                        OpReturn
// CHECK-NEXT:                        OpFunctionEnd


// Definition for: void C::SetBase(float4 v) { base = v; }
// CHECK:             %C_SetBase = OpFunction
// CHECK-NEXT:     %param_this_1 = OpFunctionParameter %_ptr_Function_C
// CHECK-NEXT:              %v_1 = OpFunctionParameter %_ptr_Function_v4float
// CHECK-NEXT:       %bb_entry_2 = OpLabel
// CHECK-NEXT:        [[v_1:%[0-9]+]] = OpLoad %v4float %v_1
// CHECK-NEXT:    [[A_ptr_2:%[0-9]+]] = OpAccessChain %_ptr_Function_A %param_this_1 %uint_0 %uint_0
// CHECK-NEXT: [[base_ptr_2:%[0-9]+]] = OpAccessChain %_ptr_Function_v4float [[A_ptr_2]] %int_0
// CHECK-NEXT:                     OpStore [[base_ptr_2]] [[v_1]]
// CHECK-NEXT:                     OpReturn
// CHECK-NEXT:                     OpFunctionEnd

// Definition for: void C::SetDerived(float4 v) { derived = v; }
// CHECK:             %C_SetDerived = OpFunction
// CHECK-NEXT:        %param_this_2 = OpFunctionParameter %_ptr_Function_C
// CHECK-NEXT:                 %v_2 = OpFunctionParameter %_ptr_Function_v4float
// CHECK-NEXT:          %bb_entry_3 = OpLabel
// CHECK-NEXT:           [[v_2:%[0-9]+]] = OpLoad %v4float %v_2
// CHECK-NEXT:       [[B_ptr_0:%[0-9]+]] = OpAccessChain %_ptr_Function_B %param_this_2 %uint_0
// CHECK-NEXT: [[derived_ptr_2:%[0-9]+]] = OpAccessChain %_ptr_Function_v4float [[B_ptr_0]] %int_1
// CHECK-NEXT:                        OpStore [[derived_ptr_2]] [[v_2]]
// CHECK-NEXT:                        OpReturn
// CHECK-NEXT:                        OpFunctionEnd

// Definition for: void C::SetValue(float4 v) { c_value = v; }
// CHECK:               %C_SetValue = OpFunction
// CHECK-NEXT:        %param_this_3 = OpFunctionParameter %_ptr_Function_C
// CHECK-NEXT:                 %v_3 = OpFunctionParameter %_ptr_Function_v4float
// CHECK-NEXT:          %bb_entry_4 = OpLabel
// CHECK-NEXT:           [[v_3:%[0-9]+]] = OpLoad %v4float %v_3
// CHECK-NEXT: [[c_value_ptr_0:%[0-9]+]] = OpAccessChain %_ptr_Function_v4float %param_this_3 %int_1
// CHECK-NEXT:                        OpStore [[c_value_ptr_1:%[0-9]+]]
// CHECK-NEXT:                        OpReturn
// CHECK-NEXT:                        OpFunctionEnd

// Definition for: float4 B::GetBase() {return base;}
// CHECK:             %B_GetBase = OpFunction
// CHECK-NEXT:     %param_this_4 = OpFunctionParameter %_ptr_Function_B
// CHECK-NEXT:       %bb_entry_5 = OpLabel
// CHECK-NEXT:    [[A_ptr_3:%[0-9]+]] = OpAccessChain %_ptr_Function_A %param_this_4 %uint_0
// CHECK-NEXT: [[base_ptr_3:%[0-9]+]] = OpAccessChain %_ptr_Function_v4float [[A_ptr_3]] %int_0
// CHECK-NEXT:     [[base:%[0-9]+]] = OpLoad %v4float [[base_ptr_3]]
// CHECK-NEXT:                     OpReturnValue [[base]]
// CHECK-NEXT:                     OpFunctionEnd

// Definition for: float4 B::GetDerived() {return derived;}
// CHECK:             %B_GetDerived = OpFunction
// CHECK-NEXT:        %param_this_5 = OpFunctionParameter %_ptr_Function_B
// CHECK-NEXT:          %bb_entry_6 = OpLabel
// CHECK-NEXT: [[derived_ptr_3:%[0-9]+]] = OpAccessChain %_ptr_Function_v4float %param_this_5 %int_1
// CHECK-NEXT:     [[derived:%[0-9]+]] = OpLoad %v4float [[derived_ptr_3]]
// CHECK-NEXT:                        OpReturnValue [[derived]]
// CHECK-NEXT:                        OpFunctionEnd

// Definition for: float4 C::GetBase() {return base;}
// CHECK:             %C_GetBase = OpFunction
// CHECK-NEXT:     %param_this_6 = OpFunctionParameter %_ptr_Function_C
// CHECK-NEXT:       %bb_entry_7 = OpLabel
// CHECK-NEXT:    [[A_ptr_4:%[0-9]+]] = OpAccessChain %_ptr_Function_A %param_this_6 %uint_0 %uint_0
// CHECK-NEXT: [[base_ptr_4:%[0-9]+]] = OpAccessChain %_ptr_Function_v4float [[A_ptr_4]] %int_0
// CHECK-NEXT:     [[base_0:%[0-9]+]] = OpLoad %v4float [[base_ptr_4]]
// CHECK-NEXT:                     OpReturnValue [[base_0]]
// CHECK-NEXT:                     OpFunctionEnd

// Definition for: float4 C::GetDerived() {return derived;}
// CHECK:             %C_GetDerived = OpFunction
// CHECK-NEXT:        %param_this_7 = OpFunctionParameter %_ptr_Function_C
// CHECK-NEXT:          %bb_entry_8 = OpLabel
// CHECK-NEXT:       [[B_ptr_1:%[0-9]+]] = OpAccessChain %_ptr_Function_B %param_this_7 %uint_0
// CHECK-NEXT: [[derived_ptr_4:%[0-9]+]] = OpAccessChain %_ptr_Function_v4float [[B_ptr_1]] %int_1
// CHECK-NEXT:     [[derived_0:%[0-9]+]] = OpLoad %v4float [[derived_ptr_4]]
// CHECK-NEXT:                        OpReturnValue [[derived_0]]
// CHECK-NEXT:                        OpFunctionEnd

// CHECK:              %C_GetCValue = OpFunction
// CHECK-NEXT:        %param_this_8 = OpFunctionParameter %_ptr_Function_C
// CHECK-NEXT:          %bb_entry_9 = OpLabel
// CHECK-NEXT: [[c_value_ptr_2:%[0-9]+]] = OpAccessChain %_ptr_Function_v4float %param_this_8 %int_1
// CHECK-NEXT:     [[c_value:%[0-9]+]] = OpLoad %v4float [[c_value_ptr_2]]
// CHECK-NEXT:                        OpReturnValue [[c_value]]
// CHECK-NEXT:                        OpFunctionEnd
