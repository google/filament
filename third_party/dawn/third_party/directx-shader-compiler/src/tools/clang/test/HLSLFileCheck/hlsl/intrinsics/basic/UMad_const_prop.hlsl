// RUN: %dxc -T ps_6_0 %s -E main | %FileCheck %s
// CHECK: call void @dx.op.storeOutput{{.*}} i32 -4

[RootSignature("")]
uint main() : SV_Target {
    uint x = -2;
    uint y = 5;
    uint z = 3;

    uint64_t xl = 0x100000000ULL;
    uint64_t yl = 0x000000002ULL;
    uint64_t zl = 0x000000003ULL;
    return mad(x, y ,z)     // -7
         + mad(xl, yl, zl); // 0x200000003
}
