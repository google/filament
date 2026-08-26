// RUN: %dxc -T ps_6_0 -E main -Zi -fcgl  %s -spirv | FileCheck %s

// CHECK:      [[file:%[0-9]+]] = OpString
// CHECK-SAME: spirv.debug.opline.variables.hlsl

float test_function_variables() {
// CHECK:                 OpLine [[file]] 9 22
// CHECK-NEXT: {{%[0-9]+}} = OpLoad %bool %init_done_foo
  static float foo = 2.f;
  return foo;
}

// CHECK:                     OpLine [[file]] 19 1
// CHECK-NEXT: %test_function_param = OpFunction %v2float None
// CHECK-NEXT:                OpLine [[file]] 19 35
// CHECK-NEXT:           %a = OpFunctionParameter %_ptr_Function_v2float
// CHECK-NEXT:                OpLine [[file]] 19 50
// CHECK-NEXT:           %b = OpFunctionParameter %_ptr_Function_v3bool
float2 test_function_param(float2 a, inout bool3 b,
// CHECK-NEXT:                OpLine [[file]] 24 38
// CHECK-NEXT:           %c = OpFunctionParameter %_ptr_Function_int
// CHECK-NEXT:                OpLine [[file]] 24 52
// CHECK-NEXT:           %d = OpFunctionParameter %_ptr_Function_v3float
                           const int c, out float3 d) {
// CHECK:      OpLine [[file]] 27 9
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
