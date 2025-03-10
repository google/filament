// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// CHECK:      OpDecorate %sampler1 DescriptorSet 0
// CHECK-NEXT: OpDecorate %sampler1 Binding 0
SamplerState sampler1;

// CHECK:      OpDecorate %texture1 DescriptorSet 0
// CHECK-NEXT: OpDecorate %texture1 Binding 1
Texture2D<float4> texture1;

// CHECK:      OpDecorate %texture2 DescriptorSet 0
// CHECK-NEXT: OpDecorate %texture2 Binding 2
Texture3D<float4> texture2;

// CHECK:      OpDecorate %sampler2 DescriptorSet 0
// CHECK-NEXT: OpDecorate %sampler2 Binding 3
SamplerState sampler2;

// CHECK:      OpDecorate %myCbuffer DescriptorSet 0
// CHECK-NEXT: OpDecorate %myCbuffer Binding 4
cbuffer myCbuffer {
    float4 stuff;
}

// CHECK:      OpDecorate %myBuffer DescriptorSet 0
// CHECK-NEXT: OpDecorate %myBuffer Binding 5
Buffer<int> myBuffer;

// CHECK:      OpDecorate %myRWBuffer DescriptorSet 0
// CHECK-NEXT: OpDecorate %myRWBuffer Binding 6
RWBuffer<float4> myRWBuffer;

struct S {
    float4 f;
};

// CHECK:      OpDecorate %myCbuffer2 DescriptorSet 0
// CHECK-NEXT: OpDecorate %myCbuffer2 Binding 7
ConstantBuffer<S> myCbuffer2;

// CHECK:      OpDecorate %sbuffer1 DescriptorSet 0
// CHECK-NEXT: OpDecorate %sbuffer1 Binding 8
  StructuredBuffer<S> sbuffer1;
// CHECK:      OpDecorate %sbuffer2 DescriptorSet 0
// CHECK-NEXT: OpDecorate %sbuffer2 Binding 9
RWStructuredBuffer<S> sbuffer2;

// CHECK:      OpDecorate %abuffer DescriptorSet 0
// CHECK-NEXT: OpDecorate %abuffer Binding 10
// CHECK-NEXT: OpDecorate %counter_var_abuffer DescriptorSet 0
// CHECK-NEXT: OpDecorate %counter_var_abuffer Binding 11
AppendStructuredBuffer<S> abuffer;

// CHECK:      OpDecorate %csbuffer DescriptorSet 0
// CHECK-NEXT: OpDecorate %csbuffer Binding 12
// CHECK-NEXT: OpDecorate %counter_var_csbuffer DescriptorSet 0
// CHECK-NEXT: OpDecorate %counter_var_csbuffer Binding 13
ConsumeStructuredBuffer<S> csbuffer;

float4 main() : SV_Target {
    return 1.0;
}
