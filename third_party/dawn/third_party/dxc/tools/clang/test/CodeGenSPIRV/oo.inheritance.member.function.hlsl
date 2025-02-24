// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s

struct Base {
    int a;
};

struct Derived : Base {
    float b;

// CHECK: %Derived_increase = OpFunction %void None
// CHECK: %param_this = OpFunctionParameter %_ptr_Function_Derived
// CHECK: OpLabel
// CHECK: OpAccessChain %_ptr_Function_float %param_this %int_1

    void increase() { ++b; }
};

void main() {
  Derived foo;
  foo.increase();
}
