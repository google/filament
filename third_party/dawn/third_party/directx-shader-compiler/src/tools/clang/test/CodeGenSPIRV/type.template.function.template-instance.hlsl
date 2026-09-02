// RUN: %dxc -T ps_6_0 -E main -HV 2021 -fcgl  %s -spirv | FileCheck %s

template <typename T>
T square(T number) {
  return number * number;
}

template <typename T>
T plusTwo(int n) {
  return 2 + n;
}

template <typename T>
int triple(T x) {
  return 3 * x;
}

void main() {
  int x;
  float y;

// CHECK: [[fn_param_int_ret_int:%[a-zA-Z0-9_]+]] = OpTypeFunction %int %_ptr_Function_int
// CHECK: [[fn_param_float_ret_float:%[a-zA-Z0-9_]+]] = OpTypeFunction %float %_ptr_Function_float
// CHECK: [[fn_param_double_ret_double:%[a-zA-Z0-9_]+]] = OpTypeFunction %double %_ptr_Function_double
// CHECK: [[fn_param_int_ret_bool:%[a-zA-Z0-9_]+]] = OpTypeFunction %bool %_ptr_Function_int
// CHECK: [[fn_param_double_ret_int:%[a-zA-Z0-9_]+]] = OpTypeFunction %int %_ptr_Function_double

// CHECK: OpFunction %int None [[fn_param_int_ret_int]]
// CHECK: OpFunctionParameter %_ptr_Function_int
  x = square<int>(x);

// CHECK: OpFunction %float None [[fn_param_float_ret_float]]
// CHECK: OpFunctionParameter %_ptr_Function_float
  y = square<float>(y);

// CHECK: OpFunction %double None [[fn_param_double_ret_double]]
// CHECK: OpFunctionParameter %_ptr_Function_double
  y = square<double>(x);

// CHECK: %plusTwo = OpFunction %bool None [[fn_param_int_ret_bool]]
// CHECK: %n = OpFunctionParameter %_ptr_Function_int
  y = plusTwo<bool>(x);

// CHECK: %triple = OpFunction %int None [[fn_param_double_ret_int]]
// CHECK: OpFunctionParameter %_ptr_Function_double
  y = triple<double>(x);
}
