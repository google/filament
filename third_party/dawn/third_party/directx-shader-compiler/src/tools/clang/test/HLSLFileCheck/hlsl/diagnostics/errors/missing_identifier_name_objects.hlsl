
// This test checks that when identifier name is missing for various object types,
// dxcompiler generates error and no crashes are observed.

// RUN: %dxc -E main -T vs_6_0 %s | FileCheck %s

// CHECK: error: expected unqualified-id
// CHECK: error: expected unqualified-id
// CHECK: error: expected unqualified-id
// CHECK: error: expected unqualified-id
// CHECK: error: expected unqualified-id
// CHECK: error: expected unqualified-id
// CHECK: error: expected unqualified-id
// CHECK: error: expected unqualified-id
// CHECK: error: expected unqualified-id
// CHECK: error: expected unqualified-id

struct S
{
    float foo;
};

Buffer<float4> : register(b0);
ByteAddressBuffer : register(b0);
Texture2D<float4> : register(t0);
ConstantBuffer<S> : register(b0);
StructuredBuffer<S> : register(b0);
RWTexture1D<float4> : register(b0);
RWBuffer<float2> : register(b0);
TextureCube<float4> : register(b0);
TextureCubeArray<float4> : register(b0);
RWStructuredBuffer<int1x1> : register(b0);

void main() {}