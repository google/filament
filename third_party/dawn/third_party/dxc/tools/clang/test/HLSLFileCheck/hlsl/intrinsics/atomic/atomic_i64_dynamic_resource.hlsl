// RUN: %dxc -T lib_6_6 %s | FileCheck %s
// Test atomic operations on dynamic resources

// CHECK: Note: shader requires additional functionality:
// CHECK: 64-bit Atomics on Typed Resources
// CHECK: 64-bit Atomics on Heap Resources

[shader("raygeneration")]
void RGInt64OnDescriptorHeapIndex()
{
    RWTexture2D<int64_t> myTexture = ResourceDescriptorHeap[7];
    InterlockedAdd(myTexture[int2(0,0)], 1);
}
