// RUN: %dxc -T ps_6_0 %s -E main | %FileCheck %s
// CHECK: call void @dx.op.storeOutput{{.*}} i32 -1

[RootSignature("")]
int main() : SV_Target {
    int x = 5;
    int y = 25;
    int z = -1;

    return min(min(x, y), z);
}
