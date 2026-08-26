// RUN: %dxc -T ps_6_0 %s -E main | %FileCheck %s
// CHECK: call void @dx.op.storeOutput{{.*}} float 5.000000e+00

[RootSignature("")]
float main() : SV_Target {
    float x = 1.5;
    float y = 2.5;
    return ceil(x) + ceil(y);
}