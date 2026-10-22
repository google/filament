// RUN: %dxilver 1.1 | %dxc -E main -T cs_6_0 %s | FileCheck %s

// CHECK-NOT: Typed UAV Load Additional Formats

struct SUnaryFPOp {
    float input;
    float output;
};
RWStructuredBuffer<SUnaryFPOp> g_buf : register(u0);
[numthreads(8,8,1)]
void main(uint GI : SV_GroupIndex) {
    SUnaryFPOp l = g_buf[GI];
    l.output = sin(l.input);
    g_buf[GI] = l;
}