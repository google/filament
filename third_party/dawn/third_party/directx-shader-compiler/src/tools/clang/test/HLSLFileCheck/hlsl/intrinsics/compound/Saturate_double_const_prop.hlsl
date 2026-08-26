// RUN: %dxc -T ps_6_0 %s -E main | %FileCheck %s
// CHECK: call void @dx.op.storeOutput{{.*}} float 1.500000e+00 

[RootSignature("")]
float main() : SV_Target {
    double x = -1.2;
    double y = 1.2;
    double z = 0.5;
    return saturate(x)
         + saturate(y)
         + saturate(z);
}