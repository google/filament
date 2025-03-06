// RUN: %dxilver 1.6 | %dxc -E main -T cs_6_0 %s | FileCheck %s

// Verify that 9 UAVs in an array will still add the 64Uav shader flag

// CHECK: Note: shader requires additional functionality:
// CHECK: 64 UAV slots
// CHECK: @main

RWBuffer<float> output : register(u0);
RWStructuredBuffer<float> g_buf[8] : register(u1);

[numthreads(8,8,1)]
void main(uint GI : SV_GroupIndex) {
    output[GI] = g_buf[1][GI] + g_buf[2][GI] + g_buf[3][GI] + g_buf[4][GI] +
                 g_buf[5][GI] + g_buf[6][GI] + g_buf[7][GI];
}