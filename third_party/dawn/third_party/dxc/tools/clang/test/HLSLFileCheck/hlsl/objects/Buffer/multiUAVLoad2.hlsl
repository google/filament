// RUN: %dxilver 1.1 | %dxc -E main -T cs_6_0 %s | FileCheck %s

// CHECK-NOT: Typed UAV Load Additional Formats

RWBuffer<float1> g_buf : register(u0);
[numthreads(8,8,1)]
void main(uint GI : SV_GroupIndex) {
    uint addr = GI * 4;
    float1 val = g_buf.Load(addr);
    g_buf[addr] = val + 1;
}
