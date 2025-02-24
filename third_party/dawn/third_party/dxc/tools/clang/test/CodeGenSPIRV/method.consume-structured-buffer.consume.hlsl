// RUN: %dxc -T vs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

struct S {
    float    a;
    float3   b;
    float2x3 c;
};

struct T {
    S        s[5];
};

ConsumeStructuredBuffer<float4> buffer1;
ConsumeStructuredBuffer<S>      buffer2;
ConsumeStructuredBuffer<T>      buffer3;

float4 main() : A {
// CHECK:      [[counter:%[0-9]+]] = OpAccessChain %_ptr_Uniform_int %counter_var_buffer1 %uint_0
// CHECK-NEXT: [[prev:%[0-9]+]] = OpAtomicISub %int [[counter]] %uint_1 %uint_0 %int_1
// CHECK-NEXT: [[index:%[0-9]+]] = OpISub %int [[prev]] %int_1
// CHECK-NEXT: [[buffer1:%[0-9]+]] = OpAccessChain %_ptr_Uniform_v4float %buffer1 %uint_0 [[index]]
// CHECK-NEXT: [[val:%[0-9]+]] = OpLoad %v4float [[buffer1]]
// CHECK-NEXT: OpStore %v [[val]]
    float4 v = buffer1.Consume();

    S s; // Will use a separate S type without layout decorations

// CHECK-NEXT: [[counter_0:%[0-9]+]] = OpAccessChain %_ptr_Uniform_int %counter_var_buffer2 %uint_0
// CHECK-NEXT: [[prev_0:%[0-9]+]] = OpAtomicISub %int [[counter_0]] %uint_1 %uint_0 %int_1
// CHECK-NEXT: [[index_0:%[0-9]+]] = OpISub %int [[prev_0]] %int_1

// CHECK-NEXT: [[buffer2:%[0-9]+]] = OpAccessChain %_ptr_Uniform_S %buffer2 %uint_0 [[index_0]]
// CHECK-NEXT: [[val_0:%[0-9]+]] = OpLoad %S [[buffer2]]

// CHECK-NEXT: [[s_a:%[0-9]+]] = OpCompositeExtract %float [[val_0]] 0
// CHECK-NEXT: [[s_b:%[0-9]+]] = OpCompositeExtract %v3float [[val_0]] 1
// CHECK-NEXT: [[s_c:%[0-9]+]] = OpCompositeExtract %mat2v3float [[val_0]] 2

// CHECK-NEXT: [[tmp:%[0-9]+]] = OpCompositeConstruct %S_0 [[s_a]] [[s_b]] [[s_c]]
// CHECK-NEXT: OpStore %s [[tmp]]
    s = buffer2.Consume();

// CHECK:      [[counter_1:%[0-9]+]] = OpAccessChain %_ptr_Uniform_int %counter_var_buffer3 %uint_0
// CHECK-NEXT: [[prev_1:%[0-9]+]] = OpAtomicISub %int [[counter_1]] %uint_1 %uint_0 %int_1
// CHECK-NEXT: [[index_1:%[0-9]+]] = OpISub %int [[prev_1]] %int_1
// CHECK-NEXT: [[buffer3:%[0-9]+]] = OpAccessChain %_ptr_Uniform_T %buffer3 %uint_0 [[index_1]]
// CHECK-NEXT: [[ac:%[0-9]+]] = OpAccessChain %_ptr_Uniform_v3float [[buffer3]] %int_0 %int_3 %int_1
// CHECK-NEXT: [[val_1:%[0-9]+]] = OpLoad %v3float [[ac]]
// CHECK-NEXT: OpStore %val [[val_1]]
    float3 val = buffer3.Consume().s[3].b;

    return v;
}
