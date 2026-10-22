// RUN: %dxc -T ps_6_0 %s -E main | %FileCheck %s
// CHECK: call void @dx.op.storeOutput{{.*}} float 5.078125e-01

[RootSignature("")]
float main() : SV_Target {
    float   x = 0.5;
    float   y = 0.25;
    float   z = 0.125;
    float   f = mad(x, y, z);    // 0.25

    double d1 = 0.0625;
    double d2 = 0.125;
    double d3 = 0.25;
    double d  = mad(d1, d2, d3); // 0.2578125

    return f + d;
}
