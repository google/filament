// RUN: %dxc -T cs_6_0 -E main %s | FileCheck %s

// Report error when cannot promote local resource.
// CHECK: local resource not guaranteed to map to unique global resource

RWByteAddressBuffer outputBuffer;
RWByteAddressBuffer outputBuffer2;
uint i;
[numthreads(8, 8, 1)]
void main( uint2 id : SV_DispatchThreadID )
{
	RWByteAddressBuffer buffer[2];
	buffer[0] = outputBuffer;
	buffer[1] = outputBuffer2;
    buffer[i].Store(id.y, id.x);
}