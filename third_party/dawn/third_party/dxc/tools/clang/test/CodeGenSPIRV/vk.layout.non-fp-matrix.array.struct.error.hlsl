// RUN: not %dxc -T vs_6_0 -E main -fcgl  %s -spirv  2>&1 | FileCheck %s

struct A {
  uint2x3 a;
};

struct B {
  A a[7];
};

StructuredBuffer<B> buffer;

struct T {
  uint a;
  float1x3 c;
  uint3x3 d;
  uint2x3 f;
  uint g;
};

struct U {
  uint a[3];
  float b;
  uint3 c;
  float2 e;
  T f[7];
  uint g;
};

StructuredBuffer<U> buffer2;

void main()
{
}


// CHECK: :11:21: error: externally initialized non-floating-point column-major matrices not supported yet
