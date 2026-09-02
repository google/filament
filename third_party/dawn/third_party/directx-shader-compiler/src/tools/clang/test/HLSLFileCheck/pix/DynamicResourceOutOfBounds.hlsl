// RUN: %dxc -EMain -Tcs_6_6 %s | %opt -S -hlsl-dxil-pix-shader-access-instrumentation,config=.256;272;1024. | %FileCheck %s

static RWByteAddressBuffer DynamicBuffer = ResourceDescriptorHeap[1];
[numthreads(1, 1, 1)]
void Main()
{
    uint val = DynamicBuffer.Load(0u);
    DynamicBuffer.Store(0u, val);
}

// check it's 6.6:
// CHECK: call %dx.types.Handle @dx.op.createHandleFromBinding

// The start of resource records has been passed in as 256. The limit of resource records is 272. 272-256 = 16.
// 8 bytes per record means we have one record for out-of-bounds (that comes first), and one record for resource index 0.
// The above HLSL references resource descriptor 1, so is out-of-bounds. Offset for out-of-bounds should thus be 256:
// The large integer is encoded flags for the ResourceAccessStyle (an enumerated type in lib\DxilPIXPasses\DxilShaderAccessTracking.cpp) 
// for this access plus 0x80000000 to indicate descriptor-heap indexing.
// CHECK:i32 256, i32 undef, i32 1476395008
// CHECK:rawBufferLoad
