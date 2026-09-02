// RUN: %dxc -T vs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

RWStructuredBuffer<float> Data;

void foo(in float a, inout float b, out float c) {
    b += a;
    c = a + b;
}

void main(float input : INPUT) {
// CHECK: %param_var_a = OpVariable %_ptr_Function_float Function

// CHECK: [[val:%[0-9]+]] = OpLoad %float %input
// CHECK:                OpStore %param_var_a [[val]]
// CHECK:  [[p0:%[0-9]+]] = OpAccessChain %_ptr_Uniform_float %Data %int_0 %uint_0
// CHECK:  [[p1:%[0-9]+]] = OpAccessChain %_ptr_Uniform_float %Data %int_0 %uint_1

// CHECK:                OpFunctionCall %void %foo %param_var_a [[p0]] [[p1]]
    foo(input, Data[0], Data[1]);
}
