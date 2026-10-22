// RUN: %dxc -T cs_6_6 -E main -fspv-use-descriptor-heap -spirv %s | FileCheck %s

// The validator now requires explicit layout for UniformConstant when
// SPV_EXT_descriptor_heap is used (KhronosGroup/SPIRV-Tools#6792). DXC's
// descriptor heap support does not yet emit ArrayStride.
// XFAIL: *

// CHECK: OpCapability DescriptorHeapEXT
// CHECK: OpExtension "SPV_EXT_descriptor_heap"

// CHECK-DAG: OpDecorate %[[ResourceHeap:[a-zA-Z0-9_]+]] BuiltIn ResourceHeapEXT
// CHECK-DAG: OpDecorate %[[SamplerHeap:[a-zA-Z0-9_]+]] BuiltIn SamplerHeapEXT

// CHECK-DAG: %[[UntypedPtrType:[a-zA-Z0-9_]+]] = OpTypeUntypedPointerKHR UniformConstant
// CHECK-DAG: %[[Tex2DType:[a-zA-Z0-9_]+]] = OpTypeImage %float 2D 2 0 0 1 Unknown
// CHECK-DAG: %[[RWTex2DType:[a-zA-Z0-9_]+]] = OpTypeImage %float 2D 2 0 0 2 Rgba32f
// CHECK-DAG: %[[BufferType:[a-zA-Z0-9_]+]] = OpTypeImage %float Buffer 2 0 0 1 Rgba32f
// CHECK-DAG: %[[RWBufferType:[a-zA-Z0-9_]+]] = OpTypeImage %float Buffer 2 0 0 2 Rgba32f
// CHECK-DAG: %[[SamplerType:[a-zA-Z0-9_]+]] = OpTypeSampler

// CHECK-DAG: %[[RA_Tex2DType:[a-zA-Z0-9_]+]] = OpTypeRuntimeArray %[[Tex2DType]]
// CHECK-DAG: %[[RA_RWTex2DType:[a-zA-Z0-9_]+]] = OpTypeRuntimeArray %[[RWTex2DType]]
// CHECK-DAG: %[[RA_BufferType:[a-zA-Z0-9_]+]] = OpTypeRuntimeArray %[[BufferType]]
// CHECK-DAG: %[[RA_RWBufferType:[a-zA-Z0-9_]+]] = OpTypeRuntimeArray %[[RWBufferType]]
// CHECK-DAG: %[[RA_SamplerType:[a-zA-Z0-9_]+]] = OpTypeRuntimeArray %[[SamplerType]]

// CHECK: %[[ResourceHeap]] = OpUntypedVariableKHR %[[UntypedPtrType]] UniformConstant
// CHECK: %[[SamplerHeap]]  = OpUntypedVariableKHR %[[UntypedPtrType]] UniformConstant

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID) {
    Texture2D<float4> myTex = ResourceDescriptorHeap[0];
    // CHECK: %[[TexIndex:[a-zA-Z0-9_]+]] = OpUntypedAccessChainKHR %[[UntypedPtrType]] %[[RA_Tex2DType]] %[[ResourceHeap]] %uint_0
    // CHECK: %[[TexHandle:[a-zA-Z0-9_]+]] = OpLoad %[[Tex2DType]] %[[TexIndex]]

    RWTexture2D<float4> myRWTex = ResourceDescriptorHeap[1];
    // CHECK: %[[RWTexIndex:[a-zA-Z0-9_]+]] = OpUntypedAccessChainKHR %[[UntypedPtrType]] %[[RA_RWTex2DType]] %[[ResourceHeap]] %uint_1
    // CHECK: %[[RWTexHandle:[a-zA-Z0-9_]+]] = OpLoad %[[RWTex2DType]] %[[RWTexIndex]]

    Buffer<float4> myBuf = ResourceDescriptorHeap[2];
    // CHECK: %[[BufIndex:[a-zA-Z0-9_]+]] = OpUntypedAccessChainKHR %[[UntypedPtrType]] %[[RA_BufferType]] %[[ResourceHeap]] %uint_2
    // CHECK: %[[BufHandle:[a-zA-Z0-9_]+]] = OpLoad %[[BufferType]] %[[BufIndex]]

    RWBuffer<float4> myRWBuf = ResourceDescriptorHeap[3];
    // CHECK: %[[RWBufIndex:[a-zA-Z0-9_]+]] = OpUntypedAccessChainKHR %[[UntypedPtrType]] %[[RA_RWBufferType]] %[[ResourceHeap]] %uint_3
    // CHECK: %[[RWBufHandle:[a-zA-Z0-9_]+]] = OpLoad %[[RWBufferType]] %[[RWBufIndex]]

    SamplerState mySamp = SamplerDescriptorHeap[0];
    // CHECK: %[[SampIndex:[a-zA-Z0-9_]+]] = OpUntypedAccessChainKHR %[[UntypedPtrType]] %[[RA_SamplerType]] %[[SamplerHeap]] %uint_0
    // CHECK: %[[SampHandle:[a-zA-Z0-9_]+]] = OpLoad %[[SamplerType]] %[[SampIndex]]

    // CHECK: %[[SampledImage:[a-zA-Z0-9_]+]] = OpSampledImage %{{.*}} %[[TexHandle]] %[[SampHandle]]
    // CHECK: %[[TexResult:[a-zA-Z0-9_]+]] = OpImageSampleExplicitLod %v4float %[[SampledImage]]
    float4 texVal = myTex.SampleLevel(mySamp, float2(0, 0), 0);

    // CHECK: %[[BufResult:[a-zA-Z0-9_]+]] = OpImageFetch %v4float %[[BufHandle]]
    float4 bufVal = myBuf.Load(tid.x);

    // CHECK: OpImageWrite %[[RWTexHandle]]
    myRWTex[tid.xy] = texVal;

    // CHECK: OpImageWrite %[[RWBufHandle]]
    myRWBuf[tid.x] = bufVal;
}
