// RUN: %dxc -EMain -Tcs_6_6 %s | %opt -S -hlsl-dxil-pix-shader-access-instrumentation,config=.256;512;1024. | %FileCheck %s

static RWByteAddressBuffer DynamicBuffer = ResourceDescriptorHeap[1];
[numthreads(1, 1, 1)]
void Main()
{
    uint val = DynamicBuffer.Load(0u);
    DynamicBuffer.Store(0u, val);
}

// check it's 6.6:
// CHECK: call %dx.types.Handle @dx.op.createHandleFromBinding

// Check we wrote to the PIX UAV:
// CHECK: call void @dx.op.bufferStore.i32

// Offset for buffer Load should be 256 + 8 (skip out-of-bounds record) + 8 (it's the 1th resource) + 4 (offset to the "read" field) = 276
// The large integer is encoded flags for the ResourceAccessStyle (an enumerated type in lib\DxilPIXPasses\DxilShaderAccessTracking.cpp) for this access
// CHECK:i32 276, i32 undef, i32 1375731712
// CHECK:rawBufferLoad

// Offset for buffer Store should be 256 + 8 (skip out-of-bounds record) + 8 (it's the 1th resource) + 0 (offset to the "write" field) = 272
// The large integer is encoded flags for the ResourceAccessStyle (an enumerated type in lib\DxilPIXPasses\DxilShaderAccessTracking.cpp) for this access
// CHECK:i32 272, i32 undef, i32 1392508928
// CHECK:rawBufferStore

