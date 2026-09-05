// RUN: %dxc -E main -T cs_6_0 %s | FileCheck %s

// CHECK: Typed UAV Load Additional Formats

RWBuffer<float4> g_buf : register(u0);
[numthreads(8,8,1)]
void main(uint GI : SV_GroupIndex) {
    uint addr = GI * 4;
    float4 val = g_buf.Load(addr);
    g_buf[addr] = val + 1;
}
