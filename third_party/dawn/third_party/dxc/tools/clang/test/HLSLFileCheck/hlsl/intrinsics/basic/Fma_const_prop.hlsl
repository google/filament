// RUN: %dxc -T ps_6_0 %s -E main | %FileCheck %s
// CHECK: call void @dx.op.storeOutput{{.*}} float 2.500000e-01

[RootSignature("")]
float main() : SV_Target {
    double x = 0.5;
    double y = 0.25;
    double z = 0.125;
    double d = fma(x, y, z); // 0.25

    return d;
}
