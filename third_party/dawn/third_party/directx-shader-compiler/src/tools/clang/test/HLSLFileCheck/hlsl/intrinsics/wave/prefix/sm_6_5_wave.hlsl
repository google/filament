// RUN: %dxc -E main -T ps_6_5 %s | FileCheck %s

StructuredBuffer<uint4> g_mask;

uint4 main(uint4 input : ATTR0) : SV_Target {
    uint4 mask = g_mask[0];

    // CHECK: call %dx.types.fouri32 @dx.op.waveMatch.i32(i32 165,
    // CHECK: call %dx.types.fouri32 @dx.op.waveMatch.i32(i32 165,
    // CHECK-DAG: and i32
    // CHECK-DAG: and i32
    // CHECK-DAG: and i32
    // CHECK-DAG: and i32
    // CHECK-DAG: call %dx.types.fouri32 @dx.op.waveMatch.i32(i32 165,
    // CHECK-DAG: call %dx.types.fouri32 @dx.op.waveMatch.i32(i32 165,
    // CHECK-DAG: and i32
    // CHECK-DAG: and i32
    // CHECK-DAG: and i32
    // CHECK-DAG: and i32
    // CHECK-DAG: and i32
    // CHECK-DAG: and i32
    // CHECK-DAG: and i32
    // CHECK-DAG: and i32
    uint4 res = WaveMatch(input);

    // CHECK: call i32 @dx.op.waveMultiPrefixOp.i32(i32 166, i32 %{{[0-9}+]}}, i32 %{{[0-9}+]}}, i32 %{{[0-9}+]}}, i32 %{{[0-9}+]}}, i32 %{{[0-9}+]}}, i8 1, i8 0)
    // CHECK: call i32 @dx.op.waveMultiPrefixOp.i32(i32 166, i32 %{{[0-9}+]}}, i32 %{{[0-9}+]}}, i32 %{{[0-9}+]}}, i32 %{{[0-9}+]}}, i32 %{{[0-9}+]}}, i8 1, i8 0)
    // CHECK: call i32 @dx.op.waveMultiPrefixOp.i32(i32 166, i32 %{{[0-9}+]}}, i32 %{{[0-9}+]}}, i32 %{{[0-9}+]}}, i32 %{{[0-9}+]}}, i32 %{{[0-9}+]}}, i8 1, i8 0)
    // CHECK: call i32 @dx.op.waveMultiPrefixOp.i32(i32 166, i32 %{{[0-9}+]}}, i32 %{{[0-9}+]}}, i32 %{{[0-9}+]}}, i32 %{{[0-9}+]}}, i32 %{{[0-9}+]}}, i8 1, i8 0)
    res += WaveMultiPrefixBitAnd(input, mask);

    // CHECK: call i32 @dx.op.waveMultiPrefixOp.i32(i32 166, i32 %{{[0-9}+]}}, i32 %{{[0-9}+]}}, i32 %{{[0-9}+]}}, i32 %{{[0-9}+]}}, i32 %{{[0-9}+]}}, i8 2, i8 0)
    // CHECK: call i32 @dx.op.waveMultiPrefixOp.i32(i32 166, i32 %{{[0-9}+]}}, i32 %{{[0-9}+]}}, i32 %{{[0-9}+]}}, i32 %{{[0-9}+]}}, i32 %{{[0-9}+]}}, i8 2, i8 0)
    // CHECK: call i32 @dx.op.waveMultiPrefixOp.i32(i32 166, i32 %{{[0-9}+]}}, i32 %{{[0-9}+]}}, i32 %{{[0-9}+]}}, i32 %{{[0-9}+]}}, i32 %{{[0-9}+]}}, i8 2, i8 0)
    // CHECK: call i32 @dx.op.waveMultiPrefixOp.i32(i32 166, i32 %{{[0-9}+]}}, i32 %{{[0-9}+]}}, i32 %{{[0-9}+]}}, i32 %{{[0-9}+]}}, i32 %{{[0-9}+]}}, i8 2, i8 0)
    res += WaveMultiPrefixBitOr(input, mask);

    // CHECK: call i32 @dx.op.waveMultiPrefixOp.i32(i32 166, i32 %{{[0-9}+]}}, i32 %{{[0-9}+]}}, i32 %{{[0-9}+]}}, i32 %{{[0-9}+]}}, i32 %{{[0-9}+]}}, i8 3, i8 0)
    // CHECK: call i32 @dx.op.waveMultiPrefixOp.i32(i32 166, i32 %{{[0-9}+]}}, i32 %{{[0-9}+]}}, i32 %{{[0-9}+]}}, i32 %{{[0-9}+]}}, i32 %{{[0-9}+]}}, i8 3, i8 0)
    // CHECK: call i32 @dx.op.waveMultiPrefixOp.i32(i32 166, i32 %{{[0-9}+]}}, i32 %{{[0-9}+]}}, i32 %{{[0-9}+]}}, i32 %{{[0-9}+]}}, i32 %{{[0-9}+]}}, i8 3, i8 0)
    // CHECK: call i32 @dx.op.waveMultiPrefixOp.i32(i32 166, i32 %{{[0-9}+]}}, i32 %{{[0-9}+]}}, i32 %{{[0-9}+]}}, i32 %{{[0-9}+]}}, i32 %{{[0-9}+]}}, i8 3, i8 0)
    res += WaveMultiPrefixBitXor(input, mask);

    // CHECK: call i32 @dx.op.waveMultiPrefixBitCount(i32 167, i1 %{{[A-Za-z0-9]+}}, i32 %{{[0-9}+]}}, i32 %{{[0-9}+]}}, i32 %{{[0-9}+]}}, i32 %{{[0-9}+]}})
    res.x += WaveMultiPrefixCountBits((input.x == 1), mask);

    // CHECK: call i32 @dx.op.waveMultiPrefixOp.i32(i32 166, i32 %{{[0-9}+]}}, i32 %{{[0-9}+]}}, i32 %{{[0-9}+]}}, i32 %{{[0-9}+]}}, i32 %{{[0-9}+]}}, i8 4, i8 1)
    // CHECK: call i32 @dx.op.waveMultiPrefixOp.i32(i32 166, i32 %{{[0-9}+]}}, i32 %{{[0-9}+]}}, i32 %{{[0-9}+]}}, i32 %{{[0-9}+]}}, i32 %{{[0-9}+]}}, i8 4, i8 1)
    // CHECK: call i32 @dx.op.waveMultiPrefixOp.i32(i32 166, i32 %{{[0-9}+]}}, i32 %{{[0-9}+]}}, i32 %{{[0-9}+]}}, i32 %{{[0-9}+]}}, i32 %{{[0-9}+]}}, i8 4, i8 1)
    // CHECK: call i32 @dx.op.waveMultiPrefixOp.i32(i32 166, i32 %{{[0-9}+]}}, i32 %{{[0-9}+]}}, i32 %{{[0-9}+]}}, i32 %{{[0-9}+]}}, i32 %{{[0-9}+]}}, i8 4, i8 1)
    res += WaveMultiPrefixProduct(input, mask);

    // CHECK: call i32 @dx.op.waveMultiPrefixOp.i32(i32 166, i32 %{{[0-9}+]}}, i32 %{{[0-9}+]}}, i32 %{{[0-9}+]}}, i32 %{{[0-9}+]}}, i32 %{{[0-9}+]}}, i8 4, i8 0)
    // CHECK: call i32 @dx.op.waveMultiPrefixOp.i32(i32 166, i32 %{{[0-9}+]}}, i32 %{{[0-9}+]}}, i32 %{{[0-9}+]}}, i32 %{{[0-9}+]}}, i32 %{{[0-9}+]}}, i8 4, i8 0)
    // CHECK: call i32 @dx.op.waveMultiPrefixOp.i32(i32 166, i32 %{{[0-9}+]}}, i32 %{{[0-9}+]}}, i32 %{{[0-9}+]}}, i32 %{{[0-9}+]}}, i32 %{{[0-9}+]}}, i8 4, i8 0)
    // CHECK: call i32 @dx.op.waveMultiPrefixOp.i32(i32 166, i32 %{{[0-9}+]}}, i32 %{{[0-9}+]}}, i32 %{{[0-9}+]}}, i32 %{{[0-9}+]}}, i32 %{{[0-9}+]}}, i8 4, i8 0)
    res += WaveMultiPrefixProduct((int4)input, mask);

    // CHECK: call i32 @dx.op.waveMultiPrefixOp.i32(i32 166, i32 %{{[0-9}+]}}, i32 %{{[0-9}+]}}, i32 %{{[0-9}+]}}, i32 %{{[0-9}+]}}, i32 %{{[0-9}+]}}, i8 0, i8 1)
    // CHECK: call i32 @dx.op.waveMultiPrefixOp.i32(i32 166, i32 %{{[0-9}+]}}, i32 %{{[0-9}+]}}, i32 %{{[0-9}+]}}, i32 %{{[0-9}+]}}, i32 %{{[0-9}+]}}, i8 0, i8 1)
    // CHECK: call i32 @dx.op.waveMultiPrefixOp.i32(i32 166, i32 %{{[0-9}+]}}, i32 %{{[0-9}+]}}, i32 %{{[0-9}+]}}, i32 %{{[0-9}+]}}, i32 %{{[0-9}+]}}, i8 0, i8 1)
    // CHECK: call i32 @dx.op.waveMultiPrefixOp.i32(i32 166, i32 %{{[0-9}+]}}, i32 %{{[0-9}+]}}, i32 %{{[0-9}+]}}, i32 %{{[0-9}+]}}, i32 %{{[0-9}+]}}, i8 0, i8 1)
    res += WaveMultiPrefixSum(input, mask);

    // CHECK: call i32 @dx.op.waveMultiPrefixOp.i32(i32 166, i32 %{{[0-9}+]}}, i32 %{{[0-9}+]}}, i32 %{{[0-9}+]}}, i32 %{{[0-9}+]}}, i32 %{{[0-9}+]}}, i8 0, i8 0)
    // CHECK: call i32 @dx.op.waveMultiPrefixOp.i32(i32 166, i32 %{{[0-9}+]}}, i32 %{{[0-9}+]}}, i32 %{{[0-9}+]}}, i32 %{{[0-9}+]}}, i32 %{{[0-9}+]}}, i8 0, i8 0)
    // CHECK: call i32 @dx.op.waveMultiPrefixOp.i32(i32 166, i32 %{{[0-9}+]}}, i32 %{{[0-9}+]}}, i32 %{{[0-9}+]}}, i32 %{{[0-9}+]}}, i32 %{{[0-9}+]}}, i8 0, i8 0)
    // CHECK: call i32 @dx.op.waveMultiPrefixOp.i32(i32 166, i32 %{{[0-9}+]}}, i32 %{{[0-9}+]}}, i32 %{{[0-9}+]}}, i32 %{{[0-9}+]}}, i32 %{{[0-9}+]}}, i8 0, i8 0)
    res += WaveMultiPrefixSum((int4)input, mask);

    return res;
}
