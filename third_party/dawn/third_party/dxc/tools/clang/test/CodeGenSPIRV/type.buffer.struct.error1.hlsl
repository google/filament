// RUN: not %dxc -T ps_6_0 -E main -fcgl  %s -spirv  2>&1 | FileCheck %s

struct S {
    float4 a;
    float3 b;
};

Buffer<S> MyBuffer;

float4 main(): SV_Target {
    return MyBuffer[0].a;
}

// CHECK: :3:8: error: resource template element type 'S' cannot fit into four 32-bit scalars
