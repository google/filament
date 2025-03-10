// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// CHECK: call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 0, float 2.500000e-01)
// CHECK-NOT: @dx.op.cbufferLoadLegacy

struct S
{
    static float4 data;
};

float4 S::data = 0.25f;

float4 main() : SV_TARGET
{
    return S::data;
}