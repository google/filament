// RUN: %dxc -T ps_6_0 %s -E main | %FileCheck %s
// CHECK: call void @dx.op.storeOutput{{.*}} i32 42

[RootSignature("")]
int main() : SV_Target {
    int x = 0x0;                   // firstbitlow(x) = -1
    int y = 0x100;                 // firstbitlow(y) =  8
    int z = 0x110;                 // firstbitlow(y) =  4
    uint64_t w = 0x30100000000ULL; // firstbitlow(w) = 32
    uint64_t s = 0x0ULL;           // firstbitlow(w) = -1
    return firstbitlow(x) + firstbitlow(y) + firstbitlow(z) + firstbitlow(w) + firstbitlow(s);
}