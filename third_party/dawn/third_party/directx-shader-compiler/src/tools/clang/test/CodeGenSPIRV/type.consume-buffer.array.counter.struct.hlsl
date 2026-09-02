// RUN: not %dxc -T cs_6_6 -E main -fcgl  %s -spirv 2>&1 | FileCheck %s

ConsumeStructuredBuffer<int> a[2];
RWStructuredBuffer<int> o;

struct S {
  ConsumeStructuredBuffer<int> a[2];
};

[numthreads(8, 8, 1)]
void main() {
	S s;
  s.a = a;
  o[0] = s.a[0].Consume();
	return;
}

// CHECK: :14:10: fatal error: Cannot access associated counter variable for an array of buffers in a struct.