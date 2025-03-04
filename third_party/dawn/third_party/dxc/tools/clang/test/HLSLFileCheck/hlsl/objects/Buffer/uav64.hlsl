// RUN: %dxc -E main -T cs_6_0 %s | FileCheck %s

// Verify that 9 UAVs will set the 64Uav shader flag

// CHECK: Note: shader requires additional functionality:
// CHECK: 64 UAV slots
// CHECK: @main

RWBuffer<float> output : register(u0);
RWStructuredBuffer<float> g_buf1 : register(u1);
RWStructuredBuffer<float> g_buf2 : register(u2);
RWStructuredBuffer<float> g_buf3 : register(u3);
RWStructuredBuffer<float> g_buf4 : register(u4);
RWStructuredBuffer<float> g_buf5 : register(u5);
RWStructuredBuffer<float> g_buf6 : register(u6);
RWStructuredBuffer<float> g_buf7 : register(u7);
RWStructuredBuffer<float> g_buf8 : register(u8);

[numthreads(8,8,1)]
void main(uint GI : SV_GroupIndex) {
    output[GI] = g_buf1[GI] + g_buf2[GI] + g_buf3[GI] + g_buf4[GI] +
                 g_buf5[GI] + g_buf6[GI] + g_buf7[GI] + g_buf8[GI];
}