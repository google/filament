// RUN: %dxilver 1.2 | %dxc -E main -T cs_6_2 -enable-16bit-types %s | FileCheck %s

// CHECK: @dx.op.unaryBits.i16
// CHECK: @dx.op.unaryBits.i16
// CHECK: @dx.op.unaryBits.i16

struct SUnaryUintOp {
    uint16_t input1;
    uint16_t4 input2;
    uint16_t4 output;
};

RWStructuredBuffer<SUnaryUintOp> g_buf;
[numthreads(8,8,1)]
void main(uint GI : SV_GroupIndex) {
    SUnaryUintOp l = g_buf[GI];
    int16_t output = 0;
    output += countbits(l.input1);
    output += reversebits(l.input2).x;
    output += firstbitlow(l.input2).x;
    l.output = output;
    g_buf[GI] = l;
};