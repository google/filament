// RUN: %dxc -T cs_6_0 -E main -HV 202x -fcgl %s -spirv | FileCheck %s

// Test that the 'auto' keyword can be used to declare variables with inferred
// types from initialization expressions when targeting SPIR-V.

// CHECK: [[INT:%[a-zA-Z0-9_]+]] = OpTypeInt 32 1
// CHECK: [[INT_1:%[a-zA-Z0-9_]+]] = OpConstant [[INT]] 1
// CHECK: [[FLOAT:%[a-zA-Z0-9_]+]] = OpTypeFloat 32
// CHECK: [[FLOAT_2:%[a-zA-Z0-9_]+]] = OpConstant [[FLOAT]] 2
// CHECK: [[BOOL:%[a-zA-Z0-9_]+]] = OpTypeBool
// CHECK: [[TRUE:%[a-zA-Z0-9_]+]] = OpConstantTrue [[BOOL]]
// CHECK: [[V4FLOAT:%[a-zA-Z0-9_]+]] = OpTypeVector [[FLOAT]] 4
// CHECK: [[VEC_CONST:%[a-zA-Z0-9_]+]] = OpConstantComposite [[V4FLOAT]] {{%[a-zA-Z0-9_]+}} [[FLOAT_2]] {{%[a-zA-Z0-9_]+}} {{%[a-zA-Z0-9_]+}}

// CHECK: [[PTR_INT:%_ptr_Function_int]] = OpTypePointer Function [[INT]]
// CHECK: [[PTR_FLOAT:%_ptr_Function_float]] = OpTypePointer Function [[FLOAT]]
// CHECK: [[PTR_BOOL:%_ptr_Function_bool]] = OpTypePointer Function [[BOOL]]
// CHECK: [[PTR_V4FLOAT:%_ptr_Function_v4float]] = OpTypePointer Function [[V4FLOAT]]

// CHECK: %a = OpVariable [[PTR_INT]] Function
// CHECK: %b = OpVariable [[PTR_FLOAT]] Function
// CHECK: %c = OpVariable [[PTR_BOOL]] Function
// CHECK: %d = OpVariable [[PTR_V4FLOAT]] Function
// CHECK: %sum = OpVariable [[PTR_INT]] Function
// CHECK: %product = OpVariable [[PTR_FLOAT]] Function

// CHECK: OpStore %a [[INT_1]]
// CHECK: OpStore %b [[FLOAT_2]]
// CHECK: OpStore %c [[TRUE]]
// CHECK: OpStore %d [[VEC_CONST]]

RWBuffer<float> output : register(u0);

[numthreads(1,1,1)]
void main() {
    // Auto deduces int from integer literal
    auto a = 1;
    // Auto deduces float from float literal
    auto b = 2.0f;
    // Auto deduces bool from bool literal
    auto c = true;
    // Auto deduces float4 from vector type
    auto d = float4(1.0f, 2.0f, 3.0f, 4.0f);

    // Auto from arithmetic expressions
    auto sum = a + a;
    auto product = b * b;

    // Use the values to prevent dead-code elimination
    output[0] = (float)sum + product + d.x + (float)c;
}
