// RUN: not %dxc -spirv -E main -T cs_6_7 %s 2>&1 | FileCheck %s

struct Globals_s {
  float4 a;
};

typedef vk::BufferPointer<Globals_s> Globals_p;
typedef vk::BufferPointer<Globals_p> Globals_pp;

[[vk::push_constant]]
Globals_pp bda;

[numthreads(1, 1, 1)]
void main() {
  float4 r = bda.Get().Get().a;
}

// CHECK: vk::push_constant attribute cannot be used on declarations with vk::BufferPointer type

