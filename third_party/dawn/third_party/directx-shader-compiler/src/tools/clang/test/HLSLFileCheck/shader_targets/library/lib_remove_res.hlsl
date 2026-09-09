// RUN: %dxc -T lib_6_3 -auto-binding-space 11 -default-linkage external %s | FileCheck %s

// Ensure UnusedBuffer is removed:
// CHECK-NOT: @"\01?UnusedBuffer{{[@$?.A-Za-z0-9_]+}}"

// Ensure resource ID is 0 for ReadBuffer1 after UnusedBuffer global is removed.
// CHECK: !{i32 0, %struct.ByteAddressBuffer* @"\01?ReadBuffer1{{[@$?.A-Za-z0-9_]+}}", !"ReadBuffer1",

RWByteAddressBuffer outputBuffer : register(u0);
ByteAddressBuffer UnusedBuffer : register(t0);
ByteAddressBuffer ReadBuffer1 : register(t1);

void test()
{
  ByteAddressBuffer buffer = UnusedBuffer;

  if (true)
     buffer = ReadBuffer1;

  uint v = buffer.Load(0);
  outputBuffer.Store(0, v);
}
