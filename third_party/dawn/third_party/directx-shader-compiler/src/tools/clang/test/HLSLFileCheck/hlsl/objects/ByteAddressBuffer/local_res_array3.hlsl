// RUN: %dxc -T cs_6_0 -E main %s | FileCheck %s

// Local resource array elements all map to same global resource, so this is legal.
// CHECK: define void @main()

RWByteAddressBuffer outputBuffer[3];
uint i;
[numthreads(8, 8, 1)]
void main( uint2 id : SV_DispatchThreadID )
{
	RWByteAddressBuffer buffer[2];
	buffer[0] = outputBuffer[2];
	buffer[1] = outputBuffer[0];
    buffer[i].Store(id.y, id.x);
}