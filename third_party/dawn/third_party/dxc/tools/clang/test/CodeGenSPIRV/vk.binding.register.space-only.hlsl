// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// CHECK:      OpDecorate %sampler1 DescriptorSet 1
// CHECK-NEXT: OpDecorate %sampler1 Binding 0
SamplerState sampler1: register(space1);

// CHECK:      OpDecorate %sampler2 DescriptorSet 1
// CHECK-NEXT: OpDecorate %sampler2 Binding 1
SamplerState sampler2 : register(space1);

// CHECK:      OpDecorate %texture1 DescriptorSet 1
// CHECK-NEXT: OpDecorate %texture1 Binding 2
Texture2D<float4> texture1: register(space1);

// CHECK:      OpDecorate %texture2 DescriptorSet 2
// CHECK-NEXT: OpDecorate %texture2 Binding 0
Texture3D<float4> texture2: register(space2);

// CHECK:      OpDecorate %myCbuffer DescriptorSet 2
// CHECK-NEXT: OpDecorate %myCbuffer Binding 1
cbuffer myCbuffer : register(space2) {
    float4 CB_a;
}

// CHECK:      OpDecorate %myTbuffer DescriptorSet 2
// CHECK-NEXT: OpDecorate %myTbuffer Binding 2
tbuffer myTbuffer : register(space2) {
    float4 TB_a;
}

// CHECK:      OpDecorate %myBuffer DescriptorSet 3
// CHECK-NEXT: OpDecorate %myBuffer Binding 0
Buffer<int> myBuffer : register(space3);

// CHECK: OpDecorate %myRWBuffer DescriptorSet 3
// CHECK-NEXT: OpDecorate %myRWBuffer Binding 1
RWBuffer<float4> myRWBuffer : register(space3);

struct S {
    float4 f;
};

// CHECK:      OpDecorate %myCbuffer2 DescriptorSet 2
// CHECK-NEXT: OpDecorate %myCbuffer2 Binding 3
ConstantBuffer<S> myCbuffer2 : register(space2);

// CHECK:      OpDecorate %myTbuffer2 DescriptorSet 4
// CHECK-NEXT: OpDecorate %myTbuffer2 Binding 0
TextureBuffer<S> myTbuffer2 : register(space4);

// CHECK:      OpDecorate %sbuffer1 DescriptorSet 1
// CHECK-NEXT: OpDecorate %sbuffer1 Binding 3
StructuredBuffer<S> sbuffer1 : register(space1);

// CHECK:      OpDecorate %sbuffer2 DescriptorSet 3
// CHECK-NEXT: OpDecorate %sbuffer2 Binding 2
RWStructuredBuffer<S> sbuffer2 : register(space3);

// CHECK:      OpDecorate %abuffer DescriptorSet 2
// CHECK-NEXT: OpDecorate %abuffer Binding 4
AppendStructuredBuffer<S> abuffer : register(space2);

// CHECK:      OpDecorate %csbuffer DescriptorSet 3
// CHECK-NEXT: OpDecorate %csbuffer Binding 3
ConsumeStructuredBuffer<S> csbuffer : register(space3);

float4 main() : SV_Target {
    return 1.0;
}

