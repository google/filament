// RUN: %dxc -T lib_6_3 -auto-binding-space 11 %s | FileCheck %s

// Make sure load resource rangeID when select resource.
// CHECK:load i32, i32* @ReadBuffer1_rangeID
// CHECK:load i32, i32* @ReadBuffer_rangeID

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