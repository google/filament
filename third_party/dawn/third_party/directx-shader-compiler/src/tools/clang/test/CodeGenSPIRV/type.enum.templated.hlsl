// RUN: %dxc -T cs_6_6 -E main -spirv %s -fcgl | FileCheck %s

template<typename T>
void templated_func() {
  T tmp;
}

template<typename T>
void templated_func_deduced(T tmp) {
}

enum MyEnum { };

template<typename T>
using TemplatedType = MyEnum;

[numthreads(1, 1, 1)]
void main() {
// CHECK: %a = OpVariable %_ptr_Function_int Function
  TemplatedType<MyEnum> a;

// CHECK: OpFunctionCall %void %templated_func
  templated_func<MyEnum>();

// CHECK: [[a:%[0-9]+]] = OpLoad %int %a
// CHECK:                 OpStore %param_var_tmp [[a]]
// CHECK:                 OpFunctionCall %void %templated_func_deduced %param_var_tmp
  templated_func_deduced(a);
}
