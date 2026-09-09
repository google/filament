// RUN: %dxc -T ps_6_0 %s -E main | %FileCheck %s
// CHECK: call void @dx.op.storeOutput{{.*}} float 1.328125e-01

[RootSignature("")]
float main() : SV_Target {
    float2 x = float2(0.5, 0.125);
    float2 y = float2(0.25, 0.0625);
    return dot(x, y);
}
