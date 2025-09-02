// RUN: not %dxc -spirv -E main -T cs_6_7 %s 2>&1 | FileCheck %s

struct Content {
  uint a;
};

typedef vk::BufferPointer<uint> BufferContent;

[[vk::push_constant]]
BufferContent buffer;

[numthreads(1, 1, 1)]
void main() {
  uint data = buffer.Get();
  buffer.Get() = data;
}

// CHECK: vk::push_constant attribute cannot be used on declarations with vk::BufferPointer type

