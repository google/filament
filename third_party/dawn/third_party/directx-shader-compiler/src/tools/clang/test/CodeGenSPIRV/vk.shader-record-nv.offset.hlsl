// RUN: %dxc -T lib_6_3 -fspv-extension=SPV_NV_ray_tracing -fcgl  %s -spirv | FileCheck %s

// CHECK: OpMemberDecorate %type_ConstantBuffer_S 0 Offset 0
// CHECK: OpMemberDecorate %type_ConstantBuffer_S 1 Offset 8
// CHECK: OpMemberDecorate %type_ConstantBuffer_S 2 Offset 32

struct S {
    float a;
    [[vk::offset(8)]]
    float2 b;
    [[vk::offset(32)]]
    float4 f;
};

[[vk::shader_record_nv]]
ConstantBuffer<S> srb;

struct Payload { float p; };
struct Attr    { float a; };
[shader("closesthit")]
void main(inout Payload P, in Attr a) {
    P.p = srb.a;
}
