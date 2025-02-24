// RUN: %dxc -T ps_6_0 -E main -fvk-b-shift 100 0 -fvk-b-shift 200 2 -fvk-t-shift 200 0 -fvk-t-shift 300 1 -fvk-t-shift 400 0 -fvk-s-shift 500 0 -fvk-s-shift 600 2 -fvk-u-shift 700 0 -fvk-u-shift 800 3 -fcgl  %s -spirv | FileCheck %s

// Tests that we can set shift for more than one sets of the same register type
// Tests that we can override shift for the same set

struct S {
    float4 f;
};

// Explicit binding assignment is unaffected.

// CHECK: OpDecorate %cbuffer3 DescriptorSet 0
// CHECK: OpDecorate %cbuffer3 Binding 42
[[vk::binding(42)]]
ConstantBuffer<S> cbuffer3 : register(b10, space2);

// CHECK: OpDecorate %cbuffer1 DescriptorSet 0
// CHECK: OpDecorate %cbuffer1 Binding 100
ConstantBuffer<S> cbuffer1 : register(b0);
// CHECK: OpDecorate %cbuffer2 DescriptorSet 2
// CHECK: OpDecorate %cbuffer2 Binding 200
ConstantBuffer<S> cbuffer2 : register(b0, space2);

// CHECK: OpDecorate %texture1 DescriptorSet 1
// CHECK: OpDecorate %texture1 Binding 301
Texture2D<float4> texture1: register(t1, space1);
// CHECK: OpDecorate %texture2 DescriptorSet 0
// CHECK: OpDecorate %texture2 Binding 401
Texture2D<float4> texture2: register(t1);

// CHECK: OpDecorate %sampler1 DescriptorSet 0
// CHECK: OpDecorate %sampler1 Binding 500
// CHECK: OpDecorate %sampler2 DescriptorSet 2
// CHECK: OpDecorate %sampler2 Binding 600
SamplerState sampler1: register(s0);
SamplerState sampler2: register(s0, space2);

// CHECK: OpDecorate %rwbuffer1 DescriptorSet 3
// CHECK: OpDecorate %rwbuffer1 Binding 803
RWBuffer<float4> rwbuffer1 : register(u3, space3);
// CHECK: OpDecorate %rwbuffer2 DescriptorSet 0
// CHECK: OpDecorate %rwbuffer2 Binding 703
RWBuffer<float4> rwbuffer2 : register(u3);

// Lacking binding assignment is unaffacted.

// CHECK: OpDecorate %cbuffer4 DescriptorSet 0
// CHECK: OpDecorate %cbuffer4 Binding 0
ConstantBuffer<S> cbuffer4;
// CHECK: OpDecorate %cbuffer5 DescriptorSet 0
// CHECK: OpDecorate %cbuffer5 Binding 1
ConstantBuffer<S> cbuffer5;

float4 main() : SV_Target {
    return cbuffer1.f;
}
