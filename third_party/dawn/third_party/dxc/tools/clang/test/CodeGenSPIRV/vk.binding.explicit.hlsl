// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// CHECK:      OpDecorate %sampler1 DescriptorSet 0
// CHECK-NEXT: OpDecorate %sampler1 Binding 0
[[vk::binding(0)]]
SamplerState sampler1      : register(s1, space1);

// CHECK:      OpDecorate %sampler2 DescriptorSet 1
// CHECK-NEXT: OpDecorate %sampler2 Binding 3
[[vk::binding(3, 1)]]
SamplerState sampler2      : register(s2);

// CHECK:      OpDecorate %texture1 DescriptorSet 0
// CHECK-NEXT: OpDecorate %texture1 Binding 2
[[vk::binding(2)]]
Texture2D<float4> texture1;

// CHECK:      OpDecorate %texture2 DescriptorSet 2
// CHECK-NEXT: OpDecorate %texture2 Binding 2
[[vk::binding(2, 2)]]
Texture3D<float4> texture2 : register(t0, space0);

// CHECK:      OpDecorate %myBuffer DescriptorSet 2
// CHECK-NEXT: OpDecorate %myBuffer Binding 3
[[vk::binding(3, 2)]]
Buffer<int> myBuffer : register(t1, space0);

// CHECK: OpDecorate %myRWBuffer DescriptorSet 1
// CHECK-NEXT: OpDecorate %myRWBuffer Binding 4
[[vk::binding(4, 1)]]
RWBuffer<float4> myRWBuffer : register(u0, space1);

// CHECK: OpDecorate %myCBuffer DescriptorSet 3
// CHECK-NEXT: OpDecorate %myCBuffer Binding 10
[[vk::binding(10, 3)]]
cbuffer myCBuffer : register(b5, space1) {
    float cbfield;
};

struct S {
    float f;
};

// CHECK: OpDecorate %myConstantBuffer DescriptorSet 3
// CHECK-NEXT: OpDecorate %myConstantBuffer Binding 5
[[vk::binding(5, 3)]]
ConstantBuffer<S> myConstantBuffer: register(b1, space1);

// CHECK:      OpDecorate %sbuffer1 DescriptorSet 0
// CHECK-NEXT: OpDecorate %sbuffer1 Binding 3
[[vk::binding(3)]]
  StructuredBuffer<S> sbuffer1 : register(t5);
// CHECK:      OpDecorate %sbuffer2 DescriptorSet 3
// CHECK-NEXT: OpDecorate %sbuffer2 Binding 2
[[vk::binding(2, 3)]]
RWStructuredBuffer<S> sbuffer2 : register(u6);

// CHECK:      OpDecorate %asbuffer DescriptorSet 1
// CHECK-NEXT: OpDecorate %asbuffer Binding 1
// CHECK-NEXT: OpDecorate %csbuffer DescriptorSet 1
// CHECK-NEXT: OpDecorate %csbuffer Binding 21
// CHECK-NEXT: OpDecorate %counter_var_asbuffer DescriptorSet 1
// CHECK-NEXT: OpDecorate %counter_var_asbuffer Binding 0
// CHECK-NEXT: OpDecorate %counter_var_csbuffer DescriptorSet 1
// CHECK-NEXT: OpDecorate %counter_var_csbuffer Binding 2
[[vk::binding(1, 1)]]
AppendStructuredBuffer<S> asbuffer : register(u10);
// Next available "hole" in set #1: binding #0
[[vk::binding(21, 1)]]
ConsumeStructuredBuffer<S> csbuffer : register(u11);
// Next available "hole" in set #1: binding #2

float4 main() : SV_Target {
    return 1.0;
}
