// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s

struct S {
    float4 f;
};

// vk::binding + vk::counter_binding
[[vk::binding(5, 3), vk::counter_binding(10)]]
RWStructuredBuffer<S> mySBuffer1;

// :register(xX, spaceY) + vk::counter_binding
[[vk::counter_binding(20)]]
AppendStructuredBuffer<S> myASBuffer1 : register(u1, space1);
// CHECK:      OpDecorate %counter_var_myASBuffer1 DescriptorSet 1
// CHECK-NEXT: OpDecorate %counter_var_myASBuffer1 Binding 20

// :register(spaceY) + vk::counter_binding
[[vk::counter_binding(15)]]
RWStructuredBuffer<S> mySBuffer3 : register(space3);

// none + vk::counter_binding
[[vk::counter_binding(2)]]
ConsumeStructuredBuffer<S> myCSBuffer1;
// CHECK:      OpDecorate %counter_var_myCSBuffer1 DescriptorSet 0
// CHECK-NEXT: OpDecorate %counter_var_myCSBuffer1 Binding 2

// CHECK:      OpDecorate %counter_var_mySBuffer1 DescriptorSet 3
// CHECK-NEXT: OpDecorate %counter_var_mySBuffer1 Binding 10

// vk::binding + none
[[vk::binding(1)]]
RWStructuredBuffer<S> mySBuffer2;

// :register(xX, spaceY) + none
AppendStructuredBuffer<S> myASBuffer2 : register(u3, space2);
// CHECK:      OpDecorate %counter_var_myASBuffer2 DescriptorSet 2
// CHECK-NEXT: OpDecorate %counter_var_myASBuffer2 Binding 0

// :register(spaceY) + None
ConsumeStructuredBuffer<S> myCSBuffer3 : register(space2);
// CHECK:      OpDecorate %counter_var_myCSBuffer3 DescriptorSet 2
// CHECK-NEXT: OpDecorate %counter_var_myCSBuffer3 Binding 2

// none + none
ConsumeStructuredBuffer<S> myCSBuffer2;
// CHECK:      OpDecorate %counter_var_myCSBuffer2 DescriptorSet 0
// CHECK-NEXT: OpDecorate %counter_var_myCSBuffer2 Binding 4

// CHECK:      OpDecorate %counter_var_mySBuffer2 DescriptorSet 0
// CHECK-NEXT: OpDecorate %counter_var_mySBuffer2 Binding 5

float4 main() : SV_Target {
    uint a = mySBuffer1.IncrementCounter();
    uint b = mySBuffer2.DecrementCounter();

    return  a + b;
}
