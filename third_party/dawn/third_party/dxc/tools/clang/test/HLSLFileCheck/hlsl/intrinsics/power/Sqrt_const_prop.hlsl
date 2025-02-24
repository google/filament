// RUN: %dxc -T ps_6_0 %s -E main | %FileCheck %s
// CHECK: call void @dx.op.storeOutput{{.*}} float 2.500000e-01

[RootSignature("")]
float main(float x : A) : SV_Target {
    float y = 0.0625;
    return sqrt(y);
}
