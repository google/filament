// RUN: %dxc -E main -T vs_6_0 %s | FileCheck %s -check-prefixes=CHK1,CHK3
// RUN: %dxc -E main -T vs_6_2 -enable-16bit-types %s | FileCheck %s -check-prefix=CHK2

// CHK1: main 
// CHK3: call float @dx.op.loadInput.f32 
// CHK3-NEXT: call float @dx.op.loadInput.f32

// CHK2: call half @dx.op.loadInput.f16
// CHK2-NEXT: call half @dx.op.loadInput.f16

half main(half a : IN0, half b : IN1) : OUT
{ 
	return a + b;
}