// RUN: %dxc -T ps_6_0 %s -E main | %FileCheck %s
// CHECK: call void @dx.op.storeOutput{{.*}} i32 1532713819

[RootSignature("")]
int main() : SV_Target {
    uint64_t s = reversebits(114ULL);
    int x = 0xdadadada;
    return reversebits(x);
}