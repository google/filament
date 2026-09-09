// RUN: %dxc -T vs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

int foo(int x = 5)
{
  return x + 1;
}

void main()
{
// CHECK:      %param_var_x = OpVariable %_ptr_Function_int Function
// CHECK-NEXT: %param_var_x_0 = OpVariable %_ptr_Function_int Function
// CHECK-NEXT: %param_var_x_1 = OpVariable %_ptr_Function_int Function

// CHECK-NEXT: OpStore %param_var_x %int_5
// CHECK-NEXT: {{%[0-9]+}} = OpFunctionCall %int %foo %param_var_x
  // Call without passing arg.
  foo();

// CHECK-NEXT: OpStore %param_var_x_0 %int_2
// CHECK-NEXT: {{%[0-9]+}} = OpFunctionCall %int %foo %param_var_x_0
  // Call with passing arg.
  foo(2);

// CHECK-NEXT: OpStore %param_var_x_1 %int_5
// CHECK-NEXT: {{%[0-9]+}} = OpFunctionCall %int %foo %param_var_x_1
  // Call without passing arg again.
  foo();
}
