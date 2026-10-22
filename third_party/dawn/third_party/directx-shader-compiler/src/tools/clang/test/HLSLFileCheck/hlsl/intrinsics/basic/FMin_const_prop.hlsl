// RUN: %dxc -T ps_6_0 %s -E main | %FileCheck %s
// CHECK: call void @dx.op.storeOutput{{.*}} float 5.000000e-01

[RootSignature("")]
float main() : SV_Target {
    float x = 0.5;
    float y = 0.25;
    
    double d1 = 0.5;
    double d2 = 0.25;

    return min(x, y) + min(d1, d2);
}
