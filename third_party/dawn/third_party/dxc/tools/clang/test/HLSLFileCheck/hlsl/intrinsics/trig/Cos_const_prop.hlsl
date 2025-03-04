// RUN: %dxc -T ps_6_0 %s -E main | %FileCheck %s
// CHECK: call void @dx.op.storeOutput{{.*}} float 0x3FB21BD540000000 

[RootSignature("")]
float main(float x : A) : SV_Target {
    float y = 1.5;
    return cos(y);
}
