// RUN: %dxc -T ps_6_0 %s -E main | %FileCheck %s
// CHECK: call void @dx.op.storeOutput{{.*}} float 0x3FF3333340000000

[RootSignature("")]
float main() : SV_Target {
    float x = -1.2;
    return abs(x);
}

