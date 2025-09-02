// RUN: not %dxc -spirv -E main -T cs_6_7 %s 2>&1 | FileCheck %s

struct Content {
  float a;
};

typedef vk::BufferPointer<Content> BufferContent;

[[vk::push_constant]]
BufferContent buffer;

[numthreads(1, 1, 1)]
void main() {
  float tmp = buffer.Get().a;
  buffer.Get().a = tmp;
}

// CHECK: vk::push_constant attribute cannot be used on declarations with vk::BufferPointer type

