// RUN: %dxc -E main -T cs_6_0 %s | FileCheck %s

// Make sure cast lvalue not work.
// CHECK: cannot compile this unexpected cast lvalue

RWBuffer<uint> buffer;

[numthreads(1,1,1)]
void main()
{
	float unused;
	InterlockedOr((int)buffer[0], 1, unused);
}