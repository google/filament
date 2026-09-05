// RUN: %dxc -E main -T cs_6_0 %s | FileCheck %s

// CHECK-NOT: dx.op.bufferLoad

struct foo {
  uint4 bar;
};

RWStructuredBuffer<foo> buf;

[numthreads(8, 8, 1)]
void main(uint2 id : SV_DispatchThreadId) {
  buf[id.x].bar.w = 1;
  buf[id.y].bar.xz = int2(2,3);
}