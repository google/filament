// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s

struct S {
  int x;
  float y;
};

S foo();

void main() {

// CHECK: [[foo:%[0-9]+]] = OpFunctionCall %S %foo
// CHECK: OpStore %result [[foo]]
  S result = foo();
}

S foo() {
// CHECK: %s = OpVariable %_ptr_Function_S Function
// CHECK: OpStore %s {{%[0-9]+}}
  S s = {1, 2.0};

// CHECK: [[s:%[0-9]+]] = OpLoad %S %s
// CHECK: OpReturnValue [[s]]
  return s;
}
