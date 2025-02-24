// RUN: %dxc -E main -T cs_6_0 %s | FileCheck %s

// CHECK: call i32 @dx.op.waveActiveOp.i32
// CHECK: call i32 @dx.op.waveActiveOp.i32

struct PerThreadData {
    int input;
    int output;
};
RWStructuredBuffer<PerThreadData> g_sb : register(u0);
[numthreads(8,8,1)]
void main(uint GI : SV_GroupIndex) {
    PerThreadData pts = g_sb[GI];
    if (GI % 2) {
        pts.output = WaveActiveSum(pts.input);
    }
    else {
        pts.output = WaveActiveSum(pts.input);
    }
    g_sb[GI] = pts;
}