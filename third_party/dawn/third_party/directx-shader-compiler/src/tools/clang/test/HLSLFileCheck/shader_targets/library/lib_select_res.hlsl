// RUN: %dxc -T lib_6_3 -auto-binding-space 11 -default-linkage external %s | FileCheck %s

// resource uses must resolve to a single resource global variable (single rangeID)
// CHECK: local resource not guaranteed to map to unique global resource

RWByteAddressBuffer outputBuffer : register(u0);
ByteAddressBuffer ReadBuffer : register(t0);
ByteAddressBuffer ReadBuffer1 : register(t1);

void test( uint cond)
{
	ByteAddressBuffer buffer = ReadBuffer;
        if (cond > 2)
           buffer = ReadBuffer1;

	uint v= buffer.Load(0);
    outputBuffer.Store(0, v);
}