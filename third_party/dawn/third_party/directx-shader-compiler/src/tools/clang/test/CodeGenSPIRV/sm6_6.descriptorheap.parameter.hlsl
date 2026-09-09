// RUN: not %dxc -T cs_6_6 -spirv %s 2>&1 | FileCheck %s

// CHECK: error: Resource/sampler heaps are not allowed as function parameters.
template<typename T>
float get(T t) {
  RWStructuredBuffer<float> data = t[0];
  return data[0];
}

[numthreads(1, 1, 1)]
void main() {
  get(ResourceDescriptorHeap);
}
