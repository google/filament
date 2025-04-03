// RUN: not %dxc -T cs_6_6 -E main -enable-16bit-types %s -spirv 2>&1 | FileCheck %s

// This test is to make sure if the SPIR-V code generation returned an
// error, the compilation fails, but the validation is not run.
// If the validation runs on incomplete code, the user might get confused
// as validation error after compilation are considered to be a actual bug.

RWBuffer<half> a;

RWBuffer<half> b;

struct S {
  float a;
};

// CHECK: error: elements of typed buffers and textures must be scalars or vectors
RWBuffer<S> buff;

[numthreads(1, 1, 1)]
void main() {
// CHECK-NOT: fatal error: generated SPIR-V is invalid
  a[0] = b[0];
}

