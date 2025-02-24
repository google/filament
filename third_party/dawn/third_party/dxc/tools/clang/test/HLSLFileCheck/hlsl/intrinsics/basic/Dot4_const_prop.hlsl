// RUN: %dxc -T ps_6_0 %s -E main | %FileCheck %s
// CHECK: call void @dx.op.storeOutput{{.*}} float 0x4044010000000000

[RootSignature("")]
float main() : SV_Target {
    float4 x = float4(0.5, 0.125, 1.5, 8.5);
    float4 y = float4(0.25, 0.0625, 2.5, 4.25);
    return dot(x, y);
}
