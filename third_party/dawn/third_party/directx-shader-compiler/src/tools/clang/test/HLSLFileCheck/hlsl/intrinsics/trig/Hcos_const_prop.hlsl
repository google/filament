// RUN: %dxc -T ps_6_0 %s -E main | %FileCheck %s
// CHECK: call void @dx.op.storeOutput{{.*}} float 0x3FF20AC180000000 

[RootSignature("")]
float main(float x : A) : SV_Target {
    float y = 0.5;
    return cosh(y);
}
