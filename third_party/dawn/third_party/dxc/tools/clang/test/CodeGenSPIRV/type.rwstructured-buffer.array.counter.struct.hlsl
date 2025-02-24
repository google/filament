// RUN: not %dxc -T cs_6_6 -E main -fcgl  %s -spirv 2>&1 | FileCheck %s

RWStructuredBuffer<int> a[2];

struct S {
  RWStructuredBuffer<int> a[2];
};

[numthreads(8, 8, 1)]
void main() {
	S s;
  s.a = a;
  s.a[0].IncrementCounter();
	return;
}

// CHECK: :13:3: fatal error: Cannot access associated counter variable for an array of buffers in a struct.