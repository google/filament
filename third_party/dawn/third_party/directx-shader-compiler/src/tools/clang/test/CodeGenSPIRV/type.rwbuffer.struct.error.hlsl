// RUN: not %dxc -T vs_6_0 -E main -fcgl  %s -spirv  2>&1 | FileCheck %s

struct S {
  float a;
  float b;
};

// CHECK: error: elements of typed buffers and textures must be scalars or vectors
RWBuffer<S> MyRWBuffer;

// CHECK: error: elements of typed buffers and textures must be scalars or vectors
RasterizerOrderedBuffer<S> MyROVBuffer;

float4 main() : A {
  return 1.0;
}

