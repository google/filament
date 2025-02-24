// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s

struct S {
    float4 f;
};

// Verify that descriptor set-indicating override per: vk::binding > :register
// using variations on these and vk::counter_binding

// explicit set vk::binding > :register
[[vk::binding(5, 4)]]
RWBuffer<float4> exBindVsReg : register(u6, space7);
// CHECK:      OpDecorate %exBindVsReg DescriptorSet 4
// CHECK-NEXT: OpDecorate %exBindVsReg Binding 5

[[vk::binding(6, 4)]]
RWBuffer<float4> exBindVsReg2 : register(space7);
// CHECK:      OpDecorate %exBindVsReg2 DescriptorSet 4
// CHECK-NEXT: OpDecorate %exBindVsReg2 Binding 6

// implicit set vk::binding > :register
[[vk::binding(8)]]
cbuffer impBindVsReg : register(b9, space1) {
    float cbfield;
};
// CHECK:      OpDecorate %impBindVsReg DescriptorSet 0
// CHECK-NEXT: OpDecorate %impBindVsReg Binding 8

[[vk::binding(9)]]
cbuffer impBindVsReg2 : register(space1) {
    float cbfield2;
};
// CHECK:      OpDecorate %impBindVsReg2 DescriptorSet 0
// CHECK-NEXT: OpDecorate %impBindVsReg2 Binding 9

// explicit set vk::binding + explict counter > :register
[[vk::binding(4,1)]]
RWStructuredBuffer<S> exBindVsRegExCt : register(u4, space9);
// CHECK:      OpDecorate %exBindVsRegExCt DescriptorSet 1
// CHECK-NEXT: OpDecorate %exBindVsRegExCt Binding 4

[[vk::binding(6,1)]]
RWStructuredBuffer<S> exBindVsRegExCt2 : register(space9);
// CHECK:      OpDecorate %exBindVsRegExCt2 DescriptorSet 1
// CHECK-NEXT: OpDecorate %exBindVsRegExCt2 Binding 6

// implicit set vk::binding + explict counter > :register
[[vk::binding(2), vk::counter_binding(5)]]
AppendStructuredBuffer<S> impBindVsRegExCt : register(u22, space8);
// CHECK-NEXT: OpDecorate %impBindVsRegExCt DescriptorSet 0
// CHECK-NEXT: OpDecorate %impBindVsRegExCt Binding 2
// CHECK-NEXT: OpDecorate %counter_var_impBindVsRegExCt DescriptorSet 0
// CHECK-NEXT: OpDecorate %counter_var_impBindVsRegExCt Binding 5

[[vk::binding(11), vk::counter_binding(12)]]
AppendStructuredBuffer<S> impBindVsRegExCt2 : register(u21, space8);
// CHECK-NEXT: OpDecorate %impBindVsRegExCt2 DescriptorSet 0
// CHECK-NEXT: OpDecorate %impBindVsRegExCt2 Binding 11
// CHECK-NEXT: OpDecorate %counter_var_impBindVsRegExCt2 DescriptorSet 0
// CHECK-NEXT: OpDecorate %counter_var_impBindVsRegExCt2 Binding 12

// explicit set vk::binding + implicit counter > :register (main part)
[[vk::binding(4, 3)]]
ConsumeStructuredBuffer<S> exBindVsRegImpCt : register(u3, space9);
// CHECK:      OpDecorate %exBindVsRegImpCt DescriptorSet 3
// CHECK-NEXT: OpDecorate %exBindVsRegImpCt Binding 4

[[vk::binding(5, 3)]]
ConsumeStructuredBuffer<S> exBindVsRegImpCt2 : register(u8, space9);
// CHECK:      OpDecorate %exBindVsRegImpCt2 DescriptorSet 3
// CHECK-NEXT: OpDecorate %exBindVsRegImpCt2 Binding 5

// implicit set vk::binding + implicit counter > :register (main part)
[[vk::binding(12)]]
RWStructuredBuffer<S> impBindVsRegImpCt : register(u9, space4);
// CHECK:      OpDecorate %impBindVsRegImpCt DescriptorSet 0
// CHECK-NEXT: OpDecorate %impBindVsRegImpCt Binding 12

[[vk::binding(21)]]
RWStructuredBuffer<S> impBindVsRegImpCt2 : register(u2, space4);
// CHECK:      OpDecorate %impBindVsRegImpCt2 DescriptorSet 0
// CHECK-NEXT: OpDecorate %impBindVsRegImpCt2 Binding 21

// explicit set vk::binding > :register implicit counter (counter part)
// CHECK:      OpDecorate %counter_var_exBindVsRegImpCt DescriptorSet 3
// CHECK-NEXT: OpDecorate %counter_var_exBindVsRegImpCt Binding 0

// CHECK:      OpDecorate %counter_var_exBindVsRegImpCt2 DescriptorSet 3
// CHECK-NEXT: OpDecorate %counter_var_exBindVsRegImpCt2 Binding 1

float4 main() : SV_Target {
    return 1.0;
}
