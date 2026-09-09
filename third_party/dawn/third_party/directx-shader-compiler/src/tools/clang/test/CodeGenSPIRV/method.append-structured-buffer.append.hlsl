// RUN: %dxc -T vs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

struct S {
    float    a;
    float3   b;
    float2x3 c;
};

AppendStructuredBuffer<float4> buffer1;
AppendStructuredBuffer<S>      buffer2;

// Use this to test appending a consumed value.
ConsumeStructuredBuffer<float4> buffer3;

void main(float4 vec: A) {
// CHECK:      [[counter:%[0-9]+]] = OpAccessChain %_ptr_Uniform_int %counter_var_buffer1 %uint_0
// CHECK-NEXT: [[index:%[0-9]+]] = OpAtomicIAdd %int [[counter]] %uint_1 %uint_0 %int_1
// CHECK-NEXT: [[buffer1:%[0-9]+]] = OpAccessChain %_ptr_Uniform_v4float %buffer1 %uint_0 [[index]]
// CHECK-NEXT: [[vec:%[0-9]+]] = OpLoad %v4float %vec
// CHECK-NEXT: OpStore [[buffer1]] [[vec]]
    buffer1.Append(vec);

    S s; // Will use a separate S type without layout decorations

// CHECK-NEXT: [[counter_0:%[0-9]+]] = OpAccessChain %_ptr_Uniform_int %counter_var_buffer2 %uint_0
// CHECK-NEXT: [[index_0:%[0-9]+]] = OpAtomicIAdd %int [[counter_0]] %uint_1 %uint_0 %int_1

// CHECK-NEXT: [[buffer2:%[0-9]+]] = OpAccessChain %_ptr_Uniform_S %buffer2 %uint_0 [[index_0]]
// CHECK-NEXT: [[s:%[0-9]+]] = OpLoad %S_0 %s

// CHECK-NEXT: [[s_a:%[0-9]+]] = OpCompositeExtract %float [[s]] 0
// CHECK-NEXT: [[s_b:%[0-9]+]] = OpCompositeExtract %v3float [[s]] 1
// CHECK-NEXT: [[s_c:%[0-9]+]] = OpCompositeExtract %mat2v3float [[s]] 2

// CHECK-NEXT: [[val:%[0-9]+]] = OpCompositeConstruct %S [[s_a]] [[s_b]] [[s_c]]
// CHECK-NEXT: OpStore [[buffer2]] [[val]]
    buffer2.Append(s);

// CHECK:           [[buffer1_0:%[0-9]+]] = OpAccessChain %_ptr_Uniform_v4float %buffer1 %uint_0 {{%[0-9]+}}
// CHECK:      [[consumed_ptr:%[0-9]+]] = OpAccessChain %_ptr_Uniform_v4float %buffer3 %uint_0 {{%[0-9]+}}
// CHECK-NEXT:     [[consumed:%[0-9]+]] = OpLoad %v4float [[consumed_ptr]]
// CHECK-NEXT:                         OpStore [[buffer1_0]] [[consumed]]
    buffer1.Append(buffer3.Consume());
}
