// RUN: %dxc -T ps_6_0 %s -E main | %FileCheck %s
// CHECK: call void @dx.op.storeOutput{{.*}} float 1.500000e+00 

[RootSignature("")]
float main() : SV_Target {
    min16float x = -1.2;
    min16float y = 1.2;
    min16float z = 0.5;
    return saturate(x)
         + saturate(y)
         + saturate(z);
}