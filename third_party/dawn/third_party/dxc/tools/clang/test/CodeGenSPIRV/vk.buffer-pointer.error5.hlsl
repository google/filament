// RUN: not %dxc -spirv -E main -T cs_6_7 %s 2>&1 | FileCheck %s

struct Content {
  int a;
};

typedef vk::BufferPointer<Content> BufferContent;
typedef vk::BufferPointer<BufferContent> BufferBuffer;

//[[vk::push_constant]]
//BufferContent buffer;

RWStructuredBuffer<BufferBuffer> rwbuf;

// Wrong type in the parameter.
void foo(BufferContent bc) {
  bc.Get().a = 1;
}

[numthreads(1, 1, 1)]
void main() {
  foo(rwbuf[0]);
}

// CHECK: no matching function for call to 'foo'

