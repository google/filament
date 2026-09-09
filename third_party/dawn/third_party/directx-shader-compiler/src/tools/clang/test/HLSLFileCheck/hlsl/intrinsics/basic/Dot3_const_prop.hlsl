// RUN: %dxc -T ps_6_0 %s -E main | %FileCheck %s
// CHECK: call void @dx.op.storeOutput{{.*}} float 0x400F100000000000

[RootSignature("")]
float main() : SV_Target {
    float3 x = float3(0.5, 0.125, 1.5);
    float3 y = float3(0.25, 0.0625, 2.5);
    return dot(x, y);
}
