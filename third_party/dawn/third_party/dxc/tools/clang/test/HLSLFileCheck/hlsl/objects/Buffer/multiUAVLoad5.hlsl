// RUN: %dxilver 1.1 | %dxc -E main -T cs_6_0 %s | FileCheck %s

// CHECK-NOT: Typed UAV Load Additional Formats

RWByteAddressBuffer g_bab : register(u0);
[numthreads(8,8,1)]
void main(uint GI : SV_GroupIndex) {
    uint addr = GI * 4;
    uint val = g_bab.Load(addr);
    g_bab.Store(addr, val + 1);
}
