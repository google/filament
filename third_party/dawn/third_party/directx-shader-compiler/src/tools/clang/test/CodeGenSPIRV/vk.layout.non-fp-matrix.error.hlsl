// RUN: not %dxc -T vs_6_0 -E main -fcgl  %s -spirv  2>&1 | FileCheck %s

cbuffer MyCBuffer {
  struct S {
    int2x3   matrices4[5];
  } s;
}

struct T {
    int2x3   t[5];
};

RWStructuredBuffer<T> rwsb;

void main() {
   int2x3 m4 = s.matrices4[1];
}

// CHECK: :6:5: error: externally initialized non-floating-point column-major matrices not supported yet
