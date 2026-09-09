// RUN: %dxc -T ps_6_0 -E main -fspv-debug=vulkan -fcgl  %s -spirv | FileCheck %s

// CHECK:      [[file:%[0-9]+]] = OpString
// CHECK:      [[src:%[0-9]+]] = OpExtInst %void %1 DebugSource [[file]]

float test_function_variables() {
// CHECK:                 DebugLine [[src]] %uint_9 %uint_9 %uint_22 %uint_22
// CHECK-NEXT: {{%[0-9]+}} = OpLoad %bool %init_done_foo
  static float foo = 2.f;
  return foo;
}

// DebugLine cannot appear before OpFunction
// CHECK:     %test_function_param = OpFunction %v2float None
// DebugLine cannot appear before OpFunctionParameter
// CHECK-NEXT:           %a = OpFunctionParameter %_ptr_Function_v2float
// DebugLine cannot appear before OpFunctionParameter
// CHECK-NEXT:           %b = OpFunctionParameter %_ptr_Function_v3bool
float2 test_function_param(float2 a, inout bool3 b,
// DebugLine cannot appear before OpFunctionParameter
// CHECK-NEXT:           %c = OpFunctionParameter %_ptr_Function_int
// DebugLine cannot appear before OpFunctionParameter
// CHECK-NEXT:           %d = OpFunctionParameter %_ptr_Function_v3float
                           const int c, out float3 d) {
// CHECK:      DebugLine [[src]] %uint_27 %uint_27 %uint_3 %uint_15
// CHECK-NEXT: OpStore %f
  float f = a.x;
  return a + d.xy + float2(f, a.y);
}

void main() {
  float bar = test_function_variables();

  bool3 param_b;
  float3 param_d;
  test_function_param(float2(1, 0), param_b, 13, param_d);
}
