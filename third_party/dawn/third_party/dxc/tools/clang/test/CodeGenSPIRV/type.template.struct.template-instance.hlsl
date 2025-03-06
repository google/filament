// RUN: %dxc -T ps_6_0 -E main -HV 2021 -fcgl  %s -spirv | FileCheck %s

// The SPIR-V backend correctly handles the template instance `Foo<int>`.
// The created template instance is ClassTemplateSpecializationDecl in AST.

template <typename T>
struct Foo {
    static const T bar = 0;

    T value;
    void set(T value_) { value = value_; }
    T get() { return value; }
};

void main() {
// CHECK: [[bar_int:%[a-zA-Z0-9_]+]] = OpVariable %_ptr_Private_int Private
// CHECK: [[bar_float:%[a-zA-Z0-9_]+]] = OpVariable %_ptr_Private_float Private

// CHECK: OpStore [[bar_int]] %int_0
// CHECK: OpStore [[bar_float]] %float_0

    Foo<int>::bar;

// CHECK: %x = OpVariable %_ptr_Function_int Function
    int x;

// CHECK: %y = OpVariable %_ptr_Function_Foo Function
    Foo<float> y;

// CHECK:       [[x:%[a-zA-Z0-9_]+]] = OpLoad %int %x
// CHECK: [[float_x:%[a-zA-Z0-9_]+]] = OpConvertSToF %float [[x]]
// CHECK:                    OpStore [[param_value_:%[a-zA-Z0-9_]+]] [[float_x]]
// CHECK:                    OpFunctionCall %void %Foo_set %y [[param_value_]]
    y.set(x);

// CHECK:     [[y_get:%[a-zA-Z0-9_]+]] = OpFunctionCall %float %Foo_get %y
// CHECK: [[y_get_int:%[a-zA-Z0-9_]+]] = OpConvertFToS %int [[y_get]]
// CHECK:                      OpStore %x [[y_get_int]]
    x = y.get();
}

// CHECK:         %Foo_set = OpFunction
// CHECK-NEXT: %param_this = OpFunctionParameter %_ptr_Function_Foo
// CHECK-NEXT:     %value_ = OpFunctionParameter %_ptr_Function_float

// CHECK:         %Foo_get = OpFunction %float
// CHECK-NEXT: [[this:%[a-zA-Z0-9_]+]] = OpFunctionParameter %_ptr_Function_Foo

// CHECK: [[ptr_value:%[a-zA-Z0-9_]+]] = OpAccessChain %_ptr_Function_float [[this]] %int_0
// CHECK:     [[value:%[a-zA-Z0-9_]+]] = OpLoad %float [[ptr_value]]
// CHECK:                      OpReturnValue [[value]]
