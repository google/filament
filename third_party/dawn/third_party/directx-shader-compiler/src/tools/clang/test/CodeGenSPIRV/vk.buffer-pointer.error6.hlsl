// RUN: not %dxc -spirv -E main -T cs_6_7 %s 2>&1 | FileCheck %s

struct Content {
  int a;
};

typedef vk::BufferPointer<Content> BufferContent;
typedef vk::BufferPointer<BufferContent> BufferBuffer;

RWStructuredBuffer<BufferContent> buf;

void foo(const BufferContent bc) {
  bc.Get().a = 1;
}

[numthreads(1, 1, 1)]
void main() {
  static BufferContent bcs = buf[0];
  static BufferBuffer bbs = (BufferContent)bcs;
}

// CHECK: cannot initialize a variable of type 'BufferPointer<BufferContent>' with an lvalue of type 'BufferPointer<Content>'

