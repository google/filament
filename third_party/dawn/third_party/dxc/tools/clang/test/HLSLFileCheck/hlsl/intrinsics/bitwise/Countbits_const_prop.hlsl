// RUN: %dxc -T ps_6_0 %s -E main | %FileCheck %s
// CHECK: call void @dx.op.storeOutput{{.*}} i32 23

[RootSignature("")]
int main() : SV_Target {
    int x = 0xdadadada;
    uint64_t y = 0x30100000000ULL;
    return countbits(x) + countbits(y);
}