// RUN: %dxc -E main -T cs_6_0 %s | FileCheck %s

// CHECK: dx.op.bufferLoad
// CHECK: switch

RWBuffer<int4> buf;

[numthreads(8, 8, 1)]
void main(uint2 id : SV_DispatchThreadId) {
  buf[id.x][id.y] = 1;
}