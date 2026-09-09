// RUN: %dxc -T cs_6_6 -E CSMain -spirv -DTYPE=0 %s | FileCheck %s --check-prefix=TYPE0
// RUN: %dxc -T cs_6_6 -E CSMain -spirv -DTYPE=1 %s | FileCheck %s --check-prefix=TYPE1
// RUN: %dxc -T cs_6_6 -E CSMain -spirv -DTYPE=2 %s | FileCheck %s --check-prefix=TYPE2
// RUN: %dxc -T cs_6_6 -E CSMain -spirv -DTYPE=3 %s | FileCheck %s --check-prefix=TYPE3

// TYPE=0: direct ResourceDescriptorHeap[] init of a `globallycoherent` static.
// TYPE0-DAG: OpDecorate %ResourceDescriptorHeap{{(_[0-9]+)?}} DescriptorSet 0
// TYPE0-DAG: OpDecorate %ResourceDescriptorHeap{{(_[0-9]+)?}} Binding 0
// TYPE0-DAG: OpDecorate %ResourceDescriptorHeap{{(_[0-9]+)?}} Coherent

// TYPE=1: bindless array carries the qualifier.
// TYPE1-DAG: OpDecorate %FakeHeapOfA DescriptorSet 0
// TYPE1-DAG: OpDecorate %FakeHeapOfA Binding 0
// TYPE1-DAG: OpDecorate %FakeHeapOfA Coherent

// TYPE=2: stand-alone `globallycoherent` UAV.
// TYPE2-DAG: OpDecorate %A DescriptorSet 0
// TYPE2-DAG: OpDecorate %A Binding 0
// TYPE2-DAG: OpDecorate %A Coherent

// TYPE=3: heap access lives inside a `globallycoherent`-returning helper, and 
// a `globallycoherent static` captures the result.
// TYPE3-DAG: OpDecorate %ResourceDescriptorHeap{{(_[0-9]+)?}} DescriptorSet 0
// TYPE3-DAG: OpDecorate %ResourceDescriptorHeap{{(_[0-9]+)?}} Binding {{[0-9]+}}
// TYPE3-DAG: OpDecorate %ResourceDescriptorHeap{{(_[0-9]+)?}} Coherent
// TYPE3-NOT: OpDecorate %Buf Coherent

#if TYPE == 0
globallycoherent static RWStructuredBuffer<uint> A = ResourceDescriptorHeap[0];
#elif TYPE == 1
globallycoherent RWStructuredBuffer<uint> FakeHeapOfA[];
globallycoherent static RWStructuredBuffer<uint> A = FakeHeapOfA[0];
#elif TYPE == 2
globallycoherent RWStructuredBuffer<uint> A;
#elif TYPE == 3
uint BindlessUAV_Buf;
typedef RWByteAddressBuffer SafeTypeBuf;
globallycoherent SafeTypeBuf GetBuf() { return ResourceDescriptorHeap[BindlessUAV_Buf]; }
static const globallycoherent SafeTypeBuf A = GetBuf();
#endif

[numthreads(64, 1, 1)]
void CSMain(uint3 ThreadId : SV_DispatchThreadId)
{
#if TYPE == 3
    A.InterlockedAdd(0, 1);
    AllMemoryBarrierWithGroupSync();
    A.Store(0, 42);
#else
    InterlockedAdd(A[0], 1);
    AllMemoryBarrierWithGroupSync();
    InterlockedAdd(A[1], A[0]);
#endif
}