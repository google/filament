// RUN: %dxc -E main -T cs_6_0 %s | FileCheck %s

// Make sure NoOp cast on atomic operations compiles.
// CHECK: main

RWBuffer<uint> buffer;

[numthreads(1,1,1)]
void main()
{
	uint unused;
	InterlockedOr((uint)buffer[0], 1, unused);
}