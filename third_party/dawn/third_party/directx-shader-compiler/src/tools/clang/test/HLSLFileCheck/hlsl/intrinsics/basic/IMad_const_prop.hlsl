// RUN: %dxc -T ps_6_0 %s -E main | %FileCheck %s
// CHECK: call void @dx.op.storeOutput{{.*}} i32 -7

[RootSignature("")]
int main() : SV_Target {
    int x = -2;
    int y = 5;
    int z = 3;
    return mad(x, y ,z); // -7
}
