// RUN: %dxc -T cs_6_0 -E main %s | FileCheck %s

// Make sure simple local resource array works.
// CHECK: main

RWByteAddressBuffer outputBuffer;
RWByteAddressBuffer outputBuffer2;

[numthreads(8, 8, 1)]
void main( uint2 id : SV_DispatchThreadID )
{
	RWByteAddressBuffer buffer[2];
	buffer[0] = outputBuffer;
	buffer[1] = outputBuffer2;
    buffer[0].Store(id.x, id.y);
    buffer[1].Store(id.y, id.x);
} 