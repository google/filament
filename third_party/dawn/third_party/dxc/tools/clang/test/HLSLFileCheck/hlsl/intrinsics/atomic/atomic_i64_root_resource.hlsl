// RUN: %dxc -T lib_6_6 %s | FileCheck %s
// Make sure library targets correctly identify non-dynamic resources

// CHECK: Note: shader requires additional functionality:
// CHECK-NOT: 64-bit Atomics on Heap Resources


RWStructuredBuffer<int64_t> myBuf : register(u0);

[shader("raygeneration")]
void RGInt64OnDescriptorHeapIndex()
{
    InterlockedAdd(myBuf[0], 1);
}
