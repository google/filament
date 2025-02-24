// RUN: %dxc -E main -T ps_6_0 %s  | FileCheck %s

class C {
    float4 f;
};

struct S {
    float4 f;
};

// CHECK: error: 'int' cannot be used as a type parameter where a struct/class is required
ConstantBuffer<int>      B1;
// CHECK: error: 'float2' cannot be used as a type parameter where a struct/class is required
TextureBuffer<float2>    B2;
// CHECK: error: 'float3x4' cannot be used as a type parameter where a struct/class is required
ConstantBuffer<float3x4> B3;

TextureBuffer<C>         B4;
// CHECK-NOT: const S
ConstantBuffer<S>        B5;
TextureBuffer<S>         B6[6];

float4 main(int a : A) : SV_Target {
  return B4.f;
}