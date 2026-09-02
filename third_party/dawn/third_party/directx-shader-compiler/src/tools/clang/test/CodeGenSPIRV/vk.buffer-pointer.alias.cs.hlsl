// RUN: %dxc -spirv -E main -T cs_6_7 %s | FileCheck %s

// Bug was causing alignment miss

struct Content {
  int a;
};

typedef vk::BufferPointer<Content> BufferContent;
typedef vk::BufferPointer<BufferContent> BufferBuffer;

RWStructuredBuffer<BufferBuffer> rwbuf;

void foo(BufferContent bc) {
  bc.Get().a = 1;
}

[numthreads(1, 1, 1)]
void main() {
  foo(rwbuf[0].Get());
}

// CHECK: [[L0:%[_0-9A-Za-z]*]] = OpLoad %{{[_0-9A-Za-z]*}} %{{[_0-9A-Za-z]*}}
// CHECK: [[L1:%[_0-9A-Za-z]*]] = OpLoad %{{[_0-9A-Za-z]*}} [[L0]] Aligned 8
// CHECK: [[L2:%[_0-9A-Za-z]*]] = OpAccessChain %{{[_0-9A-Za-z]*}} [[L1]] %int_0
// CHECK: OpStore [[L2]] %int_1 Aligned 4


