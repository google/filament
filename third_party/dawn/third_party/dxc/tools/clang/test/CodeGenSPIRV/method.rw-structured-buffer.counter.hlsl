// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s

struct S {
    float4 f;
};

// CHECK: OpMemberDecorate %type_ACSBuffer_counter 0 Offset 0
// CHECK: OpDecorate %type_ACSBuffer_counter BufferBlock

// CHECK: %type_ACSBuffer_counter = OpTypeStruct %int

// CHECK: %counter_var_wCounter1 = OpVariable %_ptr_Uniform_type_ACSBuffer_counter Uniform
// CHECK: %counter_var_wCounter2 = OpVariable %_ptr_Uniform_type_ACSBuffer_counter Uniform
RWStructuredBuffer<S> wCounter1;
RWStructuredBuffer<S> wCounter2;
RWStructuredBuffer<S> woCounter;

float4 main() : SV_Target {
// CHECK:      [[ptr1:%[0-9]+]] = OpAccessChain %_ptr_Uniform_int %counter_var_wCounter1 %uint_0
// CHECK-NEXT: [[pre1:%[0-9]+]] = OpAtomicIAdd %int [[ptr1]] %uint_1 %uint_0 %int_1
// CHECK-NEXT: [[val1:%[0-9]+]] = OpBitcast %uint [[pre1]]
// CHECK-NEXT:                 OpStore %a [[val1]]
    uint a = wCounter1.IncrementCounter();
// CHECK:      [[ptr2:%[0-9]+]] = OpAccessChain %_ptr_Uniform_int %counter_var_wCounter2 %uint_0
// CHECK-NEXT: [[pre2:%[0-9]+]] = OpAtomicISub %int [[ptr2]] %uint_1 %uint_0 %int_1
// CHECK-NEXT: [[cnt2:%[0-9]+]] = OpISub %int [[pre2]] %int_1
// CHECK-NEXT: [[val2:%[0-9]+]] = OpBitcast %uint [[cnt2]]
// CHECK-NEXT:                 OpStore %b [[val2]]
    uint b = wCounter2.DecrementCounter();

    return woCounter[0].f + float4(a, b, 0, 0);
}
