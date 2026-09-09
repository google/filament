// RUN: %dxilver 1.6 | %dxc -T ps_6_5 -DTYPE=double  %s | %FileCheck %s
// RUN: %dxc -T ps_6_5 -DTYPE=float  %s | %FileCheck %s
// RUN: %dxc -T ps_6_5 -DTYPE=int  %s | %FileCheck %s
// RUN: %dxc -T ps_6_5 -DTYPE=uint  %s | %FileCheck %s
// RUN: %dxc -T ps_6_5 -DTYPE=int16_t -enable-16bit-types  %s | %FileCheck %s
// RUN: %dxc -T ps_6_5 -DTYPE=uint16_t -enable-16bit-types  %s | %FileCheck %s
// RUN: %dxc -T ps_6_5 -DTYPE=half  %s | %FileCheck %s
// RUN: %dxc -T ps_6_5 -DTYPE=bool  %s | %FileCheck %s
// RUN: %dxc -T ps_6_5 -DTYPE=int64_t  %s | %FileCheck %s
// RUN: %dxc -T ps_6_5 -DTYPE=uint64_t  %s | %FileCheck %s

// Verify that all appropriate types can be passed to Wave and QuadRead ops

// Just need to verify that there were no errors.
// CHECK: @main
// CHECK-NOT: error:
cbuffer CB
{
  TYPE expr;
}

float4 main(uint4 id: IN0) : SV_Target
{
  TYPE ret = WaveReadLaneAt(expr, id.x) +
    WaveReadLaneFirst(expr) +
    WaveActiveAllEqual(expr) +
    WaveActiveProduct(expr) +
    WaveActiveSum(expr) +
    WaveActiveMin(expr) +
    WaveActiveMax(expr) +
    WavePrefixProduct(expr) +
    WavePrefixSum(expr) +
    WaveMultiPrefixProduct(expr, id) +
    WaveMultiPrefixSum(expr, id) +
    QuadReadLaneAt(expr, id.x) +
    QuadReadAcrossX(expr) +
    QuadReadAcrossY(expr) +
    QuadReadAcrossDiagonal(expr);
  return (float)ret;
}
