// RUN: %dxc -T lib_6_6 %s | FileCheck %s

// CHECK:error: root signature attribute not supported for raytracing entry functions


RWStructuredBuffer<int64_t> myBuf : register(u0);

[shader("raygeneration")]
[RootSignature("UAV(u0)")]
void RGInt64OnDescriptorHeapIndex()
{
    InterlockedAdd(myBuf[0], 1);
}
