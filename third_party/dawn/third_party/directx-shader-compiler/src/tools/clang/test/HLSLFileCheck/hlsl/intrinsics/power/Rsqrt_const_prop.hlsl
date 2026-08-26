// RUN: %dxc -T ps_6_0 %s -E main | %FileCheck %s
// CHECK: call void @dx.op.storeOutput{{.*}} float 4.000000e+00

[RootSignature("")]
float main(float x : A) : SV_Target {
    float y = 0.0625;
    return rsqrt(y);
}
